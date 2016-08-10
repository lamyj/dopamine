/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _28a4833c_45e9_4de8_a7ca_f45491e6410d
#define _28a4833c_45e9_4de8_a7ca_f45491e6410d

#include <functional>
#include <map>
#include <string>
#include <vector>

#include <mongo/client/dbclient.h>
#include <odil/AssociationParameters.h>
#include <odil/DataSet.h>
#include <odil/message/Request.h>
#include <odil/SCP.h>
#include <odil/Tag.h>

#include "dopamine/AccessControlList.h"
#include "dopamine/archive/DataSetGeneratorHelper.h"

namespace dopamine
{

namespace archive
{

class QueryDataSetGenerator: public odil::SCP::DataSetGenerator
{
public:
    QueryDataSetGenerator(
        mongo::DBClientConnection & connection, AccessControlList const & acl,
        std::string const & database,
        odil::AssociationParameters const & parameters);

    virtual ~QueryDataSetGenerator();

    /// @brief Return the database name.
    std::string const & get_database() const;

    /// @brief Set the database name.
    void set_database(std::string const & database);

    /// @brief Initialize the generator, the request must be a C-FIND request.
    virtual void initialize(odil::message::Request const & request);

    /// @brief Test whether all elements have been generated.
    virtual bool done() const;

    /// @brief Prepare the next element.
    virtual void next();

    /// @brief Return the current element.
    virtual odil::DataSet get() const;
private:

    typedef
        std::function<
            void(QueryDataSetGenerator const *, odil::DataSet &)>
            AttributeCalculator;

    static std::map<odil::Tag, AttributeCalculator> const _attribute_calculators;

    mongo::DBClientConnection & _connection;
    AccessControlList const & _acl;

    std::string _database;
    std::string _namespace;

    odil::AssociationParameters const & _parameters;

    std::vector<odil::Tag> _additional_attributes;

    DataSetGeneratorHelper _helper;

    mutable bool _dicom_data_set_up_to_date;
    mutable odil::DataSet _dicom_data_set;

    static std::map<odil::Tag, AttributeCalculator> _create_attribute_calculators();

    void _xxx_in_yyy(
        odil::DataSet & data_set,
        odil::Tag const & primary, odil::Tag const & secondary,
        odil::Tag const & destination) const;

    void _number_of_xxx_related_yyy(
        odil::DataSet & data_set,
        odil::Tag const & primary, odil::Tag const & secondary,
        odil::Tag const & destination) const;

    void _NumberOfPatientRelatedStudies(odil::DataSet & data_set) const;
    void _NumberOfPatientRelatedSeries(odil::DataSet & data_set) const;
    void _NumberOfPatientRelatedInstances(odil::DataSet & data_set) const;
    void _NumberOfStudyRelatedSeries(odil::DataSet & data_set) const;
    void _NumberOfStudyRelatedInstances(odil::DataSet & data_set) const;
    void _NumberOfSeriesRelatedInstances(odil::DataSet & data_set) const;
    void _ModalitiesInStudy(odil::DataSet & data_set) const;
    void _SOPClassesInStudy(odil::DataSet & data_set) const;
};

}

}

#endif // _28a4833c_45e9_4de8_a7ca_f45491e6410d
