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

#include <dcmtkpp/DataSet.h>
#include <dcmtkpp/message/Message.h>
#include <dcmtkpp/Tag.h>
#include <dcmtkpp/Value.h>

#include <mongo/client/dbclient.h>

namespace dopamine
{

namespace services
{

/**
 * @brief Create connection to the database
 * @param connection: return the connection to the database
 * @param db_name: return the database name
 * @return true if connection is established, false otherwise
 */
bool create_db_connection(mongo::DBClientConnection & connection,
                          std::string & db_name);

/**
 * @brief Insert a Dataset into Database
 * @param connection: connection to the database
 * @param db_name: Database Name
 * @param username: User name
 * @param dataset: Dataset to insert
 * @param callingaet: Calling AE Title
 * @return status of the operation
 */
dcmtkpp::Value::Integer insert_dataset(
        mongo::DBClientConnection & connection,
        std::string const & db_name,
        std::string const & username,
        dcmtkpp::DataSet const & dataset,
        std::string const & callingaet = "");

/**
 * @brief Create the status detail Dataset
 * @param errorCode: Error code
 * @param key: Tag in error
 * @param comment: error detail
 * @return the status detail Dataset
 */
dcmtkpp::DataSet create_status_detail(Uint16 const errorCode,
                                      dcmtkpp::Tag const & key,
                                      std::string const & comment);

/**
 * @brief Check if user is allowed to do given services
 * @param connection: connection to the database
 * @param db_name: Database name
 * @param username: user name
 * @param servicename: service name
 * @return true if user is allowed, false otherwise
 */
bool is_authorized(mongo::DBClientConnection &connection,
                   std::string const & db_name,
                   std::string const & username,
                   dcmtkpp::message::Message::Command::Type command_type);

/**
 * @brief Get the constraint for a given user and a given service
 * @param connection: connection to the database
 * @param db_name: Database Name
 * @param username: user name
 * @param servicename: service name
 * @return the constraints
 */
mongo::BSONObj get_constraint_for_user(
        mongo::DBClientConnection &connection,
        std::string const & db_name,
        std::string const & username,
        dcmtkpp::message::Message::Command::Type command_type);

/**
 * @brief Check if user is allowed to store dataset
 * @param connection: connection to the database
 * @param db_name: Database Name
 * @param username: user name
 * @param dataset: dataset to store
 * @return true if dataset can be store, false otherwise
 */
bool is_dataset_allowed_for_storage(mongo::DBClientConnection & connection,
                                    std::string const & db_name,
                                    std::string const & username,
                                    mongo::BSONObj const & dataset);

/**
 * @brief Convert the value of a BSONElement to a std::string
 * @param bsonelement: element to convert
 * @return value as string
 */
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

/**
 * @brief Convert a Dataset to a BSON Object
 * @param dataset: Dataset to convert
 * @param isforstorage: flag indicating if object will be stored into database
 * @return Dataset as BSON object
 */
mongo::BSONObj dataset_to_bson(dcmtkpp::DataSet const & dataset,
                               bool isforstorage = false);

/**
 * @brief Retrieve Dataset from Database
 * @param connection: connection to the database
 * @param db_name: Database Name
 * @param object: to retrieve
 * @return Dataset
 */
std::pair<dcmtkpp::DataSet, dcmtkpp::DataSet> bson_to_dataset(
        mongo::DBClientConnection &connection,
        std::string const & db_name,
        mongo::BSONObj object);

/**
 * @brief Retrieve Dataset from Database
 * @param connection: connection to the database
 * @param db_name: Database Name
 * @param object: to retrieve
 * @return the Dataset as string
 */
std::string get_dataset_as_string(mongo::DBClientConnection &connection,
                                  std::string const & db_name,
                                  mongo::BSONObj object);

} // namespace services

} // namespace dopamine

#endif // _3b57890b_d730_458f_8bce_37bd2d189a9b
