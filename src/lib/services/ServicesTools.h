/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _3b57890b_d730_458f_8bce_37bd2d189a9b
#define _3b57890b_d730_458f_8bce_37bd2d189a9b

#include <string>

/* make sure OS specific configuration is included first */
#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dcdatset.h>
#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/dcmdata/dcelem.h>
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcostrmb.h>
#include <dcmtk/dcmdata/dctag.h>
#include <dcmtk/dcmnet/dcuserid.h>
#include <dcmtk/dcmnet/dimse.h>
#include <dcmtk/ofstd/ofcond.h>

#include <mongo/client/dbclient.h>

namespace dopamine
{

namespace services
{

std::string const Service_All       = "*";
std::string const Service_Echo      = "Echo";
std::string const Service_Store     = "Store";
std::string const Service_Query     = "Query";
std::string const Service_Retrieve  = "Retrieve";

enum database_status
{
    NO_ERROR = 0,
    NOT_CONNECTED,
    CONVERSION_ERROR,
    NOT_ALLOW,
    INSERTION_FAILED
};

struct DataBaseInformation
{
    mongo::DBClientConnection connection;
    std::string db_name;
    std::string bulk_data;
};

bool create_db_connection(DataBaseInformation & db_information);

database_status insert_dataset(DataBaseInformation & db_information,
                               std::string const & username,
                               DcmDataset* dataset,
                               std::string const & callingaet = "");

void create_status_detail(Uint16 const & errorCode, DcmTagKey const & key,
                          OFString const & comment, DcmDataset **statusDetail);

std::string get_username(UserIdentityNegotiationSubItemRQ *userIdentNeg);

bool is_authorized(DataBaseInformation & db_information,
                   std::string const & username,
                   std::string const & servicename);

mongo::BSONObj get_constraint_for_user(DataBaseInformation & db_information,
                                       std::string const & username,
                                       std::string const & servicename);

bool is_dataset_allowed_for_storage(DataBaseInformation & db_information,
                                    std::string const & username,
                                    mongo::BSONObj const & dataset);

std::string bsonelement_to_string(mongo::BSONElement const & bsonelement);

/**
 * Replace all given pattern by another
 * @param value: given input string
 * @param old: search pattern to be replaced
 * @param new_: new pattern
 * @return string
 */
std::string replace(std::string const & value,
                    std::string const & old,
                    std::string const & new_);

mongo::BSONObj dataset_to_bson(DcmDataset * const dataset,
                               bool isforstorage = false);

DcmDataset * bson_to_dataset(DataBaseInformation & db_information,
                             mongo::BSONObj object);

std::string get_dataset_as_string(DataBaseInformation & db_information,
                                  mongo::BSONObj object);

} // namespace services

} // namespace dopamine

#endif // _3b57890b_d730_458f_8bce_37bd2d189a9b
