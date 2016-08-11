/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _dee83091_6560_47f5_b6e2_a08dd2474ee7
#define _dee83091_6560_47f5_b6e2_a08dd2474ee7

#include <string>

#include <mongo/client/dbclient.h>

#include <odil/AssociationParameters.h>
#include <odil/DataSet.h>
#include <odil/message/Request.h>
#include <odil/GetSCP.h>

#include "dopamine/AccessControlList.h"
#include "dopamine/archive/DataSetGeneratorHelper.h"

namespace dopamine
{

namespace archive
{

class GetDataSetGenerator: public odil::GetSCP::DataSetGenerator
{
public:
    GetDataSetGenerator(
        mongo::DBClientConnection & connection, AccessControlList const & acl,
        std::string const & database, std::string const & bulk_database,
        odil::AssociationParameters const & parameters);

    virtual ~GetDataSetGenerator();

    /// @brief Initialize the generator, the request must be a C-GET request.
    virtual void initialize(odil::message::Request const & request);

    /// @brief Test whether all elements have been generated.
    virtual bool done() const;

    /// @brief Prepare the next element.
    virtual void next();

    /// @brief Return the current element.
    virtual odil::DataSet get() const;

    /// @brief Return the number of responses.
    virtual unsigned int count() const;
private:
    mongo::DBClientConnection & _connection;
    AccessControlList const & _acl;

    std::string _namespace;

    odil::AssociationParameters const & _parameters;

    DataSetGeneratorHelper _helper;

    mutable bool _dicom_data_set_up_to_date;
    mutable odil::DataSet _dicom_data_set;
};

} // namespace archive

} // namespace dopamine

#endif // _dee83091_6560_47f5_b6e2_a08dd2474ee7
