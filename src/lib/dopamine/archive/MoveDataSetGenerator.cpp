/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "dopamine/archive/MoveDataSetGenerator.h"

#include <string>

#include <mongo/bson/bson.h>
#include <mongo/client/dbclient.h>

#include <odil/Association.h>
#include <odil/AssociationParameters.h>
#include <odil/DataSet.h>
#include <odil/Exception.h>
#include <odil/message/CMoveRequest.h>
#include <odil/message/Request.h>
#include <odil/MoveSCP.h>

#include "dopamine/AccessControlList.h"
#include "dopamine/archive/DataSetGeneratorHelper.h"
#include "dopamine/utils.h"

namespace dopamine
{

namespace archive
{

MoveDataSetGenerator
::MoveDataSetGenerator(
    mongo::DBClientConnection & connection, AccessControlList const & acl,
    std::string const & database, std::string const & bulk_database,
    odil::AssociationParameters const & parameters)
: _connection(connection), _acl(acl), _parameters(parameters),
  _helper(
    connection, acl, database, bulk_database, get_principal(parameters),
    "Retrieve")
{
    this->_datasets_namespace = database+".datasets";
    this->_peers_namespace = database+".application_entities";
}

MoveDataSetGenerator
::~MoveDataSetGenerator()
{
    // Nothing to do.
}

void
MoveDataSetGenerator
::initialize(odil::message::Request const & request)
{
    this->_helper.check_acl();

    odil::message::CMoveRequest const move_request(request);
    auto data_set = move_request.get_data_set();
    if(!data_set.has(odil::registry::SOPInstanceUID))
    {
        data_set.add(odil::registry::SOPInstanceUID);
    }

    mongo::BSONObjBuilder condition_builder;
    mongo::BSONObjBuilder projection_builder;
    this->_helper.get_condition_and_projection(
        data_set, condition_builder, projection_builder);

    auto const condition = condition_builder.obj();
    auto const projection = BSON(
        std::string(odil::registry::SOPInstanceUID) << 1
        << std::string(odil::registry::TransferSyntaxUID) << 1
        << "Content" << 1);

    auto const cursor = this->_connection.query(
        this->_datasets_namespace, condition, 0, 0, &projection);
    std::vector<mongo::BSONObj> results;
    while(cursor->more())
    {
        auto const object = cursor->next().getOwned();
        results.push_back(object);
    }
    this->_helper.set_results(results);

    this->_dicom_data_set_up_to_date = false;
}

bool
MoveDataSetGenerator
::done() const
{
    return this->_helper.done();
}

void
MoveDataSetGenerator
::next()
{
    this->_helper.next();
    this->_dicom_data_set_up_to_date = false;
}

odil::DataSet
MoveDataSetGenerator
::get() const
{
    if(!this->_dicom_data_set_up_to_date)
    {
        auto const current = this->_helper.get();
        this->_dicom_data_set = this->_helper.retrieve(
            current[std::string(odil::registry::SOPInstanceUID)].String());
        this->_dicom_data_set_up_to_date = true;
    }

    return this->_dicom_data_set;
}

unsigned int
MoveDataSetGenerator
::count() const
{
    return this->_helper.count();
}

odil::Association
MoveDataSetGenerator
::get_association(odil::message::CMoveRequest const & request) const
{
    auto const peer = this->_connection.findOne(
        this->_peers_namespace,
        BSON("ae_title" << request.get_move_destination()));
    if(peer.isEmpty())
    {
        throw odil::Exception("Unknown move destination");
    }

    odil::Association association;
    association.set_peer_host(peer["host"].String());
    association.set_peer_port(peer["port"].Int());

    return association;
}

} // namespace archive

} // namespace dopamine
