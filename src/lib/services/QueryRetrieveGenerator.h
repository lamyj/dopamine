/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _d11a0665_fab9_4ad0_8287_043e7617d0f1
#define _d11a0665_fab9_4ad0_8287_043e7617d0f1

#include <string>

#include <mongo/client/dbclient.h>
#include <odil/AssociationParameters.h>
#include <odil/DataSet.h>
#include <odil/message/Request.h>
#include <odil/SCP.h>

#include "dbconnection/MongoDBConnection.h"

namespace dopamine
{

namespace services
{

/// @brief Base class for query/retrieve generators.
class QueryRetrieveGenerator : public odil::SCP::DataSetGenerator
{
public:
    /// @brief Constructor.
    QueryRetrieveGenerator(
        odil::AssociationParameters const & parameters,
        MongoDBConnection & db_connection);

    /// @brief Destructor.
    virtual ~QueryRetrieveGenerator();

    virtual bool done() const;
    virtual odil::DataSet get() const;

protected:
    MongoDBConnection & _db_connection;
    mongo::unique_ptr<mongo::DBClientCursor> _cursor;
    std::string _username;
    odil::DataSet _data_set;

    /**
     * @brief Fill the query terms and fields builders. Throw an
     * odil::SCP::Exception on errors.
     */
    void
    _get_query_and_fields(
        odil::message::Request const & request,
        mongo::BSONArrayBuilder & query_builder,
        mongo::BSONObjBuilder & fields_builder) const;
};

} // namespace services

} // namespace dopamine

#endif // _d11a0665_fab9_4ad0_8287_043e7617d0f1
