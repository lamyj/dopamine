/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <sstream>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>

#include "ConfigurationPACS.h"
#include "core/LoggerPACS.h"
#include "ExceptionPACS.h"
#include "NetworkPACS.h"
#include "services/SCP/EchoSCP.h"
#include "services/SCP/FindSCP.h"
#include "services/SCP/GetSCP.h"
#include "services/SCP/MoveSCP.h"
#include "services/SCP/StoreSCP.h"
#include "services/ServicesTools.h"

namespace dopamine
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
    _force_stop(false),
    _timeout(1000)
{
    this->_create_authenticator();

    services::create_db_connection(this->_database_information);
    
    int port = std::atoi(ConfigurationPACS::
                         get_instance().get_value("dicom.port").c_str());
    
    OFCondition condition = ASC_initializeNetwork(NET_ACCEPTORREQUESTOR,
                                                  (int)port, 30,
                                                  &this->_network);
    if (condition.bad())
    {
        std::stringstream stream;
        stream << "cannot initialize network: " << condition.text();
        throw ExceptionPACS(stream.str());
    }
}

NetworkPACS
::~NetworkPACS()
{
    OFCondition condition = ASC_dropNetwork(&this->_network);
    if (condition.bad())
    {
        std::stringstream stream;
        stream << "cannot drop network: " << condition.text();
        throw ExceptionPACS(stream.str());
    }
}

void
NetworkPACS
::_create_authenticator()
{
    ConfigurationPACS & instance = ConfigurationPACS::get_instance();
    std::string const type = instance.get_value("authenticator.type");
    if(type == "CSV")
    {
        this->_authenticator = new authenticator::AuthenticatorCSV
            (instance.get_value("authenticator.filepath"));
    }
    else if (type == "LDAP")
    {
        this->_authenticator = new authenticator::AuthenticatorLDAP
            (
                instance.get_value("authenticator.ldap_server"),
                instance.get_value("authenticator.ldap_bind_user"),
                instance.get_value("authenticator.ldap_base"),
                instance.get_value("authenticator.ldap_filter")
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
    while (!this->_force_stop)
    {
        if (ASC_associationWaiting(this->_network, this->_timeout))
        {
            bool continue_ = true;
            T_ASC_Association * assoc;
            OFCondition condition = ASC_receiveAssociation(this->_network,
                                                           &assoc,
                                                           ASC_DEFAULTMAXPDU);
            if (condition.bad())
            {
                dopamine::logger_error() << "Failed to receive association: "
                                             << condition.text();
            }
            else
            {
                time_t t = time(NULL);
                dopamine::logger_info()
                        << "Association Received ("
                        << assoc->params->DULparams.callingPresentationAddress
                        << ":" << assoc->params->DULparams.callingAPTitle
                        << " -> "
                        << assoc->params->DULparams.calledAPTitle << ") "
                        << ctime(&t);

                OFString temp_str;
                ASC_dumpParameters(temp_str, assoc->params, ASC_ASSOC_RQ);
                
                // process
                
                if (continue_)
                {
                    // Authentication User / Password
                    if( ! (*this->_authenticator)
                           (assoc->params->DULparams.reqUserIdentNeg))
                    {
                        dopamine::logger_warning()
                                << "Refusing Association: Bad Authentication";
                        this->_refuse_association(&assoc, CTN_NoReason);
                        continue_ = false;
                    }
                    dopamine::logger_debug() << "Authentication Status: "
                                             << continue_;
                }
                
                if (continue_)
                {
                    /* Application Context Name */
                    char buf[BUFSIZ];
                    condition = ASC_getApplicationContextName(assoc->params,
                                                              buf);
                    if (condition.bad() ||
                        std::string(buf) != DICOM_STDAPPLICATIONCONTEXT)
                    {
                        /* reject: the application context name
                         * is not supported */
                        dopamine::logger_warning() << "Bad AppContextName: "
                                                   << buf;
                        this->_refuse_association(&assoc, CTN_BadAppContext);
                        continue_ = false;
                    }
                    dopamine::logger_debug()
                            << "Application Context Name Status: "
                            << continue_;
                }
                
                if (continue_)
                {
                    /* Does peer AE have access to required service ?? */
                    if (! ConfigurationPACS::get_instance().peer_in_aetitle
                                (assoc->params->DULparams.callingAPTitle))
                    {
                        dopamine::logger_warning() << "Bad AE Service";
                        this->_refuse_association(&assoc, CTN_BadAEService);
                        continue_ = false;
                    }
                    dopamine::logger_debug() << "AE Service Status: "
                                             << continue_;
                }
                
                if (continue_)
                {
                    condition = this->_negotiate_association(assoc);
                    if (condition.good() || condition == ASC_SHUTDOWNAPPLICATION)
                    {
                        bool shutdown = condition == ASC_SHUTDOWNAPPLICATION;
                        condition = ASC_acknowledgeAssociation(assoc);
                        if (condition.good())
                        {
                            // dispatch
                            this->_handle_association(assoc);
                        }
                        if (shutdown)
                        {
                            // shutdown
                            break;
                        }
                    }

                    if (condition.bad())
                    {
                        dopamine::logger_warning() << "Association error: "
                                                       << condition.text();
                    }

                    continue_ = condition.good();
                }
            }

            // cleanup code
            if (!continue_)
            {
                /* the child will handle the association, we can drop it */
                condition = ASC_dropAssociation(assoc);
                if (condition.bad())
                {
                    std::stringstream stream;
                    stream << "Cannot Drop Association: " << condition.text();
                    throw ExceptionPACS(stream.str());
                }
                condition = ASC_destroyAssociation(&assoc);
                if (condition.bad())
                {
                    std::stringstream stream;
                    stream << "Cannot Destroy Association: " << condition.text();
                    throw ExceptionPACS(stream.str());
                }
            }
        }
    }
}

void
NetworkPACS
::_refuse_association(T_ASC_Association ** assoc, CTN_RefuseReason reason)
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
    dopamine::logger_error() << "Refusing Association ("
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

    OFCondition condition = ASC_rejectAssociation(*assoc, &rej);
    if (condition.bad())
    {
        std::stringstream stream;
        stream << "Association Reject Failed: " << condition.text();
        throw ExceptionPACS(stream.str());
    }

    condition = ASC_dropAssociation(*assoc);
    if (condition.bad())
    {
        std::stringstream stream;
        stream << "Cannot Drop Association: " << condition.text();
        throw ExceptionPACS(stream.str());
    }
    condition = ASC_destroyAssociation(assoc);
    if (condition.bad())
    {
        std::stringstream stream;
        stream << "Cannot Destroy Association: " << condition.text();
        throw ExceptionPACS(stream.str());
    }
}

mongo::DBClientConnection const &
NetworkPACS
::get_connection() const
{
    return this->_database_information.connection;
}

mongo::DBClientConnection &
NetworkPACS
::get_connection()
{
    return this->_database_information.connection;
}

std::string const &
NetworkPACS
::get_db_name() const
{
    return this->_database_information.db_name;
}

std::string const &
NetworkPACS
::get_bulk_data_db() const
{
    return this->_database_information.bulk_data;
}

OFCondition
NetworkPACS
::_negotiate_association(T_ASC_Association * assoc)
{
    DIC_AE calledAETitle;
    OFCondition condition = ASC_getAPTitles(assoc->params, NULL,
                                            calledAETitle, NULL);
    if (condition.bad())
    {
        dopamine::logger_error() << "Cannot retrieve AP Titles";
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
    condition = ASC_acceptContextsWithPreferredTransferSyntaxes(
                    assoc->params,
                    (const char**)nonStorageSyntaxes,
                    numberOfNonStorageSyntaxes,
                    (const char**)transferSyntaxes,
                    numTransferSyntaxes,
                    ASC_SC_ROLE_SCUSCP);
    if (condition.bad())
    {
        OFString temp_str;
        dopamine::logger_info() << "Cannot accept presentation contexts: "
                                    << DimseCondition::dump(temp_str, condition);
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
             * We are prepared to accept whatever role he proposes.
             * Normally we can be the SCP of the Storage Service Class.
             * When processing the C-GET operation we can be the SCU of
             * the Storage Service Class.
            **/
            T_ASC_SC_ROLE role = pc.proposedRole;

            /*
             * Accept in the order "least wanted" to "most wanted" transfer
             * syntax.  Accepting a transfer syntax will override previously
             * accepted transfer syntaxes.
            **/
            for (int k = numTransferSyntaxes - 1; k >= 0; k--)
            {
                for (int j = 0; j < (int)pc.transferSyntaxCount; j++)
                {
                    /* if the transfer syntax was proposed then we can accept it
                    * appears in our supported list of transfer syntaxes
                    */
                    if (std::string(pc.proposedTransferSyntaxes[j]) ==
                        std::string(transferSyntaxes[k]))
                    {
                        condition = ASC_acceptPresentationContext(
                                        assoc->params,
                                        pc.presentationContextID,
                                        transferSyntaxes[k], role);
                        if (condition.bad()) return condition;
                    }
                }
            }
        }
    } /* for */

    /*
    ** check if we have negotiated the private "shutdown" SOP Class
    **/
    if (0 != ASC_findAcceptedPresentationContextID(assoc,
                                                   UID_PrivateShutdownSOPClass))
    {
        dopamine::logger_info()
                << "Shutting down server ... "
                << "(negotiated private \"shut down\" SOP class)";
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
                dopamine::logger_info()
                        << "Move Presentation Context but no Find "
                        << "(accepting for now)";
            }
        }
    }

    return condition;
}

void
NetworkPACS
::_handle_association(T_ASC_Association * assoc)
{
    DIC_NODENAME        peer_host_name;
    DIC_AE              peer_aetitle;
    DIC_AE              this_aetitle;

    ASC_getPresentationAddresses(assoc->params, peer_host_name, NULL);
    ASC_getAPTitles(assoc->params, peer_aetitle, this_aetitle, NULL);

    /* this while loop is executed exactly once unless
    ** the "keepDBHandleDuringAssociation_" flag is not set,
    ** in which case the inner loop is executed only once and
    ** this loop repeats for each incoming DIMSE command. In this case,
    ** the DB handle is created and released for each DIMSE command.
    */
    OFCondition condition = EC_Normal;
    while (condition.good())
    {
        T_ASC_PresentationContextID presentation_context_id;
        T_DIMSE_Message msg;
        condition = DIMSE_receiveCommand(assoc, DIMSE_BLOCKING, 0,
                                         &presentation_context_id, &msg, NULL);

        /* did peer release, abort, or do we have a valid message ? */
        if (condition.good())
        {
            /* process command */
            switch (msg.CommandField)
            {
                case DIMSE_C_ECHO_RQ:
                {
                    services::EchoSCP echoscp(assoc, presentation_context_id,
                                              &msg.msg.CEchoRQ);
                    condition = echoscp.process();
                    break;
                }
                case DIMSE_C_STORE_RQ:
                {
                    services::StoreSCP storescp(assoc, presentation_context_id,
                                                &msg.msg.CStoreRQ);
                    condition = storescp.process();
                    break;
                }
                case DIMSE_C_FIND_RQ:
                {
                    services::FindSCP findscp(assoc, presentation_context_id,
                                              &msg.msg.CFindRQ);
                    condition = findscp.process();
                    break;
                }
                case DIMSE_C_GET_RQ:
                {
                    services::GetSCP getscp(assoc, presentation_context_id,
                                            &msg.msg.CGetRQ);
                    condition = getscp.process();
                    break;
                }
                case DIMSE_C_MOVE_RQ:
                {
                    services::MoveSCP movescp(assoc, presentation_context_id,
                                              &msg.msg.CMoveRQ);
                    movescp.set_network(this->_network);
                    condition = movescp.process();
                    break;
                }
                case DIMSE_C_CANCEL_RQ:
                    /* This is a late cancel request, just ignore it */
                    dopamine::logger_info()
                            << "dispatch: late C-CANCEL-RQ, ignoring";
                    break;
                default:
                    /* we cannot handle this kind of message */
                    condition = DIMSE_BADCOMMANDTYPE;
                    dopamine::logger_error() << "Cannot handle command: 0x"
                                             << STD_NAMESPACE hex
                                             << (unsigned)msg.CommandField;
                    /* the condition will be returned,
                     * the caller will abort the association.
                    */
            }
        }
        /*else if ((condition == DUL_PEERREQUESTEDRELEASE) ||
                 (condition == DUL_PEERABORTEDASSOCIATION))
        {
            // association gone
        }*/
        else
        {
            /* the condition will be returned,
             * the caller will abort the association.
            */
        }
    }
    
    /* clean up on association termination */
    if (condition == DUL_PEERREQUESTEDRELEASE)
    {
        dopamine::logger_info() << "Association Release";
        condition = ASC_acknowledgeRelease(assoc);
        ASC_dropSCPAssociation(assoc);
    }
    else if (condition == DUL_PEERABORTEDASSOCIATION)
    {
        dopamine::logger_info() << "Association Aborted";
    }
    else
    {
        OFString temp_str;
        dopamine::logger_error() << "DIMSE Failure (aborting association): "
                                     << DimseCondition::dump(temp_str,
                                                             condition);
        /* some kind of error so abort the association */
        condition = ASC_abortAssociation(assoc);
    }

    condition = ASC_dropAssociation(assoc);
    if (condition.bad())
    {
        OFString temp_str;
        dopamine::logger_error() << "Cannot Drop Association: "
                                     << DimseCondition::dump(temp_str,
                                                             condition);
    }
    condition = ASC_destroyAssociation(&assoc);
    if (condition.bad())
    {
        OFString temp_str;
        dopamine::logger_error() << "Cannot Destroy Association: "
                                     << DimseCondition::dump(temp_str,
                                                             condition);
    }
}

void
NetworkPACS
::force_stop()
{
    this->_force_stop = true;
}

} // namespace dopamine
