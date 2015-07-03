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

std::string const Attribute_Keyword         = "<xmlattr>.keyword";
std::string const Attribute_Number          = "<xmlattr>.number";
std::string const Attribute_PrivateCreator  = "<xmlattr>.privateCreator";
std::string const Attribute_Tag             = "<xmlattr>.tag";
std::string const Attribute_VR              = "<xmlattr>.vr";
std::string const Tag_Alphabetic            = "Alphabetic";
std::string const Tag_BulkData              = "BulkData";
std::string const Tag_DicomAttribute        = "DicomAttribute";
std::string const Tag_FamilyName            = "FamilyName";
std::string const Tag_GivenName             = "GivenName";
std::string const Tag_Ideographic           = "Ideographic";
std::string const Tag_InlineBinary          = "InlineBinary";
std::string const Tag_Item                  = "Item";
std::string const Tag_MiddleName            = "MiddleName";
std::string const Tag_NamePrefix            = "NamePrefix";
std::string const Tag_NameSuffix            = "NameSuffix";
std::string const Tag_NativeDicomModel      = "NativeDicomModel";
std::string const Tag_PersonName            = "PersonName";
std::string const Tag_Phonetic              = "Phonetic";
std::string const Tag_Value                 = "Value";

/**
 * @brief \class The ConverterBSONXML class
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
