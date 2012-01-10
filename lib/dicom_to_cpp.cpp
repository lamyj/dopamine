#include "dicom_to_cpp.h"

#include <sstream>
#include <string>

#include <gdcmDataElement.h>
#include <gdcmSequenceOfItems.h>
#include <gdcmVL.h>
#include <gdcmVR.h>

namespace research_pacs
{

gdcm::VL get_values_count(gdcm::DataElement const & data_element)
{
    unsigned long count=0;

    if(data_element.GetVL() == 0)
    {
        return 0;
    }
    if(data_element.GetVR() == gdcm::VR::SQ)
    {
        count = data_element.GetValueAsSQ()->GetNumberOfItems();
    }
    else if(data_element.GetVR() & (gdcm::VR::LT | gdcm::VR::ST | gdcm::VR::UT))
    {
        // LT, ST and UT may not be multi-valued
        count=1;
    }
    else if(gdcm::VR::IsASCII(data_element.GetVR()))
    {
        count = 1+std::count(
            data_element.GetByteValue()->GetPointer(),
            data_element.GetByteValue()->GetPointer()+data_element.GetVL(),
            '\\');
    }
    else
    {
        count = data_element.GetVL()/data_element.GetVR().GetSize();
    }

    return count;
}

template<>
VRToType<gdcm::VR::AE>::Type
parse<gdcm::VR::AE>(gdcm::DataElement const & data_element, unsigned long index)
{
    return parse_ascii<VRToType<gdcm::VR::AE>::Type>(data_element, index, false);
}

template<>
VRToType<gdcm::VR::AS>::Type
parse<gdcm::VR::AS>(gdcm::DataElement const & data_element, unsigned long index)
{
    return parse_ascii<VRToType<gdcm::VR::AS>::Type>(data_element, index, false);
}

template<>
VRToType<gdcm::VR::CS>::Type
parse<gdcm::VR::CS>(gdcm::DataElement const & data_element, unsigned long index)
{
    return parse_ascii<VRToType<gdcm::VR::CS>::Type>(data_element, index, false);
}

template<>
VRToType<gdcm::VR::DA>::Type
parse<gdcm::VR::DA>(gdcm::DataElement const & data_element, unsigned long index)
{
    return parse_ascii<VRToType<gdcm::VR::DA>::Type>(data_element, index, false);
}

template<>
VRToType<gdcm::VR::DT>::Type
parse<gdcm::VR::DT>(gdcm::DataElement const & data_element, unsigned long index)
{
    return parse_ascii<VRToType<gdcm::VR::DT>::Type>(data_element, index, false);
}

template<>
VRToType<gdcm::VR::DS>::Type
parse<gdcm::VR::DS>(gdcm::DataElement const & data_element, unsigned long index)
{
    std::string const value = parse_ascii<std::string>(data_element, index, false);
    std::istringstream in(value);
    in.imbue(std::locale("C"));
    VRToType<gdcm::VR::DS>::Type result;
    in >> result;
    return result;
}

template<>
VRToType<gdcm::VR::FL>::Type
parse<gdcm::VR::FL>(gdcm::DataElement const & data_element, unsigned long index)
{
    return parse_binary<VRToType<gdcm::VR::FL>::Type>(data_element, index);
}

template<>
VRToType<gdcm::VR::FD>::Type
parse<gdcm::VR::FD>(gdcm::DataElement const & data_element, unsigned long index)
{
    return parse_binary<VRToType<gdcm::VR::FD>::Type>(data_element, index);
}

template<>
VRToType<gdcm::VR::IS>::Type
parse<gdcm::VR::IS>(gdcm::DataElement const & data_element, unsigned long index)
{
    std::string const value = parse_ascii<std::string>(data_element, index, false);
    std::istringstream in(value);
    in.imbue(std::locale("C"));
    VRToType<gdcm::VR::IS>::Type result;
    in >> result;
    return result;
}

template<>
VRToType<gdcm::VR::LO>::Type
parse<gdcm::VR::LO>(gdcm::DataElement const & data_element, unsigned long index)
{
    return parse_ascii<VRToType<gdcm::VR::LO>::Type>(data_element, index, false);
}

template<>
VRToType<gdcm::VR::LT>::Type
parse<gdcm::VR::LT>(gdcm::DataElement const & data_element, unsigned long index)
{
    return parse_ascii<VRToType<gdcm::VR::LT>::Type>(data_element, index, false);
}

template<>
VRToType<gdcm::VR::PN>::Type
parse<gdcm::VR::PN>(gdcm::DataElement const & data_element, unsigned long index)
{
    return parse_ascii<VRToType<gdcm::VR::PN>::Type>(data_element, index, false);
}

template<>
VRToType<gdcm::VR::SH>::Type
parse<gdcm::VR::SH>(gdcm::DataElement const & data_element, unsigned long index)
{
    return parse_ascii<VRToType<gdcm::VR::SH>::Type>(data_element, index, false);
}

template<>
VRToType<gdcm::VR::SL>::Type
parse<gdcm::VR::SL>(gdcm::DataElement const & data_element, unsigned long index)
{
    return parse_binary<VRToType<gdcm::VR::SL>::Type>(data_element, index);
}

template<>
VRToType<gdcm::VR::SS>::Type
parse<gdcm::VR::SS>(gdcm::DataElement const & data_element, unsigned long index)
{
    return parse_binary<VRToType<gdcm::VR::SS>::Type>(data_element, index);
}

template<>
VRToType<gdcm::VR::ST>::Type
parse<gdcm::VR::ST>(gdcm::DataElement const & data_element, unsigned long index)
{
    return parse_ascii<VRToType<gdcm::VR::ST>::Type>(data_element, index, false);
}

template<>
VRToType<gdcm::VR::TM>::Type
parse<gdcm::VR::TM>(gdcm::DataElement const & data_element, unsigned long index)
{
    return parse_ascii<VRToType<gdcm::VR::TM>::Type>(data_element, index, false);
}

template<>
VRToType<gdcm::VR::UI>::Type
parse<gdcm::VR::UI>(gdcm::DataElement const & data_element, unsigned long index)
{
    return parse_ascii<VRToType<gdcm::VR::UI>::Type>(data_element, index, false);
}

template<>
VRToType<gdcm::VR::UL>::Type
parse<gdcm::VR::UL>(gdcm::DataElement const & data_element, unsigned long index)
{
    return parse_binary<VRToType<gdcm::VR::UL>::Type>(data_element, index);
}

template<>
VRToType<gdcm::VR::US>::Type
parse<gdcm::VR::US>(gdcm::DataElement const & data_element, unsigned long index)
{
    return parse_binary<VRToType<gdcm::VR::US>::Type>(data_element, index);
}

template<>
VRToType<gdcm::VR::UT>::Type
parse<gdcm::VR::UT>(gdcm::DataElement const & data_element, unsigned long index)
{
    return parse_ascii<VRToType<gdcm::VR::UT>::Type>(data_element, index, false);
}

} // namespace research_pacs