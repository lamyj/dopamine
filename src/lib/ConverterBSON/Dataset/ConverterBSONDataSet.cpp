/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <sstream>

#include "ConverterBSONDataSet.h"
#include "core/ConverterCharactersSet.h"
#include "core/ExceptionPACS.h"

namespace dopamine
{

namespace converterBSON
{

ConverterBSONDataSet
::ConverterBSONDataSet(bool isDcmToBSON):
    _isDcmToBSON(isDcmToBSON)
{
    // Nothing to do
}

ConverterBSONDataSet
::~ConverterBSONDataSet()
{
    // Nothing to do
}

std::string
ConverterBSONDataSet
::get_specific_character_set() const
{
    std::stringstream values;
    // Specific Character Set: map to iconv encoding
    for (auto elem : this->_specific_character_sets)
    {
        if (values.str() != "") values << "\\";
        values << elem;
    }
    return values.str();
}

void
ConverterBSONDataSet
::set_specific_character_set(std::string const & specific_character_set)
{
    this->_specific_character_sets.clear();

    std::string const delimiters("\\");

    std::size_t current;
    std::size_t next=-1;
    do
    {
        current = next+1;
        next = specific_character_set.find_first_of(delimiters, current);
        std::string const element(
            specific_character_set.substr(current, next-current));

        if (characterset::_dicom_to_iconv.find(element) == characterset::_dicom_to_iconv.end())
        {
            std::stringstream streamerror;
            streamerror << "Unkown character set: " << element;
            throw ExceptionPACS(streamerror.str());
        }

        this->_specific_character_sets.push_back(element);
    }
    while(next != std::string::npos);
}

} // namespace converterBSON

} // namespace dopamine
