/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "dopamine/AccessControlList.h"

#include <string>
#include <vector>

#include <mongo/bson/bson.h>
#include <mongo/client/dbclient.h>
#include <odil/DataSet.h>

//#include "ConverterBSON/bson_converter.h"
#include "dopamine/Exception.h"

namespace dopamine
{

AccessControlList::Entry
::Entry(
    std::string const & principal, std::string const & service,
    mongo::BSONObj const & constraint)
: principal(principal), service(service), constraint(constraint)
{
    // Nothing else
}

AccessControlList
::AccessControlList(
    mongo::DBClientConnection & connection,
    std::string const & database)
: _connection(connection), _database()
{
    this->set_database(database);
}

AccessControlList
::~AccessControlList()
{
    // Nothing to do.
}

std::string const &
AccessControlList
::get_database() const
{
    return this->_database;
}

void
AccessControlList
::set_database(std::string const & database)
{
    this->_database = database;
    this->_namespace = database+".authorization";
}

std::vector<AccessControlList::Entry>
AccessControlList
::get_entries() const
{
    std::vector<AccessControlList::Entry> result;
    auto cursor = this->_connection.query(this->_namespace);
    while(cursor->more())
    {
        auto const response = cursor->next();
        result.emplace_back(
            response["principal_name"].String(),
            response["service"].String(),
            response["dataset"].Obj());
    }

    return result;
}

void
AccessControlList
::set_entries(std::vector<Entry> const & entries)
{
    this->_connection.remove(this->_namespace, mongo::Query());
    for(auto const & entry: entries)
    {
        this->_connection.insert(
            this->_namespace, BSON(
                "principal_name" << entry.principal <<
                "service" << entry.service <<
                "dataset" << entry.constraint));
    }
}

bool
AccessControlList
::is_allowed(std::string const & principal, std::string const & service) const
{
    // Return at most one record since we just need to know if there is a
    // matching record
    auto cursor = this->_connection.query(
        this->_namespace, this->_get_query(principal, service), 1);
    return (cursor->more());
}

mongo::BSONObj
AccessControlList
::get_constraints(
    std::string const & principal, std::string const & service) const
{
    mongo::BSONObj result;

    auto cursor = this->_connection.query(
        this->_namespace, this->_get_query(principal, service));
    if(cursor->more())
    {
        mongo::BSONArrayBuilder constraints;
        bool allow_all = false;

        while(cursor->more())
        {
            auto const item = cursor->next();
            auto const data_set = item["dataset"];

            if(
                data_set.type() == mongo::String && data_set.String() == "")
            {
                allow_all = true;
                break;
            }
            else if(
                data_set.type() == mongo::Object && data_set.Obj().isEmpty())
            {
                allow_all = true;
                break;
            }

            // Assume data_set is an object
            mongo::BSONArrayBuilder constraint;
            for(auto it=data_set.Obj().begin(); it.more(); /* nothing */)
            {
                auto const element = it.next();
                mongo::BSONObjBuilder term;
                if(element.type() == mongo::BSONType::RegEx)
                {
                    term.appendRegex(
                        std::string(element.fieldName())+".Value",
                        element.regex());
                }
                else
                {
                    term << std::string(element.fieldName())+".Value" << element;
                }

                constraint.append(term.obj());
            }

            constraints << BSON("$and" << constraint.arr());
        }

        if(!allow_all)
        {
            result = BSON("$or" << constraints.arr());
        }
    }

    return result;
}

mongo::BSONObj
AccessControlList
::_get_query(std::string const & principal, std::string const & service)
{
    mongo::BSONObjBuilder query;

    if(!principal.empty())
    {
        query
            << "principal_name" << BSON("$in" << BSON_ARRAY(principal << "*"));
    }
    else
    {
        query << "principal_name" << "";
    }

    query << "service" << service;

    return query.obj();
}

}
