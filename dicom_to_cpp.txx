#ifndef _69efd62b_5e38_4cd9_a5f3_d00be3019fdb
#define _69efd62b_5e38_4cd9_a5f3_d00be3019fdb

#include "dicom_to_cpp.h"

#include <algorithm>
#include <string>

#include <gdcmDataElement.h>
#include <gdcmVL.h>

template<typename Action>
void parse(gdcm::DataSet const & data_set, Action & action)
{
    action.begin_data_set(data_set);
    for(gdcm::DataSet::ConstIterator it=data_set.Begin();
        it!=data_set.End(); ++it)
    {
        action.data_element(*it);
    }
    action.end_data_set(data_set);
}

template<typename Action>
void parse(gdcm::DataElement const & data_element, Action & action)
{
    unsigned long const count = get_values_count(data_element);

    #define VR_CASE(vr) \
        if(vr == data_element.GetVR()) \
        { \
            if(count == 1) \
            { \
                action.template single_valued_element<vr>(data_element); \
            } \
            else \
            { \
                action.template begin_multiple_valued_element<vr>(data_element); \
                for(unsigned long i=0; i<count; ++i) \
                { \
                    action.template multiple_valued_element<vr>(data_element, i); \
                } \
                action.template end_multiple_valued_element<vr>(data_element); \
            }\
        }

    VR_CASE(gdcm::VR::AE);
    VR_CASE(gdcm::VR::AS);
//    VR_CASE(gdcm::VR::AT);
    VR_CASE(gdcm::VR::CS);
    VR_CASE(gdcm::VR::DA);
    VR_CASE(gdcm::VR::DS);
    VR_CASE(gdcm::VR::DT);
    VR_CASE(gdcm::VR::FD);
    VR_CASE(gdcm::VR::FL);
    VR_CASE(gdcm::VR::IS);
    VR_CASE(gdcm::VR::LO);
    VR_CASE(gdcm::VR::LT);
//    VR_CASE(gdcm::VR::OB);
//    VR_CASE(gdcm::VR::OF);
//    VR_CASE(gdcm::VR::OW);
    VR_CASE(gdcm::VR::PN);
    VR_CASE(gdcm::VR::SH);
    VR_CASE(gdcm::VR::SL);
    // SQ is special case !
//    VR_CASE(gdcm::VR::SQ);
    VR_CASE(gdcm::VR::SS);
    VR_CASE(gdcm::VR::ST);
    VR_CASE(gdcm::VR::TM);
    VR_CASE(gdcm::VR::UI);
    VR_CASE(gdcm::VR::UL);
//    VR_CASE(gdcm::VR::UN);
    VR_CASE(gdcm::VR::US);
    VR_CASE(gdcm::VR::UT);

    if(data_element.GetVR() == gdcm::VR::SQ)
    {
        action.sequence(data_element);
    }

#undef VR_CASE
}

template<typename T>
T parse_ascii(gdcm::DataElement const & data_element, unsigned long index, bool is_padding_significant)
{
    char const * const data = data_element.GetByteValue()->GetPointer();
    gdcm::VL const length = data_element.GetVL();

    char const * begin = data;
    for(unsigned long i=0; i<index; ++i)
    {
        begin = std::find(begin, data+length, '\\');
        ++begin;
    }
    char const * end = std::find(begin, data+length, '\\');

    if(!is_padding_significant)
    {
        while(*begin == ' ')
        {
            ++begin;
        }

        if(end == data+length)
        {
            --end;
        }
        while(*end == ' ' || *end == '\\' || *end == '\0')
        {
            --end;
        }
        ++end;
    }

    return T(std::string(begin, end));
}

template<typename T>
T parse_binary(gdcm::DataElement const & data_element, unsigned long index=0)
{
    T const * begin = reinterpret_cast<T const *>(data_element.GetByteValue()->GetPointer());
    return *(begin+index);
}

#endif // _69efd62b_5e38_4cd9_a5f3_d00be3019fdb
