/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "ConfigurationPACS.h"
#include "DBConnection.h"
#include "ExceptionPACS.h"
#include "NetworkPACS.h"
#include "SCP/EchoSCP.h"
#include "SCP/FindSCP.h"
#include "SCP/GetSCP.h"
#include "SCP/MoveSCP.h"
#include "SCP/StoreSCP.h"

namespace research_pacs
{
    
NetworkPACS * NetworkPACS::_instance = NULL;

NetworkPACS &
NetworkPACS
::get_instance()
{
    if(NetworkPACS::_instance == NULL)
    {
        NetworkPACS::_instance = new NetworkPACS();
    }
    return *NetworkPACS::_instance;
}
    
NetworkPACS
::NetworkPACS():
    _storage(""),
    _authenticator(NULL),
    _network(NULL)
{
    this->create_authenticator();
    
    int port = std::atoi(ConfigurationPACS::get_instance().GetValue("dicom.port").c_str());
    
    OFCondition cond = ASC_initializeNetwork(NET_ACCEPTORREQUESTOR, (int)port, 30, &this->_network);
    if (cond.bad()) 
    {
        throw ExceptionPACS("cannot initialize network: " + std::string(cond.text()));
    }
}

NetworkPACS
::~NetworkPACS()
{
    OFCondition cond = ASC_dropNetwork(&this->_network);
    if (cond.bad()) 
    {
        throw ExceptionPACS("cannot drop network: " + std::string(cond.text()));
    }
}

void 
NetworkPACS
::create_authenticator()
{
    std::string type = ConfigurationPACS::get_instance().GetValue("authenticator.type");
    if(type == "CSV")
    {
        this->_authenticator = new authenticator::AuthenticatorCSV
            (ConfigurationPACS::get_instance().GetValue("authenticator.filepath"));
    }
    else
    {
        throw ExceptionPACS("Unknown authentication type "+type);
    }
}

void
NetworkPACS
::run()
{
    int timeout = 1000; // todo multiprocess
    while (true)
    {
        if (ASC_associationWaiting(this->_network, timeout))
        {
            bool continue_ = true;
            T_ASC_Association * assoc;
            OFCondition cond = ASC_receiveAssociation(this->_network, &assoc, ASC_DEFAULTMAXPDU);
            if (cond.bad())
            {
                std::cout << "Failed to receive association: " << cond.text();
            }
            else
            {
                time_t t = time(NULL);
                std::cout << "Association Received (" << assoc->params->DULparams.callingPresentationAddress
                          << ":" << assoc->params->DULparams.callingAPTitle << " -> "
                          << assoc->params->DULparams.calledAPTitle << ") " << ctime(&t);
                          
                OFString temp_str;
                ASC_dumpParameters(temp_str, assoc->params, ASC_ASSOC_RQ);
                
                // process
                
                if (!continue_)
                {
                    // Authentication User / Password
                    if( ! (*this->_authenticator)(assoc->params->DULparams.reqUserIdentNeg))
                    {
                        std::cout << "Refusing Association: Bad User/Password" << std::endl;;
                        refuseAssociation(&assoc, CTN_NoReason);
                        continue_ = false;
                    }
                }
                
                if (continue_)
                {
                    /* Application Context Name */
                    char buf[BUFSIZ];
                    cond = ASC_getApplicationContextName(assoc->params, buf);
                    if (cond.bad() || strcmp(buf, DICOM_STDAPPLICATIONCONTEXT) != 0)
                    {
                        /* reject: the application context name is not supported */
                        std::cout << "Bad AppContextName: " << buf;
                        refuseAssociation(&assoc, CTN_BadAppContext);
                        continue_ = false;
                    }
                }
                
                if (continue_)
                {
                    /* Does peer AE have access to required service ?? */
                    if (! ConfigurationPACS::get_instance().peerInAETitle
                                (//assoc->params->DULparams.calledAPTitle,
                                 assoc->params->DULparams.callingAPTitle))
                                 //assoc->params->DULparams.callingPresentationAddress))
                    {
                        refuseAssociation(&assoc, CTN_BadAEService);
                        continue_ = false;
                    }
                }
                
                if (continue_)
                {
                    cond = negotiateAssociation(assoc);
                    if (cond.bad()) continue_ = false;
                }
                
                if (continue_)
                {
                    cond = ASC_acknowledgeAssociation(assoc);
                    if (cond.bad())
                    {
                        std::cout << cond.text();
                        continue_ = false;
                    }
                }
                
                if (continue_)
                {
                    // dispatch
                    this->handleAssociation(assoc);
                }
            }
            
            // cleanup code
            if (!continue_)
            {
                /* the child will handle the association, we can drop it */
                cond = ASC_dropAssociation(assoc);
                if (cond.bad())
                {
                    throw ExceptionPACS("Cannot Drop Association: " + std::string(cond.text()));
                }
                cond = ASC_destroyAssociation(&assoc);
                if (cond.bad())
                {
                    throw ExceptionPACS("Cannot Destroy Association: " + std::string(cond.text()));
                }
            }
            else if (cond == ASC_SHUTDOWNAPPLICATION)
            {
                // shutdown
                break;
            }
        }
        // else continue
    }
}


void 
NetworkPACS
::refuseAssociation(T_ASC_Association ** assoc, CTN_RefuseReason reason)
{
    const char *reason_string;
    switch (reason)
    {
      case CTN_TooManyAssociations:
          reason_string = "TooManyAssociations";
          break;
      case CTN_CannotFork:
          reason_string = "CannotFork";
          break;
      case CTN_BadAppContext:
          reason_string = "BadAppContext";
          break;
      case CTN_BadAEPeer:
          reason_string = "BadAEPeer";
          break;
      case CTN_BadAEService:
          reason_string = "BadAEService";
          break;
      case CTN_NoReason:
          reason_string = "NoReason";
        break;
      default:
          reason_string = "???";
          break;
    }
    std::cout << "Refusing Association (" << reason_string << ")" << std::endl;;

    T_ASC_RejectParameters rej;
    switch (reason)
    {
      case CTN_TooManyAssociations:
        rej.result = ASC_RESULT_REJECTEDTRANSIENT;
        rej.source = ASC_SOURCE_SERVICEPROVIDER_PRESENTATION_RELATED;
        rej.reason = ASC_REASON_SP_PRES_LOCALLIMITEXCEEDED;
        break;
      case CTN_CannotFork:
        rej.result = ASC_RESULT_REJECTEDPERMANENT;
        rej.source = ASC_SOURCE_SERVICEPROVIDER_PRESENTATION_RELATED;
        rej.reason = ASC_REASON_SP_PRES_TEMPORARYCONGESTION;
        break;
      case CTN_BadAppContext:
        rej.result = ASC_RESULT_REJECTEDTRANSIENT;
        rej.source = ASC_SOURCE_SERVICEUSER;
        rej.reason = ASC_REASON_SU_APPCONTEXTNAMENOTSUPPORTED;
        break;
      case CTN_BadAEPeer:
        rej.result = ASC_RESULT_REJECTEDPERMANENT;
        rej.source = ASC_SOURCE_SERVICEUSER;
        rej.reason = ASC_REASON_SU_CALLINGAETITLENOTRECOGNIZED;
        break;
      case CTN_BadAEService:
        rej.result = ASC_RESULT_REJECTEDPERMANENT;
        rej.source = ASC_SOURCE_SERVICEUSER;
        rej.reason = ASC_REASON_SU_CALLEDAETITLENOTRECOGNIZED;
        break;
      case CTN_NoReason:
      default:
        rej.result = ASC_RESULT_REJECTEDPERMANENT;
        rej.source = ASC_SOURCE_SERVICEUSER;
        rej.reason = ASC_REASON_SU_NOREASON;
        break;
    }

    OFCondition cond = ASC_rejectAssociation(*assoc, &rej);
    if (cond.bad())
    {
        throw ExceptionPACS("Association Reject Failed: " + std::string(cond.text()));
    }

    cond = ASC_dropAssociation(*assoc);
    if (cond.bad())
    {
        throw ExceptionPACS("Cannot Drop Association: " + std::string(cond.text()));
    }
    cond = ASC_destroyAssociation(assoc);
    if (cond.bad())
    {
        throw ExceptionPACS("Cannot Destroy Association: " + std::string(cond.text()));
    }
}
    
OFCondition 
NetworkPACS
::negotiateAssociation(T_ASC_Association * assoc)
{
    OFCondition cond = EC_Normal;
    int i;
    T_ASC_PresentationContextID movepid, findpid;
    OFString temp_str;
    struct { const char *moveSyntax, *findSyntax; } queryRetrievePairs[] =
    {
      { UID_MOVEPatientRootQueryRetrieveInformationModel,
        UID_FINDPatientRootQueryRetrieveInformationModel },
      { UID_MOVEStudyRootQueryRetrieveInformationModel,
        UID_FINDStudyRootQueryRetrieveInformationModel },
      { UID_RETIRED_MOVEPatientStudyOnlyQueryRetrieveInformationModel,
        UID_RETIRED_FINDPatientStudyOnlyQueryRetrieveInformationModel }
    };

    DIC_AE calledAETitle;
    ASC_getAPTitles(assoc->params, NULL, calledAETitle, NULL);

    const char* transferSyntaxes[] = { NULL, NULL, NULL, NULL };
    int numTransferSyntaxes = 0;

    // TODO mettre en conf network_transfer_syntax
/*    switch (options_.networkTransferSyntax_)
    {
      case EXS_LittleEndianImplicit:
        /* we only support Little Endian Implicit *
        transferSyntaxes[0]  = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 1;
        break;
      case EXS_LittleEndianExplicit:
        /* we prefer Little Endian Explicit *
        transferSyntaxes[0] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[1] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[2]  = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 3;
        break;
      case EXS_BigEndianExplicit:
        /* we prefer Big Endian Explicit *
        transferSyntaxes[0] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[2]  = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 3;
        break;
#ifndef DISABLE_COMPRESSION_EXTENSION
      case EXS_JPEGProcess14SV1TransferSyntax:
        /* we prefer JPEGLossless:Hierarchical-1stOrderPrediction (default lossless) *
        transferSyntaxes[0] = UID_JPEGProcess14SV1TransferSyntax;
        transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 4;
        break;
      case EXS_JPEGProcess1TransferSyntax:
        /* we prefer JPEGBaseline (default lossy for 8 bit images) *
        transferSyntaxes[0] = UID_JPEGProcess1TransferSyntax;
        transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 4;
        break;
      case EXS_JPEGProcess2_4TransferSyntax:
        /* we prefer JPEGExtended (default lossy for 12 bit images) *
        transferSyntaxes[0] = UID_JPEGProcess2_4TransferSyntax;
        transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 4;
        break;
      case EXS_JPEG2000LosslessOnly:
        /* we prefer JPEG 2000 lossless *
        transferSyntaxes[0] = UID_JPEG2000LosslessOnlyTransferSyntax;
        transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 4;
        break;
      case EXS_JPEG2000:
        /* we prefer JPEG 2000 lossy or lossless *
        transferSyntaxes[0] = UID_JPEG2000TransferSyntax;
        transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 4;
        break;
      case EXS_JPEGLSLossless:
        /* we prefer JPEG-LS Lossless *
        transferSyntaxes[0] = UID_JPEGLSLosslessTransferSyntax;
        transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 4;
        break;
      case EXS_JPEGLSLossy:
        /* we prefer JPEG-LS Lossy *
        transferSyntaxes[0] = UID_JPEGLSLossyTransferSyntax;
        transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 4;
        break;
      case EXS_MPEG2MainProfileAtMainLevel:
        /* we prefer MPEG2 MP@ML *
        transferSyntaxes[0] = UID_MPEG2MainProfileAtMainLevelTransferSyntax;
        transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 4;
        break;
      case EXS_MPEG2MainProfileAtHighLevel:
        /* we prefer MPEG2 MP@HL *
        transferSyntaxes[0] = UID_MPEG2MainProfileAtHighLevelTransferSyntax;
        transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 4;
        break;
      case EXS_RLELossless:
        /* we prefer RLE Lossless *
        transferSyntaxes[0] = UID_RLELosslessTransferSyntax;
        transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 4;
        break;
#ifdef WITH_ZLIB
      case EXS_DeflatedLittleEndianExplicit:
        /* we prefer deflated transmission *
        transferSyntaxes[0] = UID_DeflatedExplicitVRLittleEndianTransferSyntax;
        transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 4;
        break;
#endif
#endif
      default:
        /* We prefer explicit transfer syntaxes.
         * If we are running on a Little Endian machine we prefer
         * LittleEndianExplicitTransferSyntax to BigEndianTransferSyntax.
         */
        if (gLocalByteOrder == EBO_LittleEndian)  /* defined in dcxfer.h */
        {
          transferSyntaxes[0] = UID_LittleEndianExplicitTransferSyntax;
          transferSyntaxes[1] = UID_BigEndianExplicitTransferSyntax;
        } else {
          transferSyntaxes[0] = UID_BigEndianExplicitTransferSyntax;
          transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
        }
        transferSyntaxes[2] = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 3;
        //break;
    //}

    const char * const nonStorageSyntaxes[] =
    {
        UID_VerificationSOPClass,
        UID_FINDPatientRootQueryRetrieveInformationModel,
        UID_MOVEPatientRootQueryRetrieveInformationModel,
        UID_GETPatientRootQueryRetrieveInformationModel,
#ifndef NO_PATIENTSTUDYONLY_SUPPORT
        UID_RETIRED_FINDPatientStudyOnlyQueryRetrieveInformationModel,
        UID_RETIRED_MOVEPatientStudyOnlyQueryRetrieveInformationModel,
        UID_RETIRED_GETPatientStudyOnlyQueryRetrieveInformationModel,
#endif
        UID_FINDStudyRootQueryRetrieveInformationModel,
        UID_MOVEStudyRootQueryRetrieveInformationModel,
        UID_GETStudyRootQueryRetrieveInformationModel,
        UID_PrivateShutdownSOPClass
    };

    const int numberOfNonStorageSyntaxes = DIM_OF(nonStorageSyntaxes);
    const char *selectedNonStorageSyntaxes[DIM_OF(nonStorageSyntaxes)];
    int numberOfSelectedNonStorageSyntaxes = 0;
    for (i = 0; i < numberOfNonStorageSyntaxes; i++)
    {
        if (0 == strcmp(nonStorageSyntaxes[i], UID_FINDPatientRootQueryRetrieveInformationModel))
        {
            selectedNonStorageSyntaxes[numberOfSelectedNonStorageSyntaxes++] = nonStorageSyntaxes[i];
        }
        else if (0 == strcmp(nonStorageSyntaxes[i], UID_MOVEPatientRootQueryRetrieveInformationModel))
        {
            selectedNonStorageSyntaxes[numberOfSelectedNonStorageSyntaxes++] = nonStorageSyntaxes[i];
        }
        else if (0 == strcmp(nonStorageSyntaxes[i], UID_GETPatientRootQueryRetrieveInformationModel))
        {
            selectedNonStorageSyntaxes[numberOfSelectedNonStorageSyntaxes++] = nonStorageSyntaxes[i];
        }
        else if (0 == strcmp(nonStorageSyntaxes[i], UID_RETIRED_FINDPatientStudyOnlyQueryRetrieveInformationModel))
        {
            selectedNonStorageSyntaxes[numberOfSelectedNonStorageSyntaxes++] = nonStorageSyntaxes[i];
        }
        else if (0 == strcmp(nonStorageSyntaxes[i], UID_RETIRED_MOVEPatientStudyOnlyQueryRetrieveInformationModel))
        {
            selectedNonStorageSyntaxes[numberOfSelectedNonStorageSyntaxes++] = nonStorageSyntaxes[i];
        }
        else if (0 == strcmp(nonStorageSyntaxes[i], UID_RETIRED_GETPatientStudyOnlyQueryRetrieveInformationModel))
        {
            selectedNonStorageSyntaxes[numberOfSelectedNonStorageSyntaxes++] = nonStorageSyntaxes[i];
        }
        else if (0 == strcmp(nonStorageSyntaxes[i], UID_FINDStudyRootQueryRetrieveInformationModel))
        {
            selectedNonStorageSyntaxes[numberOfSelectedNonStorageSyntaxes++] = nonStorageSyntaxes[i];
        }
        else if (0 == strcmp(nonStorageSyntaxes[i], UID_MOVEStudyRootQueryRetrieveInformationModel))
        {
            selectedNonStorageSyntaxes[numberOfSelectedNonStorageSyntaxes++] = nonStorageSyntaxes[i];
        }
        else if (0 == strcmp(nonStorageSyntaxes[i], UID_GETStudyRootQueryRetrieveInformationModel))
        {
            selectedNonStorageSyntaxes[numberOfSelectedNonStorageSyntaxes++] = nonStorageSyntaxes[i];
        }
        else if (0 == strcmp(nonStorageSyntaxes[i], UID_PrivateShutdownSOPClass))
        {
            selectedNonStorageSyntaxes[numberOfSelectedNonStorageSyntaxes++] = nonStorageSyntaxes[i];
        } 
        else 
        {
            selectedNonStorageSyntaxes[numberOfSelectedNonStorageSyntaxes++] = nonStorageSyntaxes[i];
        }
    }

    /*  accept any of the non-storage syntaxes */
    cond = ASC_acceptContextsWithPreferredTransferSyntaxes(
    assoc->params,
    (const char**)selectedNonStorageSyntaxes, numberOfSelectedNonStorageSyntaxes,
    (const char**)transferSyntaxes, numTransferSyntaxes);
    if (cond.bad()) {
        DCMQRDB_ERROR("Cannot accept presentation contexts: " << DimseCondition::dump(temp_str, cond));
    }

    /* accept storage syntaxes with proposed role */
    T_ASC_PresentationContext pc;
    T_ASC_SC_ROLE role;
    int npc = ASC_countPresentationContexts(assoc->params);
    for (i = 0; i < npc; i++)
    {
        ASC_getPresentationContext(assoc->params, i, &pc);
        if (dcmIsaStorageSOPClassUID(pc.abstractSyntax))
        {
            /*
            ** We are prepared to accept whatever role he proposes.
            ** Normally we can be the SCP of the Storage Service Class.
            ** When processing the C-GET operation we can be the SCU of the Storage Service Class.
            */
            role = pc.proposedRole;

            /*
            ** Accept in the order "least wanted" to "most wanted" transfer
            ** syntax.  Accepting a transfer syntax will override previously
            ** accepted transfer syntaxes.
            */
            for (int k = numTransferSyntaxes - 1; k >= 0; k--)
            {
                for (int j = 0; j < (int)pc.transferSyntaxCount; j++)
                {
                    /* if the transfer syntax was proposed then we can accept it
                    * appears in our supported list of transfer syntaxes
                    */
                    if (strcmp(pc.proposedTransferSyntaxes[j], transferSyntaxes[k]) == 0)
                    {
                        cond = ASC_acceptPresentationContext(
                        assoc->params, pc.presentationContextID, transferSyntaxes[k], role);
                        if (cond.bad()) return cond;
                    }
                }
            }
        }
    } /* for */

    /*
     * check if we have negotiated the private "shutdown" SOP Class
     */
    if (0 != ASC_findAcceptedPresentationContextID(assoc, UID_PrivateShutdownSOPClass))
    {
        std::cout << "Shutting down server ... (negotiated private \"shut down\" SOP class)";
        refuseAssociation(&assoc, CTN_NoReason);
        return ASC_SHUTDOWNAPPLICATION;
    }

    /*
     * Refuse any "Storage" presentation contexts to non-writable
     * storage areas.
     */
    /*if (!config_->writableStorageArea(calledAETitle))
    {
        refuseAnyStorageContexts(assoc);
    }*/

    /*
     * Enforce RSNA'93 Demonstration Requirements about only
     * accepting a context for MOVE if a context for FIND is also present.
     */

    for (i = 0; i < (int)DIM_OF(queryRetrievePairs); i++) {
        movepid = ASC_findAcceptedPresentationContextID(assoc,
                            queryRetrievePairs[i].moveSyntax);
        if (movepid != 0) 
        {
            findpid = ASC_findAcceptedPresentationContextID(assoc,
                            queryRetrievePairs[i].findSyntax);
            if (findpid == 0) 
            {
                DCMQRDB_ERROR("Move Presentation Context but no Find (accepting for now)");
            }
        }
    }

    return cond;
}

void
NetworkPACS
::handleAssociation(T_ASC_Association * assoc)
{
    DIC_NODENAME        peerHostName;
    DIC_AE              peerAETitle;
    DIC_AE              myAETitle;
    OFString            temp_str;

    ASC_getPresentationAddresses(assoc->params, peerHostName, NULL);
    ASC_getAPTitles(assoc->params, peerAETitle, myAETitle, NULL);
    
    // this while loop is executed exactly once unless the "keepDBHandleDuringAssociation_"
    // flag is not set, in which case the inner loop is executed only once and this loop
    // repeats for each incoming DIMSE command. In this case, the DB handle is created
    // and released for each DIMSE command.
    OFCondition cond = EC_Normal;
    while (cond.good())
    {
        T_ASC_PresentationContextID presID;
        T_DIMSE_Message msg;
        cond = DIMSE_receiveCommand(assoc, DIMSE_BLOCKING, 0, &presID, &msg, NULL);

        if (msg.CommandField == DIMSE_C_ECHO_RQ)
        {
            // Veriry user's rights
            if (!DBConnection::get_instance().checkUserAuthorization(*assoc->params->DULparams.reqUserIdentNeg,
                                                                     msg.CommandField))
            {
                cond = DIMSE_BADCOMMANDTYPE;
                DCMQRDB_ERROR("Cannot handle command: 0x" << STD_NAMESPACE hex <<
                            (unsigned)msg.CommandField);
            }
        }
    
        /* did peer release, abort, or do we have a valid message ? */
        if (cond.good())
        {
            /* process command */
            switch (msg.CommandField) {
            case DIMSE_C_ECHO_RQ:
            {
                EchoSCP echoscp(assoc, presID, &msg.msg.CEchoRQ);
                cond = echoscp.process();
                break;
            }
            case DIMSE_C_STORE_RQ:
            {
                StoreSCP storescp(assoc, presID, &msg.msg.CStoreRQ);
                cond = storescp.process();
                break;
            }
            case DIMSE_C_FIND_RQ:
            {
                FindSCP findscp(assoc, presID, &msg.msg.CFindRQ);
                cond = findscp.process();
                break;
            }
            case DIMSE_C_GET_RQ:
            {
                GetSCP getscp(assoc, presID, &msg.msg.CGetRQ);
                cond = getscp.process();
                break;
            }
            case DIMSE_C_MOVE_RQ:
            {
                MoveSCP movescp(assoc, presID, &msg.msg.CMoveRQ);
                cond = movescp.process();
                break;
            }
            case DIMSE_C_CANCEL_RQ:
                /* This is a late cancel request, just ignore it */
                DCMQRDB_INFO("dispatch: late C-CANCEL-RQ, ignoring");
                break;
            default:
                /* we cannot handle this kind of message */
                cond = DIMSE_BADCOMMANDTYPE;
                DCMQRDB_ERROR("Cannot handle command: 0x" << STD_NAMESPACE hex <<
                        (unsigned)msg.CommandField);
                /* the condition will be returned, the caller will abort the association. */
            }
        }
        else if ((cond == DUL_PEERREQUESTEDRELEASE)||(cond == DUL_PEERABORTEDASSOCIATION))
        {
            // association gone
        }
        else
        {
            // the condition will be returned, the caller will abort the assosiation.
        }
    }
    
    /* clean up on association termination */
    if (cond == DUL_PEERREQUESTEDRELEASE) 
    {
        DCMQRDB_INFO("Association Release");
        cond = ASC_acknowledgeRelease(assoc);
        ASC_dropSCPAssociation(assoc);
    } 
    else if (cond == DUL_PEERABORTEDASSOCIATION) 
    {
        DCMQRDB_INFO("Association Aborted");
    } 
    else 
    {
        DCMQRDB_ERROR("DIMSE Failure (aborting association): " << DimseCondition::dump(temp_str, cond));
        /* some kind of error so abort the association */
        cond = ASC_abortAssociation(assoc);
    }

    cond = ASC_dropAssociation(assoc);
    if (cond.bad()) 
    {
        DCMQRDB_ERROR("Cannot Drop Association: " << DimseCondition::dump(temp_str, cond));
    }
    cond = ASC_destroyAssociation(&assoc);
    if (cond.bad()) 
    {
        DCMQRDB_ERROR("Cannot Destroy Association: " << DimseCondition::dump(temp_str, cond));
    }
}

} // namespace research_pacs
