/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

/*************************************************************************
 * See PS3.19 Chapter A.1.6 Schema
 *
# This schema was created as an intermediary, a means of describing
# native binary encoded DICOM objects as XML Infosets, thus allowing
# one to manipulate binary DICOM objects using familiar XML tools.
# As such, the schema is designed to facilitate a simple, mechanical,
# bi-directional translation between binary encoded DICOM and XML-like
# constructs without constraints, and to simplify identifying portions
# of a DICOM object using XPath statements.
#
# Since this schema has minimal type checking, it is neither intended
# to be used for any operation that involves hand coding, nor to
# describe a definitive, fully validating encoding of DICOM concepts
# into XML, as what one might use, for example, in a robust XML
# database system or in XML-based forms, though it may be used
# as a means for translating binary DICOM Objects into such a form
# (e.g., through an XSLT script).

start = element NativeDicomModel { DicomDataSet }

# A DICOM Data Set is as defined in PS3.5.  It does not appear
# as an XML Element, since it does not appear in the binary encoded
# DICOM objects.  It exists here merely as a documentation aid.
DicomDataSet = DicomAttribute*

DicomAttribute = element DicomAttribute {
  Tag, VR, Keyword?, PrivateCreator?,
  (BulkData | Value+ | Item+ | PersonName+ | InlineBinary)?
}
BulkData = element BulkData{ UUID | URI }
Value = element Value { Number, xsd:string }
InlineBinary = element InlineBinary { xsd:base64Binary }
Item = element Item { Number, DicomDataSet }
PersonName = element PersonName {
  Number,
  element Alphabetic  { NameComponents }?,
  element Ideographic { NameComponents }?,
  element Phonetic    { NameComponents }?
}

NameComponents =
  element FamilyName {xsd:string}?,
  element GivenName  {xsd:string}?,
  element MiddleName {xsd:string}?,
  element NamePrefix {xsd:string}?,
  element NameSuffix {xsd:string}?

# keyword is the attribute tag from PS3.6
# (derived from the DICOM Attribute's name)
Keyword = attribute keyword { xsd:token }
# canonical XML definition of Hex, with lowercase letters disallowed
Tag = attribute tag { xsd:string{ minLength="8" maxLength="8" pattern="[0-9A-F]{8}" } }
VR = attribute vr { "AE" | "AS" | "AT"| "CS" | "DA" | "DS" | "DT" | "FL" | "FD"
                    | "IS" | "LO" | "LT" | "OB" | "OD" | "OF" | "OW" | "PN" | "SH" | "SL"
                    | "SQ" | "SS" | "ST" | "TM" | "UI" | "UL" | "UN" | "US" | "UT" }
PrivateCreator = attribute privateCreator{ xsd:string }
UUID = attribute uuid { xsd:string }
URI = attribute uri { xsd:anyURI }
Number = attribute number { xsd:positiveInteger }
 ************************************************************************/

#ifndef _052e3da7_023c_4322_80aa_3be13ca5b813
#define _052e3da7_023c_4322_80aa_3be13ca5b813

#include "ConverterBSONXML.h"

namespace dopamine
{

namespace converterBSON
{

/**
 * @brief The BSONToXML class
 * Convert a BSON Object into XML object
 */
class BSONToXML : public ConverterBSONXML
{
public:
    /// Create an instance of BSONToXML
    BSONToXML();

    /// Destroy the instance of BSONToXML
    ~BSONToXML();

    /**
     * to_ptree
     * @param bson: BSON object to convert
     * @return converted XML dataset
     */
     boost::property_tree::ptree to_ptree(mongo::BSONObj const & bson) const;

     /**
      * to_string
      * @param bson: BSON object to convert
      * @return converted XML dataset as string
      */
     std::string to_string(mongo::BSONObj const & bson) const;

protected:
     bool is_dicom_field(std::string const & field_name) const;

private:
     template<typename TBSONType>
     struct BSONGetterType
     {
         typedef TBSONType (mongo::BSONElement::*Type)() const;
     };

     void _to_dicom_attribute(mongo::BSONElement const & bson,
                              boost::property_tree::ptree & tag_xml) const;

     void _to_dicom_model(mongo::BSONObj const & bson,
                          boost::property_tree::ptree & tag_xml) const;

     void _to_item(mongo::BSONElement const & bson,
                   boost::property_tree::ptree & tag_xml) const;

     void _to_person_name(mongo::BSONElement const & bson,
                          boost::property_tree::ptree & tag_xml) const;

     void _to_raw(mongo::BSONElement const & bson,
                  boost::property_tree::ptree & tag_xml) const;

     template<typename TBSONType>
     void _to_value(mongo::BSONElement const & bson,
                    std::string const & vr,
                    boost::property_tree::ptree & tag_xml,
                    typename BSONGetterType<TBSONType>::Type getter) const;

     void _to_value_string_number(mongo::BSONElement const & bson,
                                  boost::property_tree::ptree & tag_xml) const;

};

} // namespace converterBSON

} // namespace dopamine

#endif // _052e3da7_023c_4322_80aa_3be13ca5b813
