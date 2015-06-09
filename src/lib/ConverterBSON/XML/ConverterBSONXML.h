/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _90721708_58a8_4d32_9e93_28851c7f43fa
#define _90721708_58a8_4d32_9e93_28851c7f43fa

#include <string>

#include <boost/property_tree/ptree.hpp>

#include <mongo/bson/bson.h>

namespace dopamine
{

namespace converterBSON
{

const std::string Attribute_Keyword         = "<xmlattr>.keyword";
const std::string Attribute_Number          = "<xmlattr>.number";
const std::string Attribute_PrivateCreator  = "<xmlattr>.privateCreator";
const std::string Attribute_Tag             = "<xmlattr>.tag";
const std::string Attribute_VR              = "<xmlattr>.vr";
const std::string Tag_Alphabetic            = "Alphabetic";
const std::string Tag_BulkData              = "BulkData";
const std::string Tag_DicomAttribute        = "DicomAttribute";
const std::string Tag_FamilyName            = "FamilyName";
const std::string Tag_GivenName             = "GivenName";
const std::string Tag_Ideographic           = "Ideographic";
const std::string Tag_InlineBinary          = "InlineBinary";
const std::string Tag_Item                  = "Item";
const std::string Tag_MiddleName            = "MiddleName";
const std::string Tag_NamePrefix            = "NamePrefix";
const std::string Tag_NameSuffix            = "NameSuffix";
const std::string Tag_NativeDicomModel      = "NativeDicomModel";
const std::string Tag_PersonName            = "PersonName";
const std::string Tag_Phonetic              = "Phonetic";
const std::string Tag_Value                 = "Value";

/**
 * @brief The ConverterBSONXML class
 * Base class for converter BSON <-> XML
 */
class ConverterBSONXML
{
public:
    /// Create an instance of ConverterBSONXML
    ConverterBSONXML();

    /// Destroy the instance of ConverterBSONXML
    virtual ~ConverterBSONXML();

protected:

private:

};

} // namespace converterBSON

} // namespace dopamine

#endif // _90721708_58a8_4d32_9e93_28851c7f43fa
