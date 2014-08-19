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

#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */
#include <dcmtk/dcmnet/diutil.h>

namespace research_pacs
{
    
class DBConnection
{
public:
    static DBConnection & get_instance();
    
    virtual ~DBConnection();
    
    mongo::DBClientConnection const & get_connection() const { return this->_connection; }
    mongo::DBClientConnection & get_connection() { return this->_connection; }
    
    std::string const & get_db_name() const { return this->_db_name; }
    
    void connect();
    
    bool checkUserAuthorization(UserIdentityNegotiationSubItemRQ & userIdentNeg,
                                T_DIMSE_Command command);

protected:

private:
    DBConnection();
    
    static DBConnection * _instance;

    mongo::DBClientConnection _connection;
    std::string _db_name;
    
    // Purposely not implemented
    DBConnection(DBConnection const & other);
    void operator=(DBConnection const & other);

};
    
} // namespace research_pacs

#endif // _f10acad4_dce9_4095_9d49_3bb176705ca6
