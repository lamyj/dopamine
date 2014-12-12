/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <sstream>

#include "ConfigurationPACS.h"
#include "core/LoggerPACS.h"
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

void
NetworkPACS
::delete_instance()
{
    if (NetworkPACS::_instance != NULL)
    {
        delete NetworkPACS::_instance;
    }
    NetworkPACS::_instance = NULL;
}
    
NetworkPACS
::NetworkPACS():
    _authenticator(NULL),
    _network(NULL),
    _forceStop(false),
    _timeout(1000)
{
    this->create_authenticator();
    
    int port = std::atoi(ConfigurationPACS::get_instance().GetValue("dicom.port").c_str());
    
    OFCondition cond = ASC_initializeNetwork(NET_ACCEPTORREQUESTOR, (int)port, 30, &this->_network);
    if (cond.bad()) 
    {
        std::stringstream stream;
        stream << "cannot initialize network: " << cond.text();
        throw ExceptionPACS(stream.str());
    }
}

NetworkPACS
::~NetworkPACS()
{
    OFCondition cond = ASC_dropNetwork(&this->_network);
    if (cond.bad()) 
    {
        std::stringstream stream;
        stream << "cannot drop network: " << cond.text();
        throw ExceptionPACS(stream.str());
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
    else if (type == "LDAP")
    {
        this->_authenticator = new authenticator::AuthenticatorLDAP
            (
                ConfigurationPACS::get_instance().GetValue("authenticator.ldap_server"),
                ConfigurationPACS::get_instance().GetValue("authenticator.ldap_bind_user"),
                ConfigurationPACS::get_instance().GetValue("authenticator.ldap_base"),
                ConfigurationPACS::get_instance().GetValue("authenticator.ldap_filter")
            );
    }
    else if (type == "None")
    {
        this->_authenticator = new authenticator::AuthenticatorNone();
    }
    else
    {
        std::stringstream stream;
        stream << "Unknown authentication type " << type;
        throw ExceptionPACS(stream.str());
    }
}

void
NetworkPACS
::run()
{
    // todo multiprocess
    while (!this->_forceStop)
    {
        if (ASC_associationWaiting(this->_network, this->_timeout))
        {
            bool continue_ = true;
            T_ASC_Association * assoc;
            OFCondition cond = ASC_receiveAssociation(this->_network, &assoc, ASC_DEFAULTMAXPDU);
            if (cond.bad())
            {
                research_pacs::loggerError() << "Failed to receive association: "
                                             << cond.text();
            }
            else
            {
                time_t t = time(NULL);
                research_pacs::loggerInfo() << "Association Received ("
                                            << assoc->params->DULparams.callingPresentationAddress
                                            << ":" << assoc->params->DULparams.callingAPTitle << " -> "
                                            << assoc->params->DULparams.calledAPTitle << ") " << ctime(&t);

                OFString temp_str;
                ASC_dumpParameters(temp_str, assoc->params, ASC_ASSOC_RQ);
                
                // process
                
                if (continue_)
                {
                    // Authentication User / Password
                    if( ! (*this->_authenticator)(assoc->params->DULparams.reqUserIdentNeg))
                    {
                        research_pacs::loggerWarning() << "Refusing Association: Bad Authentication";
                        this->refuseAssociation(&assoc, CTN_NoReason);
                        continue_ = false;
                    }
                    research_pacs::loggerDebug() << "Authentication Status: " << continue_;
                }
                
                if (continue_)
                {
                    /* Application Context Name */
                    char buf[BUFSIZ];
                    cond = ASC_getApplicationContextName(assoc->params, buf);
                    if (cond.bad() || std::string(buf) != DICOM_STDAPPLICATIONCONTEXT)
                    {
                        /* reject: the application context name is not supported */
                        research_pacs::loggerWarning() << "Bad AppContextName: " << buf;
                        this->refuseAssociation(&assoc, CTN_BadAppContext);
                        continue_ = false;
                    }
                    research_pacs::loggerDebug() << "Application Context Name Status: " << continue_;
                }
                
                if (continue_)
                {
                    /* Does peer AE have access to required service ?? */
                    if (! ConfigurationPACS::get_instance().peerInAETitle
                                (assoc->params->DULparams.callingAPTitle))
                    {
                        research_pacs::loggerWarning() << "Bad AE Service";
                        this->refuseAssociation(&assoc, CTN_BadAEService);
                        continue_ = false;
                    }
                    research_pacs::loggerDebug() << "AE Service Status: " << continue_;
                }
                
                if (continue_)
                {
                    cond = this->negotiateAssociation(assoc);research_pacs::loggerDebug() << "DEBUG RLA negociation ok";
                    if (cond.good() || cond == ASC_SHUTDOWNAPPLICATION)
                    {
                        bool shutdown = cond == ASC_SHUTDOWNAPPLICATION;
                        cond = ASC_acknowledgeAssociation(assoc);research_pacs::loggerDebug() << "DEBUG RLA ack ok";
                        if (cond.good())
                        {
                            // dispatch
                            this->handleAssociation(assoc);research_pacs::loggerDebug() << "DEBUG RLA handle ok";
                        }
                        if (shutdown)
                        {
                            // shutdown
                            break;
                        }
                    }

                    if (cond.bad())
                    {
                        research_pacs::loggerWarning() << "Association error: "
                                                       << cond.text();
                    }

                    continue_ = cond.good();
                }
            }

            // cleanup code
            if (!continue_)
            {
                /* the child will handle the association, we can drop it */
                cond = ASC_dropAssociation(assoc);
                if (cond.bad())
                {
                    std::stringstream stream;
                    stream << "Cannot Drop Association: " << cond.text();
                    throw ExceptionPACS(stream.str());
                }
                cond = ASC_destroyAssociation(&assoc);
                if (cond.bad())
                {
                    std::stringstream stream;
                    stream << "Cannot Destroy Association: " << cond.text();
                    throw ExceptionPACS(stream.str());
                }
            }
        }
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
    research_pacs::loggerError() << "Refusing Association ("
                                 << reason_string << ")";

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
        std::stringstream stream;
        stream << "Association Reject Failed: " << cond.text();
        throw ExceptionPACS(stream.str());
    }

    cond = ASC_dropAssociation(*assoc);
    if (cond.bad())
    {
        std::stringstream stream;
        stream << "Cannot Drop Association: " << cond.text();
        throw ExceptionPACS(stream.str());
    }
    cond = ASC_destroyAssociation(assoc);
    if (cond.bad())
    {
        std::stringstream stream;
        stream << "Cannot Destroy Association: " << cond.text();
        throw ExceptionPACS(stream.str());
    }
}

OFCondition
NetworkPACS
::negotiateAssociation(T_ASC_Association * assoc)
{
    OFCondition cond = EC_Normal;

    DIC_AE calledAETitle;
    cond = ASC_getAPTitles(assoc->params, NULL, calledAETitle, NULL);
    if (cond.bad())
    {
        research_pacs::loggerError() << "Cannot retrieve AP Titles";
    }

    const char* transferSyntaxes[] = { NULL, NULL, NULL, NULL };

    /* We prefer explicit transfer syntaxes.
     * If we are running on a Little Endian machine we prefer
     * LittleEndianExplicitTransferSyntax to BigEndianTransferSyntax.
     */
    if (gLocalByteOrder == EBO_LittleEndian)  /* defined in dcxfer.h */
    {
      transferSyntaxes[0] = UID_LittleEndianExplicitTransferSyntax;
      transferSyntaxes[1] = UID_BigEndianExplicitTransferSyntax;
    }
    else //(gLocalByteOrder == EBO_BigEndian)
    {
      transferSyntaxes[0] = UID_BigEndianExplicitTransferSyntax;
      transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
    }
    transferSyntaxes[2] = UID_LittleEndianImplicitTransferSyntax;
    int numTransferSyntaxes = 3;

    const char* nonStorageSyntaxes[] =
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
    /*  accept any of the non-storage syntaxes */
    cond = ASC_acceptContextsWithPreferredTransferSyntaxes(assoc->params,
                                                           (const char**)nonStorageSyntaxes,
                                                           numberOfNonStorageSyntaxes,
                                                           (const char**)transferSyntaxes,
                                                           numTransferSyntaxes);
    if (cond.bad())
    {
        OFString temp_str;
        research_pacs::loggerInfo() << "Cannot accept presentation contexts: "
                                    << DimseCondition::dump(temp_str, cond);
    }

    /* accept storage syntaxes with proposed role */
    int npc = ASC_countPresentationContexts(assoc->params);
    for (int i = 0; i < npc; i++)
    {
        T_ASC_PresentationContext pc;
        ASC_getPresentationContext(assoc->params, i, &pc);
        if (dcmIsaStorageSOPClassUID(pc.abstractSyntax))
        {
            /*
            ** We are prepared to accept whatever role he proposes.
            ** Normally we can be the SCP of the Storage Service Class.
            ** When processing the C-GET operation we can be the SCU of the Storage Service Class.
            */
            T_ASC_SC_ROLE role = pc.proposedRole;

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
                    if (std::string(pc.proposedTransferSyntaxes[j]) == std::string(transferSyntaxes[k]))
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
        research_pacs::loggerInfo() << "Shutting down server ... (negotiated private \"shut down\" SOP class)";
        return ASC_SHUTDOWNAPPLICATION;
    }

    /*
     * Enforce RSNA'93 Demonstration Requirements about only
     * accepting a context for MOVE if a context for FIND is also present.
     */
    struct { const char *moveSyntax, *findSyntax; } queryRetrievePairs[] =
    {
      { UID_MOVEPatientRootQueryRetrieveInformationModel,
        UID_FINDPatientRootQueryRetrieveInformationModel },
      { UID_MOVEStudyRootQueryRetrieveInformationModel,
        UID_FINDStudyRootQueryRetrieveInformationModel },
      { UID_RETIRED_MOVEPatientStudyOnlyQueryRetrieveInformationModel,
        UID_RETIRED_FINDPatientStudyOnlyQueryRetrieveInformationModel }
    };

    T_ASC_PresentationContextID movepid, findpid;

    for (int i = 0; i < (int)DIM_OF(queryRetrievePairs); i++) {
        movepid = ASC_findAcceptedPresentationContextID(assoc,
                            queryRetrievePairs[i].moveSyntax);
        if (movepid != 0) 
        {
            findpid = ASC_findAcceptedPresentationContextID(assoc,
                            queryRetrievePairs[i].findSyntax);
            if (findpid == 0) 
            {
                research_pacs::loggerInfo() << "Move Presentation Context but no Find (accepting for now)";
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

    ASC_getPresentationAddresses(assoc->params, peerHostName, NULL);
    ASC_getAPTitles(assoc->params, peerAETitle, myAETitle, NULL);
research_pacs::loggerDebug() << "DEBUG RLA aptitle ok";
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

        if (cond.good() &&
            msg.CommandField != DIMSE_C_ECHO_RQ)
        {
            // Veriry user's rights
            if (!DBConnection::get_instance().checkUserAuthorization(*assoc->params->DULparams.reqUserIdentNeg,
                                                                     msg.CommandField))
            {
                cond = DIMSE_BADCOMMANDTYPE;
                research_pacs::loggerError() << "Cannot handle command: 0x"
                                             << STD_NAMESPACE hex
                                             << (unsigned)msg.CommandField;
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
                research_pacs::loggerInfo() << "dispatch: late C-CANCEL-RQ, ignoring";
                break;
            default:
                /* we cannot handle this kind of message */
                cond = DIMSE_BADCOMMANDTYPE;
                research_pacs::loggerError() << "Cannot handle command: 0x"
                                             << STD_NAMESPACE hex
                                             << (unsigned)msg.CommandField;
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
        research_pacs::loggerInfo() << "Association Release";
        cond = ASC_acknowledgeRelease(assoc);
        ASC_dropSCPAssociation(assoc);
    } 
    else if (cond == DUL_PEERABORTEDASSOCIATION) 
    {
        research_pacs::loggerInfo() << "Association Aborted";
    } 
    else 
    {
        OFString temp_str;
        research_pacs::loggerError() << "DIMSE Failure (aborting association): "
                                     << DimseCondition::dump(temp_str, cond);
        /* some kind of error so abort the association */
        cond = ASC_abortAssociation(assoc);
    }

    cond = ASC_dropAssociation(assoc);
    if (cond.bad()) 
    {
        OFString temp_str;
        research_pacs::loggerError() << "Cannot Drop Association: "
                                     << DimseCondition::dump(temp_str, cond);
    }
    cond = ASC_destroyAssociation(&assoc);
    if (cond.bad()) 
    {
        OFString temp_str;
        research_pacs::loggerError() << "Cannot Destroy Association: "
                                     << DimseCondition::dump(temp_str, cond);
    }
}

void 
NetworkPACS
::force_stop()
{
    this->_forceStop = true;
}

} // namespace research_pacs
