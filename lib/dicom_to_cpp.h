#ifndef _da46f3d6_6ce1_4ed4_9f67_b007637b1674
#define _da46f3d6_6ce1_4ed4_9f67_b007637b1674

#include "gdcmDataElement.h"
#include "gdcmDataSet.h"
#include "gdcmVL.h"

namespace research_pacs
{

template<typename Action>
void parse(gdcm::DataSet const & data_set, Action & action);

template<typename Action>
void parse(gdcm::DataElement const & data_element, Action & action);

// GDCM forces an even-length string, we don't want this
template<gdcm::VR::VRType VVR>
struct VRToType;

template<> struct VRToType<gdcm::VR::AE> { typedef std::string Type; };
template<> struct VRToType<gdcm::VR::AS> { typedef std::string Type; };
template<> struct VRToType<gdcm::VR::CS> { typedef std::string Type; };
template<> struct VRToType<gdcm::VR::DS> { typedef double Type; };
template<> struct VRToType<gdcm::VR::DA> { typedef std::string Type; };
template<> struct VRToType<gdcm::VR::DT> { typedef std::string Type; };
template<> struct VRToType<gdcm::VR::FL> { typedef float Type; };
template<> struct VRToType<gdcm::VR::FD> { typedef double Type; };
template<> struct VRToType<gdcm::VR::IS> { typedef int32_t Type; };
template<> struct VRToType<gdcm::VR::LO> { typedef std::string Type; };
template<> struct VRToType<gdcm::VR::LT> { typedef std::string Type; };
template<> struct VRToType<gdcm::VR::PN> { typedef std::string Type; };
template<> struct VRToType<gdcm::VR::SH> { typedef std::string Type; };
template<> struct VRToType<gdcm::VR::SL> { typedef int32_t Type; };
template<> struct VRToType<gdcm::VR::SS> { typedef int16_t Type; };
template<> struct VRToType<gdcm::VR::ST> { typedef std::string Type; };
template<> struct VRToType<gdcm::VR::TM> { typedef std::string Type; };
template<> struct VRToType<gdcm::VR::UI> { typedef std::string Type; };
template<> struct VRToType<gdcm::VR::UL> { typedef uint32_t Type; };
template<> struct VRToType<gdcm::VR::US> { typedef uint16_t Type; };
template<> struct VRToType<gdcm::VR::UT> { typedef std::string Type; };

gdcm::VL get_values_count(gdcm::DataElement const & data_element);

template<typename T>
T parse_ascii(gdcm::DataElement const & data_element, unsigned long index, bool is_padding_significant);

template<typename T>
T parse_binary(gdcm::DataElement const & data_element, unsigned long index=0);

template<gdcm::VR::VRType VVR>
typename VRToType<VVR>::Type parse(gdcm::DataElement const & data_element, unsigned long index=0);

} // namespace research_pacs

#include "dicom_to_cpp.txx"

#endif // _da46f3d6_6ce1_4ed4_9f67_b007637b1674
