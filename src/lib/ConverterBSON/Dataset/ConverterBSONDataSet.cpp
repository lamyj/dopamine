/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "ConverterBSONDataSet.h"

namespace dopamine
{

namespace converterBSON
{

std::map<std::string, std::string> const
ConverterBSONDataSet
::_dicom_to_iconv = ConverterBSONDataSet::_create_encoding_map();

ConverterBSONDataSet
::ConverterBSONDataSet(bool isDcmToBSON):
    _isDcmToBSON(isDcmToBSON), _specific_character_set(""), _converter(0)
{
    // Nothing to do
}

ConverterBSONDataSet
::~ConverterBSONDataSet()
{
    if(this->_converter != 0)
    {
        iconv_close(this->_converter);
    }
}

std::string
ConverterBSONDataSet
::get_specific_character_set() const
{
    return this->_specific_character_set;
}

void
ConverterBSONDataSet
::set_specific_character_set(std::string const & specific_character_set)
{
    std::vector<std::string> elements;
    std::string const delimiters("\\");

    std::size_t current;
    std::size_t next=-1;
    do
    {
        current = next+1;
        next = specific_character_set.find_first_of(delimiters, current);
        std::string const element(
            specific_character_set.substr(current, next-current));

        std::map<std::string, std::string>::const_iterator encoding_it =
            this->_dicom_to_iconv.find(element);
        if(encoding_it==this->_dicom_to_iconv.end())
        {
            throw std::runtime_error("Unknown encoding: '"+element+"'");
        }

        elements.push_back(element);
    }
    while(next != std::string::npos);

    // TODO : handle multi-valued specific character set
    if(elements.size() != 1)
    {
        throw std::runtime_error("Cannot handle specific character set '" +
                                 specific_character_set + "'");
    }

    this->_specific_character_set = specific_character_set;

    if(this->_converter != 0)
    {
        iconv_close(this->_converter);
    }
    
    if (this->_isDcmToBSON)
    {// Convert Dicom to BSON
        this->_converter = iconv_open("UTF-8",
            this->_dicom_to_iconv.find(elements[0])->second.c_str());
    }
    else
    {// Convert BSON to Dicom
        this->_converter = iconv_open(
            this->_dicom_to_iconv.find(elements[0])->second.c_str(), 
            "UTF-8");
    }
}

std::map<std::string, std::string>
ConverterBSONDataSet
::_create_encoding_map()
{
    std::map<std::string, std::string> result;

    // PS 3.3-2011, C.12.1.1.2 - Specific Character Set
    // Single-byte character sets without code extensions (PS 3.3, Table C.12-2)
    result[""] = "ISO-IR-6";
    result["ISO_IR 100"] = "ISO-IR-100";
    result["ISO_IR 101"] = "ISO-IR-101";
    result["ISO_IR 109"] = "ISO-IR-109";
    result["ISO_IR 110"] = "ISO-IR-110";
    result["ISO_IR 144"] = "ISO-IR-144";
    result["ISO_IR 127"] = "ISO-IR-127";
    result["ISO_IR 126"] = "ISO-IR-126";
    result["ISO_IR 138"] = "ISO-IR-138";
    result["ISO_IR 148"] = "ISO-IR-148";
    result["ISO_IR 13"] = "ISO-2022-JP"; // TODO or JP-2 or JP-3 ? DICOM says: Katakana+Romaji
    result["ISO_IR 166"] = "ISO-IR-166";
    // Single-byte character sets with code extensions (PS 3.3, Table C.12-3)
    result["ISO 2022 IR 6"] = "ISO-IR-6";
    result["ISO 2022 IR 100"] = "ISO-IR-100";
    result["ISO 2022 IR 101"] = "ISO-IR-101";
    result["ISO 2022 IR 109"] = "ISO-IR-109";
    result["ISO 2022 IR 110"] = "ISO-IR-110";
    result["ISO 2022 IR 144"] = "ISO-IR-144";
    result["ISO 2022 IR 127"] = "ISO-IR-127";
    result["ISO 2022 IR 126"] = "ISO-IR-126";
    result["ISO 2022 IR 138"] = "ISO-IR-138";
    result["ISO 2022 IR 148"] = "ISO-IR-148";
    result["ISO 2022 IR 13"] = "ISO-2022-JP"; // TODO or JP-2 or JP-3 ? DICOM says: Katakana+Romaji
    result["ISO 2022 IR 166"] = "ISO-IR-166";
    // Multi-byte character sets with code extensions (PS 3.3, Table C.12-4)
    // result["ISO 2022 IR 87"] // Kanji
    // result["ISO 2022 IR 159"] // Supplementary Kanji
    // result["ISO 2022 IR 149"] // Hangul, Hanja
    // Multi-byte character sets without code extensions (PS 3.3, Table C.12-5)
    result["ISO_IR 192"] = "UTF-8";
    result["GB18030"] = "GB18030";

    return result;
}

} // namespace converterBSON

} // namespace dopamine
