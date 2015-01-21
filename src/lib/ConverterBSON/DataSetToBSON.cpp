/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <errno.h>

#include "DataSetToBSON.h"

namespace dopamine
{

DataSetToBSON
::DataSetToBSON()
: ConverterBSONDataSet(true),
  _default_filter(FilterAction::INCLUDE)
{
    this->set_specific_character_set("");
}

DataSetToBSON
::~DataSetToBSON()
{
    // Nothing to do
}

DataSetToBSON::FilterAction::Type const &
DataSetToBSON
::get_default_filter() const
{
    return this->_default_filter;
}

void
DataSetToBSON
::set_default_filter(DataSetToBSON::FilterAction::Type const & action)
{
    this->_default_filter = action;
}

std::vector<DataSetToBSON::Filter> &
DataSetToBSON
::get_filters()
{
    return this->_filters;
}

void
DataSetToBSON
::set_filters(std::vector<DataSetToBSON::Filter> const & filters)
{
    this->_filters = filters;
}

void
DataSetToBSON
::operator()(DcmObject * dataset, mongo::BSONObjBuilder & builder)
{
    DcmObject * it = NULL;
    while(NULL != (it = dataset->nextInContainer(it)))
    {
        FilterAction::Type action = FilterAction::UNKNOWN;
        for(std::vector<Filter>::const_iterator filters_it=this->_filters.begin();
            filters_it != this->_filters.end(); ++filters_it)
        {
            Condition const & condition = *(filters_it->first);
            DcmElement * element = dynamic_cast<DcmElement*>(it);
            if(condition(element))
            {
                action = filters_it->second;
                break;
            }
        }
        if(action == FilterAction::UNKNOWN)
        {
            action = this->_default_filter;
        }

        if(action == FilterAction::EXCLUDE)
        {
            continue;
        }

        if(it->getTag() == DCM_SpecificCharacterSet)
        {
            // Specific Character Set: setup internal iconv converter
            DcmCodeString * specific_character_set = 
                dynamic_cast<DcmCodeString*>(it);
            char* value;
            OFCondition condition = specific_character_set->getString(value);
            if (condition.bad())
            {
                std::stringstream stream;
                stream << "Cannot find specific character set: " << condition.text();
                throw dopamine::ExceptionPACS(stream.str());
            }
            this->set_specific_character_set(value);
        }

        if(it->getETag() == 0)
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
    this->_to_bson_number<Uint32, unsigned>(
        dynamic_cast<DcmElement*>(element), &DcmElement::getUint32, builder);
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
    this->_to_bson_number<Float64, double>(
        dynamic_cast<DcmElement*>(element), &DcmElement::getFloat64, builder);
}

template<>
void
DataSetToBSON::_to_bson<EVR_FL>(DcmObject * element,
                                mongo::BSONArrayBuilder & builder) const
{
    this->_to_bson_number<Float32, double>(
        dynamic_cast<DcmElement*>(element), &DcmElement::getFloat32, builder);
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
            sub_builder.append<int>(value);
        }

        builder.append(sub_builder.arr());
    }
    else
    {
        Sint32 value;
        is->getSint32(value, 0);
        builder.append<int>(value);
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
    this->_to_bson_number<Sint32, int>(
        dynamic_cast<DcmElement*>(element), &DcmElement::getSint32, builder);
}

// SQ is not processed here

template<>
void
DataSetToBSON::_to_bson<EVR_SS>(DcmObject * element,
                                mongo::BSONArrayBuilder & builder) const
{
    this->_to_bson_number<Sint16, int>(
        dynamic_cast<DcmElement*>(element), &DcmElement::getSint16, builder);
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
    this->_to_bson_number<Uint32, unsigned>(
        dynamic_cast<DcmElement*>(element), &DcmElement::getUint32, builder);
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
    this->_to_bson_number<Uint16, unsigned>(
        dynamic_cast<DcmElement*>(element), &DcmElement::getUint16, builder);
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

    mongo::BSONArrayBuilder sub_builder;

    mongo::BSONArrayBuilder * const current_builder =
        (count>1)?&sub_builder:&builder;

    for(unsigned long i=0; i<count; ++i)
    {
        OFString value;
        element->getOFString(value, i);
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

            size_t const result = iconv(this->get_converter(),
                &inbuf, &inbytesleft, &outbuf, &outbytesleft);
            if(result == size_t(-1))
            {
                throw std::runtime_error(std::string("iconv error ")+strerror(errno));
            }

            value = OFString(buffer, buffer_size-outbytesleft);
        }

        current_builder->append(std::string(value.c_str()));

        if(use_utf8)
        {
            delete[] buffer;
        }
    }

    if(count > 1)
    {
        builder.append(sub_builder.arr());
    }
}

template<typename TDICOMValue, typename TBSONValue>
void
DataSetToBSON::_to_bson_number(DcmElement * element,
    OFCondition (DcmElement::*getter)(TDICOMValue &, unsigned long),
    mongo::BSONArrayBuilder & builder) const
{
    unsigned long count = element->getVM();
    if(count > 1)
    {
        mongo::BSONArrayBuilder sub_builder;
        for(unsigned long i=0; i<count; ++i)
        {
            TDICOMValue value;
            (element->*getter)(value, i);
            sub_builder.append<TBSONValue>(value);
        }
        builder.append(sub_builder.arr());
    }   
    else
    {   
        TDICOMValue value;
        (element->*getter)(value, 0);
        builder.append<TBSONValue>(value);
    }   
}

void
DataSetToBSON::_to_bson_binary(DcmElement * element,
                               mongo::BSONArrayBuilder & builder) const
{
    DcmOtherByteOtherWord* byte_string = dynamic_cast<DcmOtherByteOtherWord*>(element);
    if(byte_string != NULL)
    {
        void* begin(NULL);
        if(element->getVR() != EVR_OW)
        {
            byte_string->getUint8Array(*reinterpret_cast<Uint8**>(&begin));
        }
        else
        {
            byte_string->getUint16Array(*reinterpret_cast<Uint16**>(&begin));
        }

        mongo::BSONObjBuilder binary_data_builder;
        binary_data_builder.appendBinData("data", element->getLength(),
                                          mongo::BinDataGeneral, begin);
        builder.append(binary_data_builder.obj().getField("data"));
    }
    else
    {
        throw std::runtime_error(
            std::string("Cannot handle conversion of ")+
                DcmVR(element->getVR()).getValidVRName());
    }
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
        
        // OB or OW: depending on context
        else if (vr == EVR_ox)
        {
            if (std::string(DcmVR(vr).getValidVRName()) == "OW")
            {
                this->_to_bson<EVR_OW>(element, value_builder);
            }
            else // if (std::string(DcmVR(vr).getValidVRName()) == "OB")
            {
                this->_to_bson<EVR_OB>(element, value_builder);
            }
        }
        
        // US or SS: depending on context
        else if (vr == EVR_xs)
        {
            if (std::string(DcmVR(vr).getValidVRName()) == "SS")
            {
                this->_to_bson<EVR_SS>(element, value_builder);
            }
            else // if (std::string(DcmVR(vr).getValidVRName()) == "US")
            {
                this->_to_bson<EVR_US>(element, value_builder);
            }
        }
        
        // US, SS or OW: depending on context
        else if (vr == EVR_lt)
        {
            if (std::string(DcmVR(vr).getValidVRName()) == "OW")
            {
                this->_to_bson<EVR_OW>(element, value_builder);
            }
            else if (std::string(DcmVR(vr).getValidVRName()) == "SS")
            {
                this->_to_bson<EVR_SS>(element, value_builder);
            }
            else // if (std::string(DcmVR(vr).getValidVRName()) == "US")
            {
                this->_to_bson<EVR_US>(element, value_builder);
            }
        }
        
        // default
        else
        {
            throw std::runtime_error(std::string("Unhandled VR:") + DcmVR(vr).getValidVRName());
        }
    }

    static char buffer[9];
    snprintf(buffer, 9, "%04x%04x", element->getGTag(), element->getETag());

    builder << buffer << value_builder.arr();
}

} // namespace dopamine
