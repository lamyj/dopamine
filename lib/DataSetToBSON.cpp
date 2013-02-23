#include "DataSetToBSON.h"

#include <algorithm>
#include <stdexcept>

#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dctk.h>

#include <errno.h>
#include <iconv.h>

std::map<std::string, std::string> const
DataSetToBSON
::_dicom_to_iconv = DataSetToBSON::_create_encoding_map();


DataSetToBSON
::DataSetToBSON()
: _specific_character_set(""), _converter(0),
  _filter(Filter::EXCLUDE), _filtered_tags()
{
    this->set_specific_character_set("");
    // Use EXCLUDE and an empty _filtered_tags so that by default all elements are
    // included
}

DataSetToBSON
::~DataSetToBSON()
{
    if(this->_converter != 0)
    {
        iconv_close(this->_converter);
    }
}

std::string
DataSetToBSON
::get_specific_character_set() const
{
    return this->_specific_character_set;
}

void
DataSetToBSON
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
        throw std::runtime_error(
            "Cannot handle specific character set '"+specific_character_set+"'");
    }

    this->_specific_character_set = specific_character_set;

    if(this->_converter != 0)
    {
        iconv_close(this->_converter);
    }
    this->_converter = iconv_open("UTF-8",
        this->_dicom_to_iconv.find(elements[0])->second.c_str());

}

DataSetToBSON::Filter::Type const &
DataSetToBSON
::get_filter() const
{
    return this->_filter;
}

void
DataSetToBSON
::set_filter(Filter::Type const & filter)
{
    if(filter<0 || filter>=Filter::MAX)
    {
        throw std::runtime_error("Incorrect filter value");
    }
    this->_filter = filter;
}

void
DataSetToBSON
::add_filtered_tag(DcmTag const & tag)
{
    this->_filtered_tags.insert(tag);
}

void
DataSetToBSON
::remove_filtered_tag(DcmTag const & tag)
{
    this->_filtered_tags.erase(tag);
}

void
DataSetToBSON
::clear_filtered_tags()
{
    this->_filtered_tags.clear();
}

bool
DataSetToBSON
::is_tag_filtered(DcmTag const & tag) const
{
    return (this->_filtered_tags.find(tag) == this->_filtered_tags.end());
}

void
DataSetToBSON
::operator()(DcmObject * dataset, mongo::BSONObjBuilder & builder)
{
    DcmObject * it = NULL;
    while(NULL != (it = dataset->nextInContainer(it)))
    {
        bool skip = false;
        DcmTag const & tag = it->getTag();
        if(this->_filter == Filter::INCLUDE)
        {
            if(this->_filtered_tags.find(tag) != this->_filtered_tags.end())
            {
                // Included
                skip = false;
            }
            else
            {
                // Not included => exclude
                skip = true;
            }
        }
        else if(this->_filter == Filter::EXCLUDE)
        {
            if(this->_filtered_tags.find(tag) != this->_filtered_tags.end())
            {
                // Excluded
                skip = true;
            }
            else
            {
                // Not excluded => include
                skip = false;
            }
        }

        if(skip)
        {
            continue;
        }

        uint16_t const tag_group = tag.getGTag();
        uint16_t const tag_element = tag.getETag();

        if(tag_group == 0x0008 && tag_element == 0x0005)
        {
            // Specific Character Set: setup internal iconv converter
            DcmCodeString * specific_character_set = 
                dynamic_cast<DcmCodeString*>(it);
            char* value;
            specific_character_set->getString(value);
            this->set_specific_character_set(value);
        }

        if(tag_element == 0)
        {
            // Group length, do nothing
            continue;
        }
        else
        {
            this->_add_element(it, builder);
        }
    }
}

std::map<std::string, std::string>
DataSetToBSON
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

/*******************************************************************************
 * Specializations of DataSetToBSON::_to_bson for the different VRs.
 ******************************************************************************/

template<>
void
DataSetToBSON::_to_bson<EVR_AE>(DcmObject * element,
                                mongo::BSONArrayBuilder & builder) const
{
    this->_to_bson_text(dynamic_cast<DcmByteString*>(element), builder, false);
}

template<>
void
DataSetToBSON::_to_bson<EVR_AS>(DcmObject * element,
                                      mongo::BSONArrayBuilder & builder) const
{
    this->_to_bson_text(dynamic_cast<DcmByteString*>(element), builder, false);
}

template<>
void
DataSetToBSON::_to_bson<EVR_AT>(DcmObject * element,
                                      mongo::BSONArrayBuilder & builder) const
{
    this->_to_bson_number(dynamic_cast<DcmElement*>(element), 
        &DcmElement::getUint32, builder);
}

template<>
void
DataSetToBSON::_to_bson<EVR_CS>(DcmObject * element,
                                      mongo::BSONArrayBuilder & builder) const
{
    this->_to_bson_text(dynamic_cast<DcmByteString*>(element), builder, false);
}

template<>
void
DataSetToBSON::_to_bson<EVR_DA>(DcmObject * element,
                                      mongo::BSONArrayBuilder & builder) const
{
    this->_to_bson_text(dynamic_cast<DcmByteString*>(element), builder, false);
}

template<>
void
DataSetToBSON::_to_bson<EVR_DT>(DcmObject * element,
                                      mongo::BSONArrayBuilder & builder) const
{
    this->_to_bson_text(dynamic_cast<DcmByteString*>(element), builder, false);
}

template<>
void
DataSetToBSON::_to_bson<EVR_DS>(DcmObject * element,
                                      mongo::BSONArrayBuilder & builder) const
{
    DcmDecimalString * ds = dynamic_cast<DcmDecimalString*>(element);
    unsigned long count = ds->getVM();

    if(count > 1)
    {
        mongo::BSONArrayBuilder sub_builder;

        for(unsigned long i=0; i<count; ++i)
        {
            Float64 value;
            ds->getFloat64(value, i);
            sub_builder.append(value);
        }

        builder.append(sub_builder.arr());
    }
    else
    {
        Float64 value;
        ds->getFloat64(value, 0);
        builder.append(value);
    }
}

template<>
void
DataSetToBSON::_to_bson<EVR_FD>(DcmObject * element,
                                      mongo::BSONArrayBuilder & builder) const
{
    this->_to_bson_number(dynamic_cast<DcmElement*>(element), 
        &DcmElement::getFloat64, builder);
}

template<>
void
DataSetToBSON::_to_bson<EVR_FL>(DcmObject * element,
                                      mongo::BSONArrayBuilder & builder) const
{
    this->_to_bson_number(dynamic_cast<DcmElement*>(element), 
        &DcmElement::getFloat32, builder);
}

template<>
void
DataSetToBSON::_to_bson<EVR_IS>(DcmObject * element,
                                      mongo::BSONArrayBuilder & builder) const
{
    DcmIntegerString * is = dynamic_cast<DcmIntegerString*>(element);
    unsigned long count = is->getVM();

    if(count > 1)
    {
        mongo::BSONArrayBuilder sub_builder;

        for(unsigned long i=0; i<count; ++i)
        {
            Sint32 value;
            is->getSint32(value, i);
            sub_builder.append(value);
        }

        builder.append(sub_builder.arr());
    }
    else
    {
        Sint32 value;
        is->getSint32(value, 0);
        builder.append(value);
    }
}

template<>
void
DataSetToBSON::_to_bson<EVR_LO>(DcmObject * element,
                                      mongo::BSONArrayBuilder & builder) const
{
    this->_to_bson_text(dynamic_cast<DcmByteString*>(element), builder, true);
}

template<>
void
DataSetToBSON::_to_bson<EVR_LT>(DcmObject * element,
                                      mongo::BSONArrayBuilder & builder) const
{
    this->_to_bson_text(dynamic_cast<DcmByteString*>(element), builder, true);
}

template<>
void
DataSetToBSON::_to_bson<EVR_OB>(DcmObject * element,
                                      mongo::BSONArrayBuilder & builder) const
{
    this->_to_bson_binary(dynamic_cast<DcmElement*>(element), builder);
}

template<>
void
DataSetToBSON::_to_bson<EVR_OF>(DcmObject * element,
                                      mongo::BSONArrayBuilder & builder) const
{
    this->_to_bson_binary(dynamic_cast<DcmElement*>(element), builder);
}

template<>
void
DataSetToBSON::_to_bson<EVR_OW>(DcmObject * element,
                                      mongo::BSONArrayBuilder & builder) const
{
    this->_to_bson_binary(dynamic_cast<DcmElement*>(element), builder);
}

template<>
void
DataSetToBSON::_to_bson<EVR_PN>(DcmObject * element,
                                      mongo::BSONArrayBuilder & builder) const
{
    this->_to_bson_text(dynamic_cast<DcmByteString*>(element), builder, true);
}

template<>
void
DataSetToBSON::_to_bson<EVR_SH>(DcmObject * element,
                                      mongo::BSONArrayBuilder & builder) const
{
    this->_to_bson_text(dynamic_cast<DcmByteString*>(element), builder, true);
}

template<>
void
DataSetToBSON::_to_bson<EVR_SL>(DcmObject * element,
                                      mongo::BSONArrayBuilder & builder) const
{
    this->_to_bson_number(dynamic_cast<DcmElement*>(element), 
        &DcmElement::getSint32, builder);
}

// SQ is not processed here

template<>
void
DataSetToBSON::_to_bson<EVR_SS>(DcmObject * element,
                                      mongo::BSONArrayBuilder & builder) const
{
    this->_to_bson_number(dynamic_cast<DcmElement*>(element), 
        &DcmElement::getSint16, builder);
}

template<>
void
DataSetToBSON::_to_bson<EVR_ST>(DcmObject * element,
                                      mongo::BSONArrayBuilder & builder) const
{
    this->_to_bson_text(dynamic_cast<DcmByteString*>(element), builder, true);
}

template<>
void
DataSetToBSON::_to_bson<EVR_TM>(DcmObject * element,
                                      mongo::BSONArrayBuilder & builder) const
{
    this->_to_bson_text(dynamic_cast<DcmByteString*>(element), builder, false);
}

template<>
void
DataSetToBSON::_to_bson<EVR_UI>(DcmObject * element,
                                      mongo::BSONArrayBuilder & builder) const
{
    this->_to_bson_text(dynamic_cast<DcmByteString*>(element), builder, false);
}

template<>
void
DataSetToBSON::_to_bson<EVR_UL>(DcmObject * element,
                                      mongo::BSONArrayBuilder & builder) const
{
    this->_to_bson_number(dynamic_cast<DcmElement*>(element), 
        &DcmElement::getUint32, builder);
}

template<>
void
DataSetToBSON::_to_bson<EVR_UN>(DcmObject * element,
                                      mongo::BSONArrayBuilder & builder) const
{
    this->_to_bson_binary(dynamic_cast<DcmElement*>(element), builder);
}

template<>
void
DataSetToBSON::_to_bson<EVR_US>(DcmObject * element,
                                      mongo::BSONArrayBuilder & builder) const
{
    this->_to_bson_number(dynamic_cast<DcmElement*>(element), 
        &DcmElement::getUint16, builder);
}

template<>
void
DataSetToBSON::_to_bson<EVR_UT>(DcmObject * element,
                                      mongo::BSONArrayBuilder & builder) const
{
    this->_to_bson_text(dynamic_cast<DcmByteString*>(element), builder, true);
}

/*******************************************************************************
 * End of specializations of DataSetToBSON::_to_bson for the different VRs.
 ******************************************************************************/

void
DataSetToBSON::_to_bson_text(
    DcmByteString * element, mongo::BSONArrayBuilder & builder,
    bool use_utf8) const
{
    unsigned long count = element->getVM();

    if(count > 1)
    {
        // TODO
    }
    else
    {
        OFString value;
        element->getOFString(value, 0);
        char* buffer = NULL;
        if(use_utf8)
        {
            unsigned long size = value.size();
            unsigned long buffer_size = size*4; // worst case: ascii->UCS-32
            buffer = new char[buffer_size];
            std::fill(buffer, buffer+buffer_size, 0);

            size_t inbytesleft=size;
            size_t outbytesleft=buffer_size;
            char* inbuf = const_cast<char*>(&value[0]);
            char* outbuf = buffer;

            size_t const result = iconv(this->_converter,
                &inbuf, &inbytesleft, &outbuf, &outbytesleft);
            if(result == size_t(-1))
            {
                throw std::runtime_error(std::string("iconv error ")+strerror(errno));
            }

            value = OFString(buffer, buffer_size-outbytesleft);
        }

        builder.append(std::string(value.c_str()));

        if(use_utf8)
        {
            delete[] buffer;
        }
    }
}

template<typename TValue>
void
DataSetToBSON::_to_bson_number(DcmElement * element,
    OFCondition (DcmElement::*getter)(TValue &, unsigned long),
    mongo::BSONArrayBuilder & builder) const
{
    unsigned long count = element->getVM();
    if(count > 1)
    {
        mongo::BSONArrayBuilder sub_builder;
        for(unsigned long i=0; i<count; ++i)
        {
            TValue value;
            (element->*getter)(value, i);
            sub_builder.append(value);
        }
        builder.append(sub_builder.arr());
    }   
    else
    {   
        TValue value;
        (element->*getter)(value, 0);
        builder.append(value);
    }   
}

void
DataSetToBSON::_to_bson_binary(DcmElement * element,
                               mongo::BSONArrayBuilder & builder) const
{

    char* begin;
    element->getString(begin);
    mongo::BSONObjBuilder binary_data_builder;
    binary_data_builder.appendBinData("data", element->getVM(), 
                                      mongo::BinDataGeneral, begin);
    builder.append(binary_data_builder.obj().getField("data"));

}

void
DataSetToBSON
::_add_element(DcmObject * element, mongo::BSONObjBuilder & builder) const
{
    DcmEVR const vr(element->getVR());

    mongo::BSONArrayBuilder value_builder;
    value_builder.append(DcmVR(vr).getValidVRName());

    if(vr == EVR_SQ)
    {
        DcmSequenceOfItems * sequence = dynamic_cast<DcmSequenceOfItems*>(element);
        mongo::BSONArrayBuilder sequence_builder;

        DcmObject * sequence_it = NULL;
        while(NULL != (sequence_it = sequence->nextInContainer(sequence_it)))
        {
            mongo::BSONObjBuilder item_builder;
            DataSetToBSON converter;
            converter.set_specific_character_set(this->get_specific_character_set());
            converter(sequence_it, item_builder);
            sequence_builder.append(item_builder.obj());
        }
        value_builder.append(sequence_builder.arr());
    }
    else if(element->getLength() == 0)
    {
        value_builder.appendNull();
    }
    else
    {
        if(vr == EVR_AE) this->_to_bson<EVR_AE>(element, value_builder);
        else if(vr == EVR_AS) this->_to_bson<EVR_AS>(element, value_builder);
        else if(vr == EVR_AT) this->_to_bson<EVR_AT>(element, value_builder);
        else if(vr == EVR_CS) this->_to_bson<EVR_CS>(element, value_builder);
        else if(vr == EVR_DA) this->_to_bson<EVR_DA>(element, value_builder);
        else if(vr == EVR_DT) this->_to_bson<EVR_DT>(element, value_builder);
        else if(vr == EVR_DS) this->_to_bson<EVR_DS>(element, value_builder);
        else if(vr == EVR_FD) this->_to_bson<EVR_FD>(element, value_builder);
        else if(vr == EVR_FL) this->_to_bson<EVR_FL>(element, value_builder);
        else if(vr == EVR_IS) this->_to_bson<EVR_IS>(element, value_builder);
        else if(vr == EVR_LO) this->_to_bson<EVR_LO>(element, value_builder);
        else if(vr == EVR_LT) this->_to_bson<EVR_LT>(element, value_builder);
        else if(vr == EVR_OB) this->_to_bson<EVR_OB>(element, value_builder);
        else if(vr == EVR_OF) this->_to_bson<EVR_OF>(element, value_builder);
        else if(vr == EVR_OW) this->_to_bson<EVR_OW>(element, value_builder);
        else if(vr == EVR_PN) this->_to_bson<EVR_PN>(element, value_builder);
        else if(vr == EVR_SH) this->_to_bson<EVR_SH>(element, value_builder);
        // SQ is not processed here
        else if(vr == EVR_SL) this->_to_bson<EVR_SL>(element, value_builder);
        else if(vr == EVR_SS) this->_to_bson<EVR_SS>(element, value_builder);
        else if(vr == EVR_ST) this->_to_bson<EVR_ST>(element, value_builder);
        else if(vr == EVR_TM) this->_to_bson<EVR_TM>(element, value_builder);
        else if(vr == EVR_UI) this->_to_bson<EVR_UI>(element, value_builder);
        else if(vr == EVR_UL) this->_to_bson<EVR_UL>(element, value_builder);
        else if(vr == EVR_UN) this->_to_bson<EVR_UN>(element, value_builder);
        else if(vr == EVR_US) this->_to_bson<EVR_US>(element, value_builder);
        else if(vr == EVR_UT) this->_to_bson<EVR_UT>(element, value_builder);
        else
        {
            throw std::runtime_error(std::string("Unhandled VR:") + DcmVR(vr).getValidVRName());
        }
    }

    static char buffer[9];
    snprintf(buffer, 9, "%04x%04x", element->getGTag(), element->getETag());

    builder << buffer << value_builder.arr();
}

