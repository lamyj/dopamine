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

#include <dcmtk/config/osconfig.h> /* make sure OS specific configuration is included first */
#include <dcmtk/dcmdata/dcdatset.h>
#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/dcmdata/dcelem.h>
#include <dcmtk/dcmdata/dctag.h>
#include <dcmtk/dcmnet/dcuserid.h>
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

bool create_db_connection(mongo::DBClientConnection & connection,
                          std::string & db_name);

void createStatusDetail(Uint16 const & errorCode, DcmTagKey const & key,
                        OFString const & comment, DcmDataset **statusDetail);

std::string get_username(UserIdentityNegotiationSubItemRQ *userIdentNeg);

bool is_authorized(mongo::DBClientConnection &connection,
                   std::string const & db_name,
                   std::string const & username,
                   std::string const & servicename);

mongo::BSONObj get_constraint_for_user(mongo::DBClientConnection &connection,
                                       std::string const & db_name,
                                       std::string const & username,
                                       std::string const & servicename);

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

DcmDataset * bson_to_dataset(mongo::BSONObj object);

} // namespace services

} // namespace dopamine

#endif // _3b57890b_d730_458f_8bce_37bd2d189a9b
