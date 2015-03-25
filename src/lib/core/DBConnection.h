/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
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

namespace dopamine
{

std::string const Service_All       = "*";
std::string const Service_Echo      = "Echo";
std::string const Service_Store     = "Store";
std::string const Service_Query     = "Query";
std::string const Service_Retrieve  = "Retrieve";
    
/**
 * @brief Class to create and manage the Database Connection
 */
class DBConnection
{
public:
    /// Create an instance of DBConnection
    DBConnection();
    
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
     * Check if user is allowed to perform a given Command
     * @param userIdentNeg: User identity
     * @param Request service
     * @return true if user is allowed, false otherwise
     */
    bool checkUserAuthorization(UserIdentityNegotiationSubItemRQ & userIdentNeg,
                                T_DIMSE_Command command);

    static std::string DIMSE_Command_to_Service(T_DIMSE_Command command);

    static std::string WebService_to_Service(std::string const & webservice);

protected:

private:
    /// Database connection
    mongo::DBClientConnection _connection;
    
    /// Database name
    std::string _db_name;
    
    // Purposely not implemented
    DBConnection(DBConnection const & other);
    void operator=(DBConnection const & other);

};
    
} // namespace dopamine

#endif // _f10acad4_dce9_4095_9d49_3bb176705ca6
