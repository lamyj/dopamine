/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "fixtures/SampleData.h"

#include <algorithm>
#include <functional>
#include <string>
#include <vector>

#include <mongo/bson/bson.h>

#include <odil/DataSet.h>
#include <odil/registry.h>
#include <odil/Value.h>
#include <odil/VR.h>

#include "dopamine/archive/Storage.h"

#include "fixtures/Authorization.h"

namespace fixtures
{

SampleData
::SampleData()
{
    auto entries = this->acl.get_entries();
    entries.emplace_back(
        "restricted_query", "Query",
        BSON(
            std::string(odil::registry::PatientName)+".Value.Alphabetic"
            << "Patient 1"));
    this->acl.set_entries(entries);

    this->_populate();
}

SampleData
::~SampleData()
{
    // Nothing to do.
}

bool
SampleData
::less(odil::DataSet const & x, odil::DataSet const & y)
{
    std::vector<odil::Tag> const tags{
        odil::registry::PatientID, odil::registry::StudyInstanceUID,
        odil::registry::SeriesInstanceUID, odil::registry::SOPInstanceUID
    };

    std::vector<std::string> x_values;
    std::vector<std::string> y_values;
    for(auto const & tag: tags)
    {
        if(x.has(tag))
        {
            x_values.push_back(x.as_string(tag, 0));
        }
        if(y.has(tag))
        {
            y_values.push_back(y.as_string(tag, 0));
        }
    }

    return std::lexicographical_compare(
        x_values.begin(), x_values.end(), y_values.begin(), y_values.end(),
        std::less<std::string>());
}

void
SampleData
::_populate()
{
    dopamine::archive::Storage storage(this->connection, this->database);

    std::vector<std::string> const modalities{"MR", "CT"};
    std::vector<std::string> const sop_classes{
        odil::registry::MRImageStorage, odil::registry::CTImageStorage };

    for(int patient_index=1; patient_index<=2; ++patient_index)
    {
        for(int study_index=1; study_index<=patient_index; ++study_index)
        {
            for(int series_index=1; series_index<=study_index; ++series_index)
            {
                for(int instance_index=1; instance_index<=series_index; ++instance_index)
                {
                    odil::DataSet data_set;

                    auto const patient = std::to_string(patient_index);
                    auto const study = patient+"."+std::to_string(study_index);
                    auto const series = study+"."+std::to_string(series_index);
                    auto const instance = series+"."+std::to_string(instance_index);

                    data_set.add(
                        odil::registry::PatientName, { "Patient "+patient });
                    data_set.add(odil::registry::PatientID, { patient });

                    data_set.add(
                        odil::registry::StudyInstanceUID, {study});
                    data_set.add(
                        odil::registry::StudyDescription, { "Study "+study });

                    data_set.add(
                        odil::registry::SeriesInstanceUID, { series });
                    data_set.add(
                        odil::registry::Modality,
                        { modalities[series_index-1] });

                    data_set.add(
                        odil::registry::SOPInstanceUID, { instance });
                    data_set.add(
                        odil::registry::ImageComments,
                        { "Instance "+instance });
                    data_set.add(
                        odil::registry::SOPClassUID,
                        { sop_classes[instance_index-1] });

                    data_set.add(
                        odil::registry::PixelData,
                        { odil::Value::Binary::value_type{
                            uint8_t(patient_index), uint8_t(study_index),
                            uint8_t(series_index), uint8_t(instance_index) } },
                        odil::VR::OB);

                    storage.store(data_set);
                }
            }
        }
    }
}

} // namespace fixtures
