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

        for(gdcm::SequenceOfItems::ConstIterator it=sequence->Begin();
            it!=sequence->End(); ++it)
        {
            mongo::BSONObjBuilder dataset_builder;
            DataSetToBSON converter;
            converter.set_specific_character_set(this->get_specific_character_set());
            converter(it->GetNestedDataSet(), dataset_builder);
            sequence_builder.append(dataset_builder.obj());
        }

        value_builder.append(sequence_builder.arr());
    }
    else if(vr & (gdcm::VR::OB | gdcm::VR::OF | gdcm::VR::OW | gdcm::VR::UN))
    {
        unsigned long length=0;
        if(vr == gdcm::VR::OB) length = element.GetVL();
        else if(vr == gdcm::VR::OF) length = element.GetVL()/4;
        else if(vr == gdcm::VR::OW) length = element.GetVL()/2;
        else if(vr == gdcm::VR::UN) length = element.GetVL();

        mongo::BSONObjBuilder binary_data_builder;
        binary_data_builder.appendBinData("data", length, mongo::BinDataGeneral,
            element.GetByteValue()->GetPointer());
        value_builder.append(binary_data_builder.obj().getField("data"));
    }
    else if(gdcm::VR::IsBinary(vr) || gdcm::VR::IsASCII(vr))
    {
        gdcm::ByteValue const * byte_value = element.GetByteValue();

        unsigned int count = 0;
        if(byte_value == NULL)
        {
            count = 0;
        }
        else if(gdcm::VR::IsBinary(vr))
        {
            count = byte_value->GetLength()/vr.GetSize();
        }
        else // gdcm::VR::IsASCII(vr)
        {
            count = gdcm::VM::GetNumberOfElementsFromArray(
                byte_value->GetPointer(), byte_value->GetLength());
        }

        if(count == 0)
        {
            value_builder.appendNull();
        }
        else if(count == 1)
        {
            char const * begin = byte_value->GetPointer();
            char const * end = begin+byte_value->GetLength();
            // Process (begin, end)
            this->_add_element(begin, end, vr, value_builder);
        }
        else // count > 1
        {
            mongo::BSONArrayBuilder sequence_builder;

            char const * begin = byte_value->GetPointer();
            char const * end = begin+byte_value->GetLength();
            for(unsigned int i=0; i<count; ++i)
            {
                char const * item_end = begin+this->_get_length(begin, end, vr)+1;
                this->_add_element(begin, item_end, vr, sequence_builder);
                begin = item_end;
            }
            value_builder.append(sequence_builder.arr());
        }
    }

    static char buffer[9];
    snprintf(buffer, 9, "%08x", element.GetTag().GetElementTag());

    builder << buffer << value_builder.arr();
}

void
DataSetToBSON
::_add_element(char const * begin, char const * end, gdcm::VR const & vr,
                  mongo::BSONArrayBuilder & builder) const
{
    static std::string const whitespace(" \0", 2);
    static std::string const whitespace_and_backslash(" \0\\", 3);

    if(vr & (gdcm::VR::AE | gdcm::VR::AS | gdcm::VR::CS | gdcm::VR::DA |
             gdcm::VR::DT | gdcm::VR::TM | gdcm::VR::UI))
    {
        // ASCII text content, leading and trailing whitespaces are not significant
        char const * first = find_first_not_of(begin, end,
             whitespace_and_backslash.begin(), whitespace_and_backslash.end());

        unsigned int size=0;
        if(first != end)
        {
            char const * last = find_last_not_of(begin, end,
                whitespace_and_backslash.begin(), whitespace_and_backslash.end());
            size = last-first+1;
        }
        builder.append(std::string(first, size));
    }
    else if(vr & (gdcm::VR::LO | gdcm::VR::LT | gdcm::VR::PN | gdcm::VR::SH |
                  gdcm::VR::ST | gdcm::VR::UT))
    {
        // Non-ASCII text content

        char const * first;
        if(vr & (gdcm::VR::LT | gdcm::VR::ST | gdcm::VR::UT))
        {
            // Leading spaces are significant for LT, ST, and UT
            first = find_first_not_of(begin, end,
                whitespace.begin(), whitespace.end());
        }
        else
        {
            first = find_first_not_of(begin, end,
                whitespace_and_backslash.begin(), whitespace_and_backslash.end());
        }

        unsigned int size=0;
        if(first != end)
        {
            char const * last;
            if(vr & (gdcm::VR::LT | gdcm::VR::ST | gdcm::VR::UT))
            {
                // LT, ST and UT may not be multi-valued
                last = end-1;
            }
            else
            {
                last = find_last_not_of(begin, end,
                    whitespace_and_backslash.begin(), whitespace_and_backslash.end());
            }
            size = last-first+1;
        }

        unsigned long buffer_size = size*4; // worst case: ascii->UCS-32
        char* buffer = new char[buffer_size];
        std::fill(buffer, buffer+buffer_size, 0);

        size_t inbytesleft=size;
        size_t outbytesleft=buffer_size;
        char* inbuf = const_cast<char*>(first);
        char* outbuf = buffer;

        size_t const result = iconv(this->_converter,
            &inbuf, &inbytesleft, &outbuf, &outbytesleft);
        if(result == size_t(-1))
        {
            throw std::runtime_error(std::string("iconv error ")+strerror(errno));
        }

        builder.append(std::string(buffer, buffer_size-outbytesleft));

        delete[] buffer;
    }
    else if(vr == gdcm::VR::AT)
    {
        // TODO : AT
    }
    else if(vr == gdcm::VR::DS)
    {
        char * old_numeric = setlocale(LC_NUMERIC, NULL);
        setlocale(LC_NUMERIC, "C");
        char* endptr;
        double const d = std::strtod(begin, &endptr);
        setlocale(LC_NUMERIC, old_numeric);

        if(endptr == begin)
        {
            throw std::runtime_error("Cannot parse DS");
        }

        builder.append(d);
    }
    else if(vr == gdcm::VR::FD)
    {
        // TODO : correct 32 bits type
        builder.append(*reinterpret_cast<float const *>(begin));
    }
    else if(vr == gdcm::VR::FL)
    {
        // TODO : correct 64 bits type
        builder.append(*reinterpret_cast<double const *>(begin));
    }
    else if(vr == gdcm::VR::IS)
    {
        char * old_numeric = setlocale(LC_NUMERIC, NULL);
        setlocale(LC_NUMERIC, "C");
        char* endptr;
        long const d = std::strtol(begin, &endptr, 10);
        setlocale(LC_NUMERIC, old_numeric);

        if(endptr == begin)
        {
            throw std::runtime_error("Cannot parse IS");
        }

        builder.append(static_cast<long long>(d));
    }
    else if(vr == gdcm::VR::SL)
    {
        builder.append(*reinterpret_cast<int32_t const *>(begin));
    }
    else if(vr == gdcm::VR::SS)
    {
        builder.append(*reinterpret_cast<int16_t const *>(begin));
    }
    else if(vr == gdcm::VR::UL)
    {
        builder.append(*reinterpret_cast<uint32_t const *>(begin));
    }
    else if(vr == gdcm::VR::US)
    {
        builder.append(*reinterpret_cast<uint16_t const *>(begin));
    }
    else
    {
        std::ostringstream message;
        message << "Cannot parse VR " << vr;
        throw std::runtime_error(message.str());
    }
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
        return 4;
    }
    else if(vr == gdcm::VR::FL)
    {
        return 8;
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
