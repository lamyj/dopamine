/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "ConverterBSONDataSet.h"

ConverterBSONDataSet
::ConverterBSONDataSet():
    _specific_character_set(""), _converter(0)
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
