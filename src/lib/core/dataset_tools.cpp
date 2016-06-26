/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "dataset_tools.h"

#include <boost/property_tree/xml_parser.hpp>
#include <boost/version.hpp>

#include <odil/json_converter.h>
#include <odil/xml_converter.h>

std::string dopamine::dataset_to_json_string(odil::DataSet const & data_set)
{
    return odil::as_json(data_set).toStyledString();
}


std::string dopamine::dataset_to_xml_string(odil::DataSet const & data_set)
{
    auto const xml = odil::as_xml(data_set);

    std::stringstream xmldataset;

#if BOOST_VERSION >= 105600
    typedef boost::property_tree::xml_writer_settings<std::string> SettingsType;
#else
    typedef boost::property_tree::xml_writer_settings<char> SettingsType;
#endif
    SettingsType settings(' ', 4);
    boost::property_tree::write_xml(xmldataset, xml, settings);

    return xmldataset.str();
}
