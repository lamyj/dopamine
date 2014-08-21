/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _f10acad4_dce9_4095_9d49_3bb176705ca6
#define _f10acad4_dce9_4095_9d49_3bb176705ca6

#include <mongo/client/dbclient.h>

#include "dcmtk/config/osconfig.h" /* make sure OS specific configuration is included first */
#include <dcmtk/dcmnet/diutil.h>

namespace research_pacs
{
    
/**
 * @brief Class to create and manage the Database Connection
 */
class DBConnection
{
public:
    /**
     * Create (if not exist) and return an unique instance of DBConnection
     * @return unique instance of DBConnection
     */
    static DBConnection & get_instance();
    
    /// Destroy the DBConnection
    virtual ~DBConnection();
    
    /**
     * Get the connection with database
     * @return database connection
     */
    mongo::DBClientConnection const & get_connection() const 
        { return this->_connection; }
    
    /**
     * Get the connection with database
     * @return database connection
     */
    mongo::DBClientConnection & get_connection() 
        { return this->_connection; }
    
    /**
     * Get the database name
     * @return database name
     */
    std::string const & get_db_name() const { return this->_db_name; }
    
    /**
     * Create the connection with the database
     */
    void connect();
    
    /**
     * Check if user is allowed to perform a given Command
     * @param userIdentNeg: User identity
     * @param Request service
     * @return true if user is allowed, false otherwise
     */
    bool checkUserAuthorization(UserIdentityNegotiationSubItemRQ & userIdentNeg,
                                T_DIMSE_Command command);

protected:

private:
    /// Create an instance of DBConnection
    DBConnection();
    
    /// Unique instance
    static DBConnection * _instance;

    /// Database connection
    mongo::DBClientConnection _connection;
    
    /// Database name
    std::string _db_name;
    
    // Purposely not implemented
    DBConnection(DBConnection const & other);
    void operator=(DBConnection const & other);

};
    
} // namespace research_pacs

#endif // _f10acad4_dce9_4095_9d49_3bb176705ca6
