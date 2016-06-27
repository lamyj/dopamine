/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _abef9025_9f25_4a3f_bc82_695c873ee02a
#define _abef9025_9f25_4a3f_bc82_695c873ee02a

#include <odil/AssociationParameters.h>
#include <odil/message/Request.h>

#include "dbconnection/MongoDBConnection.h"
#include "services/QueryRetrieveGenerator.h"

namespace dopamine
{

namespace services
{

/**
 * @brief \class Response Generator for C-GET services.
 */
class RetrieveGenerator : public QueryRetrieveGenerator
{
public:
    /// @brief Constructor.
    RetrieveGenerator(
        odil::AssociationParameters const & parameters,
        MongoDBConnection & db_connection);

    /// @brief Destructor.
    virtual ~RetrieveGenerator();

    virtual void initialize(odil::message::Request const & request);
    virtual void next();
};

} // namespace services

} // namespace dopamine

#endif // _abef9025_9f25_4a3f_bc82_695c873ee02a
