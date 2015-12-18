/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _aad1a7c4_1d62_42e8_af93_56cd10ee68d0
#define _aad1a7c4_1d62_42e8_af93_56cd10ee68d0

#include <string>
#include <vector>

#include <dcmtkpp/message/Message.h>

#include <mongo/client/dbclient.h>

namespace dopamine
{

class MongoDBConnection
{
public:
    struct DataBaseInformation
    {
        mongo::DBClientConnection connection;
        std::string db_name;
        std::string bulk_data;

        DataBaseInformation():
            db_name(""), bulk_data("")
        {
            // Nothing else.
        }

        ~DataBaseInformation()
        {
            // Nothing to do.
        }

        DataBaseInformation(DataBaseInformation const & other)
        {
            db_name = other.db_name;
            bulk_data = other.bulk_data;
        }

        DataBaseInformation& operator=(DataBaseInformation const & other)
        {
            db_name = other.db_name;
            bulk_data = other.bulk_data;
        }

    };

    MongoDBConnection(DataBaseInformation const & db_information = DataBaseInformation(),
                      std::string const & host_name = "localhost",
                      int port = -1,
                      std::vector<std::string> const & indexes = {});

    virtual ~MongoDBConnection();

    mongo::DBClientConnection const & get_connection() const;

    mongo::DBClientConnection & get_connection();

    /// @brief Return the name of the database holding the meta-data.
    std::string const & get_db_name() const;

    void set_db_name(std::string const & db_name);

    /// @brief Return the name of the database holding the bulk data.
    std::string const & get_bulk_data_db() const;

    std::string get_host_name() const;

    void set_host_name(std::string const & host_name);

    int get_port() const;

    void set_port(int port);

    std::vector<std::string> get_indexes() const;

    void set_indexes(std::vector<std::string> const & indexes);

    bool connect();

    bool is_authorized(std::string const & user,
                       dcmtkpp::message::Message::Command::Type command);

    mongo::BSONObj get_constraints(
            std::string const & user,
            dcmtkpp::message::Message::Command::Type command);

    mongo::unique_ptr<mongo::DBClientCursor> get_datasets_cursor(
            mongo::Query const & query = mongo::Query(), int nToReturn = 0,
            int nToSkip = 0, mongo::BSONObj const * fieldsToReturn = NULL);

    bool run_command(mongo::BSONObj const & command, mongo::BSONObj & response);

    dcmtkpp::Value::Integer insert_dataset(std::string const & username,
                                           dcmtkpp::DataSet const & dataset,
                                           std::string const & callingaet);

    std::pair<dcmtkpp::DataSet, dcmtkpp::DataSet> get_dataset(
            mongo::BSONObj const & object);

    /**
     * @brief Convert the value of a BSONElement to a std::string
     * @param bsonelement: element to convert
     * @return value as string
     */
    static std::string as_string(mongo::BSONElement const & bsonelement);

    static std::string as_string(
            dcmtkpp::message::Message::Command::Type command);

private:
    /**
     * @brief Retrieve Dataset from Database
     * @param connection: connection to the database
     * @param db_name: Database Name
     * @param object: to retrieve
     * @return the Dataset as string
     */
    std::string dataset_as_string(mongo::BSONObj const & object);

    bool is_dataset_allowed_for_storage(std::string const & username,
                                        mongo::BSONObj const & dataset);

    /// Database information
    DataBaseInformation _database_information;

    /// Database Host name
    std::string _host_name;

    /// Database Port
    int _port;

    /// Database Indexes
    std::vector<std::string> _indexes;

};

} // namespace dopamine

#endif // _aad1a7c4_1d62_42e8_af93_56cd10ee68d0
