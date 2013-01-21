#include "BSONToDataSet.h"

#include <stdexcept>

#include <errno.h>
#include <iconv.h>

#include <gdcmDataElement.h>
#include <gdcmTag.h>

BSONToDataSet
::BSONToDataSet()
: _specific_character_set(""), _converter(0)
{
}

BSONToDataSet
::~BSONToDataSet()
{
    if(this->_converter != 0)
    {
        iconv_close(this->_converter);
    }
}

std::string
BSONToDataSet
::get_specific_character_set() const
{
    return this->_specific_character_set;
}

void
BSONToDataSet
::set_specific_character_set(std::string const & specific_character_set)
{
    std::string encoding;

    // Tests files from dclunie
    // SCSARAB : ok
    // SCSFREN : ok
    // SCSGERM : ok
    // SCSGREEK : ok
    // SCSH31 : ?
    // SCSH32 : fail ('ISO 2022 IR 13')
    // SCSHBRW : ok
    // SCSI2 : fail (Invalid or incomplete multibyte or wide character, ISO 2022 IR 149)
    // SCSRUSS : ok
    // SCSX1 : ok
    // SCSX2 : ok

    if(specific_character_set == "") { encoding = "ASCII"; }
    // Single-byte character sets without code extensions (PS 3.3, Table C.12-2)
    else if(specific_character_set == "ISO_IR 100") { encoding = "ISO-8859-1"; }
    else if(specific_character_set == "ISO_IR 101") encoding = "ISO-8859-2";
    else if(specific_character_set == "ISO_IR 109") encoding = "ISO-8859-3";
    else if(specific_character_set == "ISO_IR 110") encoding = "ISO-8859-4";
    else if(specific_character_set == "ISO_IR 144") encoding = "ISO-8859-5";
    else if(specific_character_set == "ISO_IR 127") encoding = "ISO-8859-6";
    else if(specific_character_set == "ISO_IR 126") encoding = "ISO-8859-7";
    else if(specific_character_set == "ISO_IR 138") encoding = "ISO-8859-8";
    else if(specific_character_set == "ISO_IR 148") encoding = "ISO-8859-9";
    else if(specific_character_set == "ISO_IR 13") encoding = "ISO−2022−JP";
    // CP874 seems to be a superset of TIS-620/ISO-IR-166 (e.g.
    // presence of the euro sign in the CP874 at an unassigned place
    // of TIS-620), but we should get away with it.
    else if(specific_character_set == "ISO_IR 166") encoding = "CP-874";
    // Single-byte character sets with code extensions (PS 3.3, Table C.12-3)
//                ISO 2022 IR 6
//                ISO 2022 IR 100
//                ISO 2022 IR 101
//                ISO 2022 IR 109
//                ISO 2022 IR 110
//                ISO 2022 IR 144
//                ISO 2022 IR 127
//                ISO 2022 IR 126
//                ISO 2022 IR 138
//                ISO 2022 IR 148
//                ISO 2022 IR 113
//                ISO 2022 IR 166
    // Multi-byte character sets with code extensions (PS 3.3, Table C.12-4)
//                ISO 2022 IR 87
//                ISO 2022 IR 159
//                ISO 2022 IR 149
    // Multi-byte character sets without code extensions (PS 3.3, Table C.12-5)
    else if(specific_character_set == "ISO_IR 192") encoding = "UTF-8";
    else if(specific_character_set == "GB18030 ") encoding = "GB18030";
    else
    {
        std::ostringstream message;
        message << "Unkown specific character set: '" << specific_character_set << "'";
        throw std::runtime_error(message.str());
    }

    if(this->_converter != 0)
    {
        iconv_close(this->_converter);
    }
    this->_converter = iconv_open("UTF-8", encoding.c_str());
}

gdcm::DataSet
BSONToDataSet
::operator()(mongo::BSONObj const & bson)
{
    gdcm::DataSet data_set;
    for(mongo::BSONObj::iterator it = bson.begin(); it.more();)
    {
        mongo::BSONElement const element_bson = it.next();
        this->_add_element(element_bson, data_set);
    }

    return data_set;
}

void
BSONToDataSet
::_add_element(mongo::BSONElement const & bson, gdcm::DataSet & data_set) const
{
    gdcm::DataElement element;

    // Get the tag from the field name
    std::string const field_name = bson.fieldName();
    char * old_numeric = setlocale(LC_NUMERIC, NULL);
    setlocale(LC_NUMERIC, "C");
    char* endptr;
    long const d = std::strtol(field_name.c_str(), &endptr, 16);
    setlocale(LC_NUMERIC, old_numeric);
    element.SetTag(gdcm::Tag(d));

    // Value holding the VR and the data
    std::vector<mongo::BSONElement> const array = bson.Array();

    // Get the VR : first item of value
    gdcm::VR const vr(gdcm::VR::GetVRType(array[0].String().c_str()));
    element.SetVR(vr);

    // Get the byte value
    if(vr == gdcm::VR::SQ)
    {
        // TODO : SQ
        /*
        gdcm::SequenceOfItems sequence;
        for(;;)
        {
            BSONToDataSet converter;
            converter.set_specific_character_set(this->get_specific_character_set());

            gdcm::DataSet const item = converter(array[1]);
        }
        */
    }
    else if(vr & (gdcm::VR::OB | gdcm::VR::OF | gdcm::VR::OW | gdcm::VR::UN))
    {
    }
    else if(vr & (gdcm::VR::AE | gdcm::VR::AS | gdcm::VR::CS | gdcm::VR::DA |
                  gdcm::VR::DT | gdcm::VR::TM | gdcm::VR::UI))
    {
        if(!array[1].isNull())
        {
            std::string const value = array[1].String().c_str();
            element.SetByteValue(value.c_str(), value.size());
        }
    }
    else if(vr & (gdcm::VR::LO | gdcm::VR::LT | gdcm::VR::PN | gdcm::VR::SH |
                  gdcm::VR::ST | gdcm::VR::UT))
    {
        // Non-ASCII text content
    }
    // TODO : non-text content

    std::cout << element << "\n";
    data_set.Insert(element);
}
