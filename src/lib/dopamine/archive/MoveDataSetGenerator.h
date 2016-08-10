/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _1af29580_991c_4a6b_99be_bc3debef7c3b
#define _1af29580_991c_4a6b_99be_bc3debef7c3b

#include <string>

#include <mongo/client/dbclient.h>

#include <odil/Association.h>
#include <odil/AssociationParameters.h>
#include <odil/DataSet.h>
#include <odil/message/CMoveRequest.h>
#include <odil/message/Request.h>
#include <odil/MoveSCP.h>

#include "dopamine/AccessControlList.h"
#include "dopamine/archive/DataSetGeneratorHelper.h"

namespace dopamine
{

namespace archive
{

class MoveDataSetGenerator: public odil::MoveSCP::DataSetGenerator
{
public:
    MoveDataSetGenerator(
        mongo::DBClientConnection & connection, AccessControlList const & acl,
        std::string const & database, std::string const & bulk_database,
        odil::AssociationParameters const & parameters);

    virtual ~MoveDataSetGenerator();

    /// @brief Return the database name.
    std::string const & get_database() const;

    /// @brief Set the database name.
    void set_database(std::string const & database);

    /// @brief Return the bulk database name.
    std::string const & get_bulk_database() const;

    /// @brief Set the bulk database name.
    void set_bulk_database(std::string const & bulk_database);

    /// @brief Initialize the generator, the request must be a C-MOVE request.
    virtual void initialize(odil::message::Request const & request);

    /// @brief Test whether all elements have been generated.
    virtual bool done() const;

    /// @brief Prepare the next element.
    virtual void next();

    /// @brief Return the current element.
    virtual odil::DataSet get() const;

    /// @brief Return the number of responses.
    virtual unsigned int count() const;

    /// @brief Return the sub-association to send responses on.
    virtual odil::Association get_association(
        odil::message::CMoveRequest const & request) const;
private:
    mongo::DBClientConnection & _connection;
    AccessControlList const & _acl;

    std::string _database;
    std::string _datasets_namespace;
    std::string _peers_namespace;

    std::string _bulk_database;

    odil::AssociationParameters const & _parameters;

    DataSetGeneratorHelper _helper;

    mutable bool _dicom_data_set_up_to_date;
    mutable odil::DataSet _dicom_data_set;
};

} // namespace archive

} // namespace dopamine

#endif // _1af29580_991c_4a6b_99be_bc3debef7c3b