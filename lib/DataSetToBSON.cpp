#include "DataSetToBSON.h"

#include <algorithm>
#include <stdexcept>

#include <errno.h>
#include <iconv.h>

#include <gdcmAttribute.h>
#include <gdcmByteValue.h>
#include <gdcmDataElement.h>
#include <gdcmDataSet.h>
#include <gdcmSequenceOfItems.h>
#include <gdcmSmartPointer.h>
#include <gdcmVM.h>
#include <gdcmVR.h>

template<typename TIterator1, typename TIterator2>
TIterator1 find_first_not_of(TIterator1 first1, TIterator1 last1,
                            TIterator2 first2, TIterator2 last2)
{
    TIterator1 it=first1;
    while(it != last1)
    {
        if(std::find(first2, last2, *it) == last2)
        {
            break;
        }
        else
        {
            ++it;
        }
    }
    return it;
}

template<typename TIterator1, typename TIterator2>
TIterator1 find_last_not_of(TIterator1 first1, TIterator1 last1,
                            TIterator2 first2, TIterator2 last2)
{
    if(first1 == last1)
    {
        // Empty sequence
        return first1;
    }
    // If nothing is found,
    TIterator1 result=last1;

    // Start at the last element
    TIterator1 it=last1;
    --it;

    while(it != first1)
    {
        if(std::find(first2, last2, *it) == last2)
        {
            result = it;
            break;
        }
        --it;
    }

    // First element of the sequence
    if(it == first1)
    {
        if(std::find(first2, last2, *it) == last2)
        {
            result = it;
        }
    }
    return result;
}


DataSetToBSON
::DataSetToBSON()
: _specific_character_set(""), _converter(0)
{
    //this->set_order(Order::INCLUDE_FIRST);
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

/*
DataSetToBSON::Order::Type const &
DataSetToBSON
::get_order() const
{
    return this->_order;
}

void
DataSetToBSON
::set_order(Order::Type const & order)
{
    if(order<0 || order>=Order::MAX)
    {
        throw std::runtime_error("Incorrect order value");
    }
    this->_order = order;
}
*/

void
DataSetToBSON
::operator()(gdcm::DataSet const & dataset, mongo::BSONObjBuilder & builder)
{
    for(gdcm::DataSet::ConstIterator it=dataset.Begin(); it!=dataset.End(); ++it)
    {
        uint16_t const tag_group = it->GetTag().GetGroup();
        uint16_t const tag_element = it->GetTag().GetElement();

        if(tag_group == 0x0008 && tag_element == 0x0005)
        {
            // Specific Character Set: map to iconv encoding
            gdcm::Attribute<0x0008,0x0005> attribute;
            attribute.SetFromDataElement(*it);
            // TODO : multi-valued Specific Character Set
            this->set_specific_character_set(attribute.GetValue());
        }

        if(tag_element == 0)
        {
            // Group length, do nothing
            continue;
        }
        else
        {
            this->_add_element(*it, builder);
        }
    }
}

template<gdcm::VR::VRType VVR>
void
DataSetToBSON::_to_bson(char const * begin, char const * end,
                        mongo::BSONArrayBuilder & builder) const
{
    std::cout << "TODO " << gdcm::VR(VVR) << std::endl;
}

/*******************************************************************************
 * Specializations of DataSetToBSON::_to_bson for the different VRs.
 ******************************************************************************/

template<>
void
DataSetToBSON::_to_bson<gdcm::VR::AE>(char const * begin, char const * end,
                                      mongo::BSONArrayBuilder & builder) const
{
    this->_to_bson_text(begin, end, builder, true, true, " ", true, false);
}

template<>
void
DataSetToBSON::_to_bson<gdcm::VR::AS>(char const * begin, char const * end,
                                      mongo::BSONArrayBuilder & builder) const
{
    this->_to_bson_text(begin, end, builder, false, false, " ", true, false);
}

template<>
void
DataSetToBSON::_to_bson<gdcm::VR::AT>(char const * begin, char const * end,
                                      mongo::BSONArrayBuilder & builder) const
{
    this->_to_bson_reinterpret_cast<uint32_t>(begin, end, builder, gdcm::VR::AT);
}

template<>
void
DataSetToBSON::_to_bson<gdcm::VR::CS>(char const * begin, char const * end,
                                      mongo::BSONArrayBuilder & builder) const
{
    this->_to_bson_text(begin, end, builder, true, true, " ", true, false);
}

template<>
void
DataSetToBSON::_to_bson<gdcm::VR::DA>(char const * begin, char const * end,
                                      mongo::BSONArrayBuilder & builder) const
{
    this->_to_bson_text(begin, end, builder, true, true, " ", true, false);
}

template<>
void
DataSetToBSON::_to_bson<gdcm::VR::DT>(char const * begin, char const * end,
                                      mongo::BSONArrayBuilder & builder) const
{
    this->_to_bson_text(begin, end, builder, true, true, " ", true, false);
}

template<>
void
DataSetToBSON::_to_bson<gdcm::VR::DS>(char const * begin, char const * end,
                                      mongo::BSONArrayBuilder & builder) const
{
    unsigned long count = 1+std::count(begin, end, '\\');

    if(count > 1)
    {
        mongo::BSONArrayBuilder sub_builder;

        char const * item_begin = begin;
        for(unsigned long i=0; i<count; ++i)
        {
            char const * item_end = std::find(item_begin, end, '\\');
            this->_to_bson<gdcm::VR::DS>(item_begin, item_end, sub_builder);
            if(item_end == end)
            {
                item_begin = item_end;
            }
            else
            {
                item_begin = item_end+1;
            }
        }

        builder.append(sub_builder.arr());
    }
    else
    {
        std::string const value(begin, end-begin);

        char * old_numeric = setlocale(LC_NUMERIC, NULL);
        setlocale(LC_NUMERIC, "C");
        char* endptr;
        double const d = std::strtod(value.c_str(), &endptr);
        setlocale(LC_NUMERIC, old_numeric);

        if(endptr == begin)
        {
            throw std::runtime_error("Cannot parse DS");
        }

        builder.append(d);
    }
}

template<>
void
DataSetToBSON::_to_bson<gdcm::VR::FD>(char const * begin, char const * end,
                                      mongo::BSONArrayBuilder & builder) const
{
    this->_to_bson_reinterpret_cast<double>(begin, end, builder, gdcm::VR::FD);
}

template<>
void
DataSetToBSON::_to_bson<gdcm::VR::FL>(char const * begin, char const * end,
                                      mongo::BSONArrayBuilder & builder) const
{
    this->_to_bson_reinterpret_cast<float>(begin, end, builder, gdcm::VR::FL);
}

template<>
void
DataSetToBSON::_to_bson<gdcm::VR::IS>(char const * begin, char const * end,
                                      mongo::BSONArrayBuilder & builder) const
{
    unsigned long count = 1+std::count(begin, end, '\\');

    if(count > 1)
    {
        mongo::BSONArrayBuilder sub_builder;

        char const * item_begin = begin;
        for(unsigned long i=0; i<count; ++i)
        {
            char const * item_end = std::find(item_begin, end, '\\');
            this->_to_bson<gdcm::VR::IS>(item_begin, item_end, sub_builder);
            if(item_end == end)
            {
                item_begin = item_end;
            }
            else
            {
                item_begin = item_end+1;
            }
        }

        builder.append(sub_builder.arr());
    }
    else
    {
        std::string const value(begin, end-begin);

        char * old_numeric = setlocale(LC_NUMERIC, NULL);
        setlocale(LC_NUMERIC, "C");
        char* endptr;
        long const d = std::strtol(value.c_str(), &endptr, 10);
        setlocale(LC_NUMERIC, old_numeric);

        if(endptr == begin)
        {
            throw std::runtime_error("Cannot parse IS");
        }

        builder.append(static_cast<long long>(d));
    }
}

template<>
void
DataSetToBSON::_to_bson<gdcm::VR::LO>(char const * begin, char const * end,
                                      mongo::BSONArrayBuilder & builder) const
{
    this->_to_bson_text(begin, end, builder, true, true, " ", true, true);
}

template<>
void
DataSetToBSON::_to_bson<gdcm::VR::LT>(char const * begin, char const * end,
                                      mongo::BSONArrayBuilder & builder) const
{
    this->_to_bson_text(begin, end, builder, false, true, " ", false, true);
}

template<>
void
DataSetToBSON::_to_bson<gdcm::VR::OB>(char const * begin, char const * end,
                                      mongo::BSONArrayBuilder & builder) const
{
    this->_to_bson_binary(begin, end, builder);
}

template<>
void
DataSetToBSON::_to_bson<gdcm::VR::OF>(char const * begin, char const * end,
                                      mongo::BSONArrayBuilder & builder) const
{
    this->_to_bson_binary(begin, end, builder);
}

template<>
void
DataSetToBSON::_to_bson<gdcm::VR::OW>(char const * begin, char const * end,
                                      mongo::BSONArrayBuilder & builder) const
{
    this->_to_bson_binary(begin, end, builder);
}

template<>
void
DataSetToBSON::_to_bson<gdcm::VR::PN>(char const * begin, char const * end,
                                      mongo::BSONArrayBuilder & builder) const
{
    this->_to_bson_text(begin, end, builder, false, true, " ", true, true);
}

template<>
void
DataSetToBSON::_to_bson<gdcm::VR::SH>(char const * begin, char const * end,
                                      mongo::BSONArrayBuilder & builder) const
{
    this->_to_bson_text(begin, end, builder, true, true, " ", true, true);
}

template<>
void
DataSetToBSON::_to_bson<gdcm::VR::SL>(char const * begin, char const * end,
                                      mongo::BSONArrayBuilder & builder) const
{
    this->_to_bson_reinterpret_cast<int32_t>(begin, end, builder, gdcm::VR::SL);
}

// SQ is not processed here

template<>
void
DataSetToBSON::_to_bson<gdcm::VR::SS>(char const * begin, char const * end,
                                      mongo::BSONArrayBuilder & builder) const
{
    this->_to_bson_reinterpret_cast<int16_t>(begin, end, builder, gdcm::VR::SS);
}

template<>
void
DataSetToBSON::_to_bson<gdcm::VR::ST>(char const * begin, char const * end,
                                      mongo::BSONArrayBuilder & builder) const
{
    this->_to_bson_text(begin, end, builder, false, true, " ", false, true);
}

template<>
void
DataSetToBSON::_to_bson<gdcm::VR::TM>(char const * begin, char const * end,
                                      mongo::BSONArrayBuilder & builder) const
{
    this->_to_bson_text(begin, end, builder, true, true, " ", true, false);
}

template<>
void
DataSetToBSON::_to_bson<gdcm::VR::UI>(char const * begin, char const * end,
                                      mongo::BSONArrayBuilder & builder) const
{
    this->_to_bson_text(begin, end, builder, true, true, std::string(1, '\0'), true, false);
}

template<>
void
DataSetToBSON::_to_bson<gdcm::VR::UL>(char const * begin, char const * end,
                                      mongo::BSONArrayBuilder & builder) const
{
    this->_to_bson_reinterpret_cast<uint32_t>(begin, end, builder, gdcm::VR::UL);
}

template<>
void
DataSetToBSON::_to_bson<gdcm::VR::UN>(char const * begin, char const * end,
                                      mongo::BSONArrayBuilder & builder) const
{
    this->_to_bson_binary(begin, end, builder);
}

template<>
void
DataSetToBSON::_to_bson<gdcm::VR::US>(char const * begin, char const * end,
                                      mongo::BSONArrayBuilder & builder) const
{
    this->_to_bson_reinterpret_cast<uint16_t>(begin, end, builder, gdcm::VR::US);
}

template<>
void
DataSetToBSON::_to_bson<gdcm::VR::UT>(char const * begin, char const * end,
                                      mongo::BSONArrayBuilder & builder) const
{
    this->_to_bson_text(begin, end, builder, false, true, " ", false, true);
}

/*******************************************************************************
 * End of specializations of DataSetToBSON::_to_bson for the different VRs.
 ******************************************************************************/

void
DataSetToBSON::_to_bson_text(
    char const * begin, char const * end, mongo::BSONArrayBuilder & builder,
    bool trim_left, bool trim_right, std::string const & whitespace,
    bool multiple_items, bool use_utf8) const
{
    if(multiple_items)
    {
        unsigned long count = 1+std::count(begin, end, '\\');

        if(count == 1)
        {
            this->_to_bson_text(begin, end, builder,
                                trim_left, trim_right, whitespace, false, use_utf8);
        }
        else
        {
            mongo::BSONArrayBuilder sub_builder;

            char const * item_begin = begin;
            for(unsigned long i=0; i<count; ++i)
            {
                char const * item_end = std::find(item_begin, end, '\\');
                this->_to_bson_text(item_begin, item_end, sub_builder,
                                    trim_left, trim_right, whitespace, false, use_utf8);
                if(item_end == end)
                {
                    item_begin = item_end;
                }
                else
                {
                    item_begin = item_end+1;
                }
            }

            builder.append(sub_builder.arr());
        }
    }
    else
    {
        // Single item
        char const * value_begin=begin;
        char const * value_end=end;
        if(trim_left)
        {
            value_begin = find_first_not_of(begin, end, whitespace.begin(), whitespace.end());
        }
        if(trim_right)
        {
            value_end = find_last_not_of(begin, end, whitespace.begin(), whitespace.end());
            if(value_end != end)
            {
                ++value_end;
            }
        }

        char* buffer = NULL;
        if(use_utf8)
        {
            unsigned long size = value_end-value_begin;
            unsigned long buffer_size = size*4; // worst case: ascii->UCS-32
            char* buffer = new char[buffer_size];
            std::fill(buffer, buffer+buffer_size, 0);

            size_t inbytesleft=size;
            size_t outbytesleft=buffer_size;
            char* inbuf = const_cast<char*>(value_begin);
            char* outbuf = buffer;

            size_t const result = iconv(this->_converter,
                &inbuf, &inbytesleft, &outbuf, &outbytesleft);
            if(result == size_t(-1))
            {
                throw std::runtime_error(std::string("iconv error ")+strerror(errno));
            }

            value_begin = buffer;
            value_end = buffer+buffer_size-outbytesleft;
        }

        std::string const value(value_begin, value_end-value_begin);
        builder.append(value);

        if(use_utf8)
        {
            delete[] buffer;
        }
    }
}

template<typename T>
void
DataSetToBSON::_to_bson_reinterpret_cast(char const * begin, char const * end,
                                         mongo::BSONArrayBuilder & builder, gdcm::VR const & vr) const
{
    unsigned long const count = (end-begin)/vr.GetSize();
    if(count>1)
    {
        mongo::BSONArrayBuilder sub_builder;

        char const * item_begin = begin;
        for(unsigned int i=0; i<count; ++i)
        {
            char const * item_end = item_begin+this->_get_length(item_begin, end, vr);
            this->_to_bson_reinterpret_cast<T>(item_begin, item_end, sub_builder, vr);
            item_begin = item_end;
        }
        builder.append(sub_builder.arr());
    }
    else if(count == 1)
    {
        builder.append(*reinterpret_cast<T const *>(begin));
    }
    else
    {
        // This should not happen.
        // Do nothing.
    }
}

void
DataSetToBSON::_to_bson_binary(char const * begin, char const * end,
                               mongo::BSONArrayBuilder & builder) const
{
    mongo::BSONObjBuilder binary_data_builder;
    binary_data_builder.appendBinData("data", end-begin, mongo::BinDataGeneral, begin);
    builder.append(binary_data_builder.obj().getField("data"));
}

void
DataSetToBSON
::_add_element(gdcm::DataElement const & element, mongo::BSONObjBuilder & builder) const
{
    gdcm::VR const vr = element.GetVR();

    mongo::BSONArrayBuilder value_builder;
    value_builder.append(gdcm::VR::GetVRString(vr));

    if(vr == gdcm::VR::SQ)
    {
        gdcm::SmartPointer<gdcm::SequenceOfItems> sequence = element.GetValueAsSQ();
        mongo::BSONArrayBuilder sequence_builder;

        for(gdcm::SequenceOfItems::ConstIterator sequence_it=sequence->Begin();
            sequence_it!=sequence->End(); ++sequence_it)
        {
            mongo::BSONObjBuilder item_builder;
            DataSetToBSON converter;
            converter.set_specific_character_set(this->get_specific_character_set());
            converter(sequence_it->GetNestedDataSet(), item_builder);
            sequence_builder.append(item_builder.obj());
        }
        value_builder.append(sequence_builder.arr());
    }
    else if(element.GetByteValue() == NULL)
    {
        value_builder.appendNull();
    }
    else
    {
        char const * begin = element.GetByteValue()->GetPointer();
        char const * end = begin+element.GetByteValue()->GetLength();
        if(vr == gdcm::VR::AE) this->_to_bson<gdcm::VR::AE>(begin, end, value_builder);
        else if(vr == gdcm::VR::AS) this->_to_bson<gdcm::VR::AS>(begin, end, value_builder);
        else if(vr == gdcm::VR::AT) this->_to_bson<gdcm::VR::AT>(begin, end, value_builder);
        else if(vr == gdcm::VR::CS) this->_to_bson<gdcm::VR::CS>(begin, end, value_builder);
        else if(vr == gdcm::VR::DA) this->_to_bson<gdcm::VR::DA>(begin, end, value_builder);
        else if(vr == gdcm::VR::DT) this->_to_bson<gdcm::VR::DT>(begin, end, value_builder);
        else if(vr == gdcm::VR::DS) this->_to_bson<gdcm::VR::DS>(begin, end, value_builder);
        else if(vr == gdcm::VR::FD) this->_to_bson<gdcm::VR::FD>(begin, end, value_builder);
        else if(vr == gdcm::VR::FL) this->_to_bson<gdcm::VR::FL>(begin, end, value_builder);
        else if(vr == gdcm::VR::IS) this->_to_bson<gdcm::VR::IS>(begin, end, value_builder);
        else if(vr == gdcm::VR::LO) this->_to_bson<gdcm::VR::LO>(begin, end, value_builder);
        else if(vr == gdcm::VR::LT) this->_to_bson<gdcm::VR::LT>(begin, end, value_builder);
        else if(vr == gdcm::VR::OB) this->_to_bson<gdcm::VR::OB>(begin, end, value_builder);
        else if(vr == gdcm::VR::OF) this->_to_bson<gdcm::VR::OF>(begin, end, value_builder);
        else if(vr == gdcm::VR::OW) this->_to_bson<gdcm::VR::OW>(begin, end, value_builder);
        else if(vr == gdcm::VR::PN) this->_to_bson<gdcm::VR::PN>(begin, end, value_builder);
        else if(vr == gdcm::VR::SH) this->_to_bson<gdcm::VR::SH>(begin, end, value_builder);
        // SQ is not processed here
        else if(vr == gdcm::VR::SL) this->_to_bson<gdcm::VR::SL>(begin, end, value_builder);
        else if(vr == gdcm::VR::SS) this->_to_bson<gdcm::VR::SS>(begin, end, value_builder);
        else if(vr == gdcm::VR::ST) this->_to_bson<gdcm::VR::ST>(begin, end, value_builder);
        else if(vr == gdcm::VR::TM) this->_to_bson<gdcm::VR::TM>(begin, end, value_builder);
        else if(vr == gdcm::VR::UI) this->_to_bson<gdcm::VR::UI>(begin, end, value_builder);
        else if(vr == gdcm::VR::UL) this->_to_bson<gdcm::VR::UL>(begin, end, value_builder);
        else if(vr == gdcm::VR::UN) this->_to_bson<gdcm::VR::UN>(begin, end, value_builder);
        else if(vr == gdcm::VR::US) this->_to_bson<gdcm::VR::US>(begin, end, value_builder);
        else if(vr == gdcm::VR::UT) this->_to_bson<gdcm::VR::UT>(begin, end, value_builder);
        else
        {
            throw std::runtime_error(std::string("Unhandled VR:") + gdcm::VR::GetVRString(vr));
        }
    }

    static char buffer[9];
    snprintf(buffer, 9, "%08x", element.GetTag().GetElementTag());

    builder << buffer << value_builder.arr();
}

unsigned long
DataSetToBSON
::_get_length(char const * begin, char const * end, gdcm::VR const & vr) const
{
    if(vr & (gdcm::VR::AE | gdcm::VR::AS | gdcm::VR::CS | gdcm::VR::DA |
             gdcm::VR::DS | gdcm::VR::DT | gdcm::VR::IS | gdcm::VR::LO |
             gdcm::VR::PN | gdcm::VR::SH | gdcm::VR::TM | gdcm::VR::UI))
    {
        char const * value_end = std::find(begin, end, '\\');
        return (value_end-begin);
    }
    else if(vr & (gdcm::VR::LT | gdcm::VR::ST | gdcm::VR::UT))
    {
        // LT, ST and UT may not be multi-valued
        return 1;
    }
    else if(vr == gdcm::VR::AT)
    {
        return 4;
    }
    else if(vr == gdcm::VR::FD)
    {
        return 8;
    }
    else if(vr == gdcm::VR::FL)
    {
        return 4;
    }
    else if(vr == gdcm::VR::SL)
    {
        return 4;
    }
    else if(vr == gdcm::VR::SS)
    {
        return 2;
    }
    else if(vr == gdcm::VR::UL)
    {
        return 4;
    }
    else if(vr == gdcm::VR::US)
    {
        return 2;
    }
    else
    {
        std::ostringstream message;
        message << "Cannot get length for VR " << vr;
        throw std::runtime_error(message.str());
    }
}
