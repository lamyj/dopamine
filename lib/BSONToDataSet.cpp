#include "BSONToDataSet.h"

#include <locale>
#include <sstream>
#include <stdexcept>
#include <stdint.h>
#include <typeinfo>

#include <errno.h>
#include <iconv.h>

#include <gdcmDataElement.h>
#include <gdcmSequenceOfItems.h>
#include <gdcmItem.h>
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
    this->_converter = iconv_open(encoding.c_str(), "UTF-8");
}

gdcm::DataSet
BSONToDataSet
::operator()(mongo::BSONObj const & bson)
{
    gdcm::DataSet data_set;
    for(mongo::BSONObj::iterator it = bson.begin(); it.more();)
    {
        mongo::BSONElement const element_bson = it.next();
        if(element_bson.fieldName() == std::string("_id"))
        {
            continue;
        }
        this->_add_element(element_bson, data_set);
    }

    return data_set;
}

template<gdcm::VR::VRType VVR>
std::vector<uint8_t>
BSONToDataSet
::_to_gdcm(mongo::BSONElement const & bson) const
{
    throw std::runtime_error(std::string("Conversion for ")+
                             gdcm::VR::GetVRString(VVR)+
                             std::string(" is not implemented"));
}

/*******************************************************************************
 * Specializations of BSONToDataSet::_to_gdcm for the different VRs.
 ******************************************************************************/

template<>
std::vector<uint8_t>
BSONToDataSet
::_to_gdcm<gdcm::VR::AE>(mongo::BSONElement const & bson) const
{
    return this->_to_gdcm_text(bson, false, ' ');
}

template<>
std::vector<uint8_t>
BSONToDataSet
::_to_gdcm<gdcm::VR::AS>(mongo::BSONElement const & bson) const
{
    return this->_to_gdcm_text(bson, false, ' ');
}

// TODO : AT

template<>
std::vector<uint8_t>
BSONToDataSet
::_to_gdcm<gdcm::VR::CS>(mongo::BSONElement const & bson) const
{
    return this->_to_gdcm_text(bson,  false, ' ');
}

template<>
std::vector<uint8_t>
BSONToDataSet
::_to_gdcm<gdcm::VR::DA>(mongo::BSONElement const & bson) const
{
    return this->_to_gdcm_text(bson, false, ' ');
}

template<>
std::vector<uint8_t>
BSONToDataSet
::_to_gdcm<gdcm::VR::DS>(mongo::BSONElement const & bson) const
{
    return this->_to_gdcm_number_string(bson);
}

template<>
std::vector<uint8_t>
BSONToDataSet
::_to_gdcm<gdcm::VR::DT>(mongo::BSONElement const & bson) const
{
    return this->_to_gdcm_text(bson, false, ' ');
}

template<>
std::vector<uint8_t>
BSONToDataSet
::_to_gdcm<gdcm::VR::FD>(mongo::BSONElement const & bson) const
{
    return this->_to_gdcm_binary<double>(bson);
}

template<>
std::vector<uint8_t>
BSONToDataSet
::_to_gdcm<gdcm::VR::FL>(mongo::BSONElement const & bson) const
{
    return this->_to_gdcm_binary<float>(bson);
}

template<>
std::vector<uint8_t>
BSONToDataSet
::_to_gdcm<gdcm::VR::IS>(mongo::BSONElement const & bson) const
{
    return this->_to_gdcm_number_string(bson);
}

template<>
std::vector<uint8_t>
BSONToDataSet
::_to_gdcm<gdcm::VR::LO>(mongo::BSONElement const & bson) const
{
    return this->_to_gdcm_text(bson, true, ' ');
}

template<>
std::vector<uint8_t>
BSONToDataSet
::_to_gdcm<gdcm::VR::LT>(mongo::BSONElement const & bson) const
{
    return this->_to_gdcm_text(bson, true, ' ');
}

template<>
std::vector<uint8_t>
BSONToDataSet
::_to_gdcm<gdcm::VR::OB>(mongo::BSONElement const & bson) const
{
    return this->_to_gdcm_raw(bson);
}

template<>
std::vector<uint8_t>
BSONToDataSet
::_to_gdcm<gdcm::VR::OF>(mongo::BSONElement const & bson) const
{
    return this->_to_gdcm_raw(bson);
}

template<>
std::vector<uint8_t>
BSONToDataSet
::_to_gdcm<gdcm::VR::OW>(mongo::BSONElement const & bson) const
{
    return this->_to_gdcm_raw(bson);
}

template<>
std::vector<uint8_t>
BSONToDataSet
::_to_gdcm<gdcm::VR::PN>(mongo::BSONElement const & bson) const
{
    return this->_to_gdcm_text(bson, true, ' ');
}

template<>
std::vector<uint8_t>
BSONToDataSet
::_to_gdcm<gdcm::VR::SH>(mongo::BSONElement const & bson) const
{
    return this->_to_gdcm_text(bson, true, ' ');
}

template<>
std::vector<uint8_t>
BSONToDataSet
::_to_gdcm<gdcm::VR::SL>(mongo::BSONElement const & bson) const
{
    return this->_to_gdcm_binary<int32_t>(bson);
}

// SQ is not processed here

template<>
std::vector<uint8_t>
BSONToDataSet
::_to_gdcm<gdcm::VR::SS>(mongo::BSONElement const & bson) const
{
    return this->_to_gdcm_binary<int16_t>(bson);
}

template<>
std::vector<uint8_t>
BSONToDataSet
::_to_gdcm<gdcm::VR::ST>(mongo::BSONElement const & bson) const
{
    return this->_to_gdcm_text(bson, true, ' ');
}

template<>
std::vector<uint8_t>
BSONToDataSet
::_to_gdcm<gdcm::VR::TM>(mongo::BSONElement const & bson) const
{
    return this->_to_gdcm_text(bson, false, ' ');
}

template<>
std::vector<uint8_t>
BSONToDataSet
::_to_gdcm<gdcm::VR::UI>(mongo::BSONElement const & bson) const
{
    return this->_to_gdcm_text(bson, false, '\0');
}

template<>
std::vector<uint8_t>
BSONToDataSet
::_to_gdcm<gdcm::VR::UL>(mongo::BSONElement const & bson) const
{
    return this->_to_gdcm_binary<uint32_t>(bson);
}

template<>
std::vector<uint8_t>
BSONToDataSet
::_to_gdcm<gdcm::VR::UN>(mongo::BSONElement const & bson) const
{
    return this->_to_gdcm_raw(bson);
}

template<>
std::vector<uint8_t>
BSONToDataSet
::_to_gdcm<gdcm::VR::US>(mongo::BSONElement const & bson) const
{
    return this->_to_gdcm_binary<uint16_t>(bson);
}

template<>
std::vector<uint8_t>
BSONToDataSet
::_to_gdcm<gdcm::VR::UT>(mongo::BSONElement const & bson) const
{
    return this->_to_gdcm_text(bson, true, ' ');
}

/*******************************************************************************
 * End of specializations of BSONToDataSet::_to_gdcm for the different VRs.
 ******************************************************************************/

void
BSONToDataSet
::_add_element(mongo::BSONElement const & bson, gdcm::DataSet & data_set)
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

    if(!array[1].isNull())
    {
        if(d == 0x00080005)
        {
            // Specific Character Set: map to iconv encoding
            std::string value = array[1].String();
            if(value.size()%2 != 0)
            {
                value += ' ';
            }
            // TODO : multi-valued Specific Character Set
            this->set_specific_character_set(value);
        }

        if(vr == gdcm::VR::SQ)
        {
            gdcm::SmartPointer<gdcm::SequenceOfItems> sequence(new gdcm::SequenceOfItems());

            std::vector<mongo::BSONElement> elements = array[1].Array();
            for(std::vector<mongo::BSONElement>::const_iterator it=elements.begin();
                it != elements.end(); ++it)
            {
                BSONToDataSet converter;
                converter.set_specific_character_set(this->get_specific_character_set());

                gdcm::DataSet const item_dataset = converter(it->Obj());
                gdcm::Item item;
                item.SetNestedDataSet(item_dataset);
                sequence->AddItem(item);
            }

            element.SetValue(*sequence);
        }
        else
        {
            std::vector<uint8_t> value;
            if(vr == gdcm::VR::AE) value=this->_to_gdcm<gdcm::VR::AE>(array[1]);
            else if(vr == gdcm::VR::AS) value=this->_to_gdcm<gdcm::VR::AS>(array[1]);
            else if(vr == gdcm::VR::AT) value=this->_to_gdcm<gdcm::VR::AT>(array[1]);
            else if(vr == gdcm::VR::CS) value=this->_to_gdcm<gdcm::VR::CS>(array[1]);
            else if(vr == gdcm::VR::DA) value=this->_to_gdcm<gdcm::VR::DA>(array[1]);
            else if(vr == gdcm::VR::DT) value=this->_to_gdcm<gdcm::VR::DT>(array[1]);
            else if(vr == gdcm::VR::DS) value=this->_to_gdcm<gdcm::VR::DS>(array[1]);
            else if(vr == gdcm::VR::FD) value=this->_to_gdcm<gdcm::VR::FD>(array[1]);
            else if(vr == gdcm::VR::FL) value=this->_to_gdcm<gdcm::VR::FL>(array[1]);
            else if(vr == gdcm::VR::IS) value=this->_to_gdcm<gdcm::VR::IS>(array[1]);
            else if(vr == gdcm::VR::LO) value=this->_to_gdcm<gdcm::VR::LO>(array[1]);
            else if(vr == gdcm::VR::LT) value=this->_to_gdcm<gdcm::VR::LT>(array[1]);
            else if(vr == gdcm::VR::OB) value=this->_to_gdcm<gdcm::VR::OB>(array[1]);
            else if(vr == gdcm::VR::OF) value=this->_to_gdcm<gdcm::VR::OF>(array[1]);
            else if(vr == gdcm::VR::OW) value=this->_to_gdcm<gdcm::VR::OW>(array[1]);
            else if(vr == gdcm::VR::PN) value=this->_to_gdcm<gdcm::VR::PN>(array[1]);
            else if(vr == gdcm::VR::SH) value=this->_to_gdcm<gdcm::VR::SH>(array[1]);
            // SQ is not processed here
            else if(vr == gdcm::VR::SL) value=this->_to_gdcm<gdcm::VR::SL>(array[1]);
            else if(vr == gdcm::VR::SS) value=this->_to_gdcm<gdcm::VR::SS>(array[1]);
            else if(vr == gdcm::VR::ST) value=this->_to_gdcm<gdcm::VR::ST>(array[1]);
            else if(vr == gdcm::VR::TM) value=this->_to_gdcm<gdcm::VR::TM>(array[1]);
            else if(vr == gdcm::VR::UI) value=this->_to_gdcm<gdcm::VR::UI>(array[1]);
            else if(vr == gdcm::VR::UL) value=this->_to_gdcm<gdcm::VR::UL>(array[1]);
            else if(vr == gdcm::VR::UN) value=this->_to_gdcm<gdcm::VR::UN>(array[1]);
            else if(vr == gdcm::VR::US) value=this->_to_gdcm<gdcm::VR::US>(array[1]);
            else if(vr == gdcm::VR::UT) value=this->_to_gdcm<gdcm::VR::UT>(array[1]);
            else
            {
                throw std::runtime_error(std::string("Unhandled VR:") + gdcm::VR::GetVRString(vr));
            }
            element.SetByteValue(reinterpret_cast<char*>(&value[0]), value.size());
        }
    }

    data_set.Insert(element);
}

std::vector<uint8_t>
BSONToDataSet
::_to_gdcm_text(mongo::BSONElement const & bson, bool use_utf8, char padding,
                bool add_padding) const
{
    std::vector<uint8_t> value;

    if(bson.isABSONObj())
    {
        // Multiple value
        std::vector<mongo::BSONElement> const elements = bson.Array();
        std::vector<mongo::BSONElement>::const_iterator const last_it = --elements.end();
        for(std::vector<mongo::BSONElement>::const_iterator it=elements.begin();
            it != elements.end(); ++it)
        {
            std::vector<uint8_t> const element_value =
                this->_to_gdcm_text(*it, use_utf8, padding, false);

            value.resize(value.size()+element_value.size());
            std::copy(element_value.begin(), element_value.end(),
                      value.end()-element_value.size());
            if(it != last_it)
            {
                value.push_back('\\');
            }
        }

        if(value.size()%2!=0)
        {
            value.push_back(padding);
        }
    }
    else
    {
        std::string const string = bson.String();

        if(use_utf8)
        {
            unsigned long size = string.size();
            unsigned long buffer_size = size*4; // worst case: UTF-8 with only ascii->UCS-32
            char* buffer = new char[buffer_size];
            std::fill(buffer, buffer+buffer_size, 0);

            size_t inbytesleft=size;
            size_t outbytesleft=buffer_size;
            char* inbuf = const_cast<char*>(&string[0]);
            char* outbuf = buffer;

            size_t const result = iconv(this->_converter,
                &inbuf, &inbytesleft, &outbuf, &outbytesleft);
            if(result == size_t(-1))
            {
                throw std::runtime_error(std::string("iconv error ")+strerror(errno));
            }

            value.resize(buffer_size-outbytesleft);
            std::copy(buffer, buffer+buffer_size-outbytesleft, value.begin());
        }
        else
        {
            value.resize(string.size());
            std::copy(string.begin(), string.end(), value.begin());
        }

        if(add_padding && value.size()%2!=0)
        {
            value.push_back(padding);
        }
    }

    return value;
}

template<typename T>
std::vector<uint8_t>
BSONToDataSet
::_to_gdcm_binary(mongo::BSONElement const & bson) const
{
    std::vector<uint8_t> value;

    if(bson.isABSONObj())
    {
        // Multiple value
        std::vector<mongo::BSONElement> const elements = bson.Array();
        std::vector<mongo::BSONElement>::const_iterator const last_it = --elements.end();
        for(std::vector<mongo::BSONElement>::const_iterator it=elements.begin();
            it != elements.end(); ++it)
        {
            std::vector<uint8_t> const element_value = this->_to_gdcm_binary<T>(*it);

            value.resize(value.size()+element_value.size());
            std::copy(element_value.begin(), element_value.end(),
                      value.end()-element_value.size());
        }
    }
    else
    {
        T number=0;
        if(bson.type() == mongo::NumberInt)
        {
            number = bson.Int();
        }
        else if(bson.type() == mongo::NumberLong)
        {
            number = bson.Long();
        }
        else if(bson.type() == mongo::NumberDouble)
        {
            number = bson.Double();
        }
        else
        {
            throw std::runtime_error("Cannot convert BSON element");
        }
        value.resize(sizeof(number));
        std::copy(reinterpret_cast<char const*>(&number),
                  reinterpret_cast<char const*>(&number)+sizeof(number),
                  value.begin());
    }

    return value;
}

std::vector<uint8_t>
BSONToDataSet
::_to_gdcm_raw(mongo::BSONElement const & bson) const
{
    int size;
    char const * begin = bson.binData(size);
    std::vector<uint8_t> value(size);
    std::copy(begin, begin+size, value.begin());

    return value;
}

std::vector<uint8_t>
BSONToDataSet
::_to_gdcm_number_string(mongo::BSONElement const & bson, bool add_padding) const
{
    std::vector<uint8_t> value;

    if(bson.isABSONObj())
    {
        // Multiple value
        std::vector<mongo::BSONElement> const elements = bson.Array();
        std::vector<mongo::BSONElement>::const_iterator const last_it = --elements.end();
        for(std::vector<mongo::BSONElement>::const_iterator it=elements.begin();
            it != elements.end(); ++it)
        {
            std::vector<uint8_t> const element_value = this->_to_gdcm_number_string(*it, false);

            value.resize(value.size()+element_value.size());
            std::copy(element_value.begin(), element_value.end(),
                      value.end()-element_value.size());
            if(it != last_it)
            {
                value.push_back('\\');
            }
        }

        if(value.size()%2!=0)
        {
            value.push_back(' ');
        }
    }
    else
    {
        std::ostringstream stream;
        stream.imbue(std::locale("C"));

        if(bson.type() == mongo::NumberDouble)
        {
            double const number = bson.Double();
            stream << number;
        }
        else if(bson.type() == mongo::NumberInt)
        {
            int const number = bson.Int();
            stream << number;
        }
        else if(bson.type() == mongo::NumberLong)
        {
            long long const number = bson.Long();
            stream << number;
        }

        std::string const string = stream.str();

        value.resize(string.size());
        std::copy(string.begin(), string.end(), value.begin());

        if(add_padding && value.size()%2!=0)
        {
            value.push_back(' ');
        }
    }

    return value;
}
