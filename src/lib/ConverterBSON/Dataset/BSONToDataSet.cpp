/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <locale>

#include <errno.h>

#include "BSONToDataSet.h"
#include "core/ExceptionPACS.h"
#include "services/ServicesTools.h"

namespace dopamine
{

namespace converterBSON
{

BSONToDataSet
::BSONToDataSet()
: ConverterBSONDataSet(false)
{
    this->set_specific_character_set("");
}

BSONToDataSet
::~BSONToDataSet()
{
    // Nothing to do
}

DcmDataset
BSONToDataSet
::operator()(mongo::BSONObj const & bson)
{
    if(bson.hasField("00080005") && !bson["00080005"].isNull())
    {
        // Specific Character Set: map to iconv encoding
        std::string value = bson["00080005"].Array()[1].String();
        if(value.size()%2 != 0)
        {
            value += ' ';
        }
        // TODO : multi-valued Specific Character Set
        this->set_specific_character_set(value);
    }

    DcmDataset dataset;
    for(mongo::BSONObj::iterator it = bson.begin(); it.more();)
    {
        mongo::BSONElement const element_bson = it.next();

        if(element_bson.isNull())
        {
            // Skip null elements (might happen when bson is a query result)
            continue;
        }

        // Skip elements that do not look like DICOM tags
        std::string const field_name = element_bson.fieldName();
        bool skip_field = (field_name.size()!=8);
        if(!skip_field)
        {
            for(std::string::const_iterator field_name_it=field_name.begin();
                field_name_it!=field_name.end(); ++field_name_it)
            {
                if(!((*field_name_it>='0' && *field_name_it<='9') ||
                     (*field_name_it>='a' && *field_name_it<='f') ||
                     (*field_name_it>='A' && *field_name_it<='F')))
                {
                    skip_field = true;
                    break;
                }
            }
        }

        if(skip_field)
        {
            continue;
        }
        this->_add_element(element_bson, dataset);
    }

    return dataset;
}

/*******************************************************************************
 * Specializations of BSONToDataSet::_to_dcmtk for the different VRs.
 ******************************************************************************/

template<>
void
BSONToDataSet
::_to_dcmtk<EVR_AE>(mongo::BSONElement const & bson, DcmDataset & dataset,
                    DcmTag const & tag) const
{
    this->_to_text(bson, false, ' ', dataset, tag);
}

template<>
void
BSONToDataSet
::_to_dcmtk<EVR_AS>(mongo::BSONElement const & bson, DcmDataset & dataset,
                    DcmTag const & tag) const
{
    this->_to_text(bson, false, ' ', dataset, tag);
}

template<>
void
BSONToDataSet
::_to_dcmtk<EVR_AT>(mongo::BSONElement const & bson, DcmDataset & dataset,
                    DcmTag const & tag) const
{
    mongo::BSONArrayBuilder arraybuilder;
    for (auto value : bson.Array())
    {
        std::string value_at = value.String();
        value_at = dopamine::services::replace(value_at, "(", "");
        value_at = dopamine::services::replace(value_at, ")", "");
        value_at = dopamine::services::replace(value_at, ",", "");

        char * old_numeric = setlocale(LC_NUMERIC, NULL);
        setlocale(LC_NUMERIC, "C");
        char* endptr;
        long const d = std::strtol(value_at.c_str(), &endptr, 16);
        setlocale(LC_NUMERIC, old_numeric);

        DcmTag const tag(d>>16, d&0xffff);

        arraybuilder << tag.getGroup() << tag.getElement();
    }

    this->_to_binary<Uint16, int>(BSON("data" << arraybuilder.arr()).getField("data"),
                                  &mongo::BSONElement::Int,
                                  dataset, tag, &DcmElement::putUint16Array);
}

template<>
void
BSONToDataSet
::_to_dcmtk<EVR_CS>(mongo::BSONElement const & bson, DcmDataset & dataset,
                    DcmTag const & tag) const
{
    this->_to_text(bson, false, ' ', dataset, tag);
}

template<>
void
BSONToDataSet
::_to_dcmtk<EVR_DA>(mongo::BSONElement const & bson, DcmDataset & dataset,
                    DcmTag const & tag) const
{
    this->_to_text(bson, false, ' ', dataset, tag);
}

template<>
void
BSONToDataSet
::_to_dcmtk<EVR_DS>(mongo::BSONElement const & bson, DcmDataset & dataset,
                    DcmTag const & tag) const
{
    this->_to_number_string(bson, dataset, tag);
}

template<>
void
BSONToDataSet
::_to_dcmtk<EVR_DT>(mongo::BSONElement const & bson, DcmDataset & dataset,
                    DcmTag const & tag) const
{
    this->_to_text(bson, false, ' ', dataset, tag);
}

template<>
void
BSONToDataSet
::_to_dcmtk<EVR_FD>(mongo::BSONElement const & bson, DcmDataset & dataset,
                    DcmTag const & tag) const
{
    this->_to_binary<Float64, double>(bson, &mongo::BSONElement::Double,
                                      dataset, tag, &DcmElement::putFloat64Array);
}

template<>
void
BSONToDataSet
::_to_dcmtk<EVR_FL>(mongo::BSONElement const & bson, DcmDataset & dataset,
                    DcmTag const & tag) const
{
    this->_to_binary<Float32, double>(bson, &mongo::BSONElement::Double,
                                      dataset, tag, &DcmElement::putFloat32Array);
}

template<>
void
BSONToDataSet
::_to_dcmtk<EVR_IS>(mongo::BSONElement const & bson, DcmDataset & dataset,
                   DcmTag const & tag) const
{
    this->_to_number_string(bson, dataset, tag);
}

template<>
void
BSONToDataSet
::_to_dcmtk<EVR_LO>(mongo::BSONElement const & bson, DcmDataset & dataset,
                    DcmTag const & tag) const
{
    this->_to_text(bson, true, ' ', dataset, tag);
}

template<>
void
BSONToDataSet
::_to_dcmtk<EVR_LT>(mongo::BSONElement const & bson, DcmDataset & dataset,
                    DcmTag const & tag) const
{
    this->_to_text(bson, true, ' ', dataset, tag);
}

template<>
void
BSONToDataSet
::_to_dcmtk<EVR_OB>(mongo::BSONElement const & bson, DcmDataset & dataset,
                    DcmTag const & tag) const
{
    this->_to_raw(bson, dataset, tag);
}

template<>
void
BSONToDataSet
::_to_dcmtk<EVR_OF>(mongo::BSONElement const & bson, DcmDataset & dataset,
                    DcmTag const & tag) const
{
    this->_to_raw(bson, dataset, tag);
}

template<>
void
BSONToDataSet
::_to_dcmtk<EVR_OW>(mongo::BSONElement const & bson, DcmDataset & dataset,
                    DcmTag const & tag) const
{
    this->_to_raw(bson, dataset, tag);
}

template<>
void
BSONToDataSet
::_to_dcmtk<EVR_PN>(mongo::BSONElement const & bson, DcmDataset & dataset,
                    DcmTag const & tag) const
{
    mongo::BSONArrayBuilder arraybuilder;
    for (auto value : bson.Array())
    {
        if (value.Obj().hasField("Alphabetic"))
        {
            arraybuilder << value.Obj().getField("Alphabetic");
        }
    }

    this->_to_text(BSON("data" << arraybuilder.arr()).getField("data"), true, ' ', dataset, tag);
}

template<>
void
BSONToDataSet
::_to_dcmtk<EVR_SH>(mongo::BSONElement const & bson, DcmDataset & dataset,
                    DcmTag const & tag) const
{
    this->_to_text(bson, true, ' ', dataset, tag);
}

template<>
void
BSONToDataSet
::_to_dcmtk<EVR_SL>(mongo::BSONElement const & bson, DcmDataset & dataset,
                    DcmTag const & tag) const
{
    this->_to_binary<Sint32, int>(bson, &mongo::BSONElement::Int,
                                  dataset, tag, &DcmElement::putSint32Array);
}

// SQ is not processed here

template<>
void
BSONToDataSet
::_to_dcmtk<EVR_SS>(mongo::BSONElement const & bson, DcmDataset & dataset,
                    DcmTag const & tag) const
{
    this->_to_binary<Sint16, int>(bson, &mongo::BSONElement::Int,
                                  dataset, tag, &DcmElement::putSint16Array);
}

template<>
void
BSONToDataSet
::_to_dcmtk<EVR_ST>(mongo::BSONElement const & bson, DcmDataset & dataset,
                    DcmTag const & tag) const
{
    this->_to_text(bson, true, ' ', dataset, tag);
}

template<>
void
BSONToDataSet
::_to_dcmtk<EVR_TM>(mongo::BSONElement const & bson, DcmDataset & dataset,
                    DcmTag const & tag) const
{
    this->_to_text(bson, false, ' ', dataset, tag);
}

template<>
void
BSONToDataSet
::_to_dcmtk<EVR_UI>(mongo::BSONElement const & bson, DcmDataset & dataset,
                    DcmTag const & tag) const
{
    this->_to_text(bson, false, '\0', dataset, tag);
}


template<>
void
BSONToDataSet
::_to_dcmtk<EVR_UL>(mongo::BSONElement const & bson, DcmDataset & dataset,
                    DcmTag const & tag) const
{
    this->_to_binary<Uint32, int>(bson, &mongo::BSONElement::Int,
                                  dataset, tag, &DcmElement::putUint32Array);
}

template<>
void
BSONToDataSet
::_to_dcmtk<EVR_UN>(mongo::BSONElement const & bson, DcmDataset & dataset,
                    DcmTag const & tag) const
{
    this->_to_raw(bson, dataset, tag);
}

template<>
void
BSONToDataSet
::_to_dcmtk<EVR_US>(mongo::BSONElement const & bson, DcmDataset & dataset,
                    DcmTag const & tag) const
{
    this->_to_binary<Uint16, int>(bson, &mongo::BSONElement::Int,
                                  dataset, tag, &DcmElement::putUint16Array);
}

template<>
void
BSONToDataSet
::_to_dcmtk<EVR_UT>(mongo::BSONElement const & bson, DcmDataset & dataset,
                    DcmTag const & tag) const
{
    this->_to_text(bson, true, ' ', dataset, tag);
}

/*******************************************************************************
 * End of specializations of BSONToDataSet::_to_dcmtk for the different VRs.
 ******************************************************************************/

void
BSONToDataSet
::_add_element(mongo::BSONElement const & bson, DcmDataset & dataset)
{
    // Get the tag from the field name
    std::string const field_name = bson.fieldName();
    char * old_numeric = setlocale(LC_NUMERIC, NULL);
    setlocale(LC_NUMERIC, "C");
    char* endptr;
    long const d = std::strtol(field_name.c_str(), &endptr, 16);
    setlocale(LC_NUMERIC, old_numeric);

    DcmTag const tag(d>>16, d&0xffff);

    // Value holding the VR and the data
    mongo::BSONObj const object = bson.Obj();

    // Get the VR : first item of value
    DcmVR const vr(object.getField("vr").String().c_str());
    DcmEVR const evr(vr.getEVR());

    mongo::BSONElement element;
    if (evr == EVR_OB || evr == EVR_OF || evr == EVR_OW || evr == EVR_UN)
    {
        element = object.getField("InlineBinary");
    }
    else
    {
        element = object.getField("Value");
    }
    if(!element.isNull() && ((element.type() == mongo::BSONType::Array && element.Array().size() != 0) ||
                              element.type() == mongo::BSONType::BinData))
    {
        if(evr == EVR_SQ)
        {
            for(unsigned int i = 0; i < element.Array().size(); ++i)
            {
                DcmItem* item = NULL;

                mongo::BSONElement const subelement = element.Array()[i];

                if (!subelement.isNull())
                {
                    BSONToDataSet converter;
                    converter.set_specific_character_set(this->get_specific_character_set());
                    DcmDataset itemdataset = converter(subelement.Obj());

                    item = new DcmItem(itemdataset);
                }

                OFCondition condition = dataset.insertSequenceItem(tag, item, -2); // -2 = create new
                if (condition.bad())
                {
                    std::stringstream streamerror;
                    streamerror << "Cannot insert sequence '" << tag.toString().c_str()
                                << "': " << condition.text();
                    throw ExceptionPACS(streamerror.str());
                }
            }
        }
        else
        {
            if(evr == EVR_AE) this->_to_dcmtk<EVR_AE>(element, dataset, tag);
            else if(evr == EVR_AS) this->_to_dcmtk<EVR_AS>(element, dataset, tag);
            else if(evr == EVR_AT) this->_to_dcmtk<EVR_AT>(element, dataset, tag);
            else if(evr == EVR_CS) this->_to_dcmtk<EVR_CS>(element, dataset, tag);
            else if(evr == EVR_DA) this->_to_dcmtk<EVR_DA>(element, dataset, tag);
            else if(evr == EVR_DS) this->_to_dcmtk<EVR_DS>(element, dataset, tag);
            else if(evr == EVR_DT) this->_to_dcmtk<EVR_DT>(element, dataset, tag);
            else if(evr == EVR_FD) this->_to_dcmtk<EVR_FD>(element, dataset, tag);
            else if(evr == EVR_FL) this->_to_dcmtk<EVR_FL>(element, dataset, tag);
            else if(evr == EVR_IS) this->_to_dcmtk<EVR_IS>(element, dataset, tag);
            else if(evr == EVR_LO) this->_to_dcmtk<EVR_LO>(element, dataset, tag);
            else if(evr == EVR_LT) this->_to_dcmtk<EVR_LT>(element, dataset, tag);
            else if(evr == EVR_OB) this->_to_dcmtk<EVR_OB>(element, dataset, tag);
            else if(evr == EVR_OF) this->_to_dcmtk<EVR_OF>(element, dataset, tag);
            else if(evr == EVR_OW) this->_to_dcmtk<EVR_OW>(element, dataset, tag);
            else if(evr == EVR_PN) this->_to_dcmtk<EVR_PN>(element, dataset, tag);
            else if(evr == EVR_SH) this->_to_dcmtk<EVR_SH>(element, dataset, tag);
            // // SQ is not processed here
            else if(evr == EVR_SL) this->_to_dcmtk<EVR_SL>(element, dataset, tag);
            else if(evr == EVR_SS) this->_to_dcmtk<EVR_SS>(element, dataset, tag);
            else if(evr == EVR_ST) this->_to_dcmtk<EVR_ST>(element, dataset, tag);
            else if(evr == EVR_TM) this->_to_dcmtk<EVR_TM>(element, dataset, tag);
            else if(evr == EVR_UI) this->_to_dcmtk<EVR_UI>(element, dataset, tag);
            else if(evr == EVR_UL) this->_to_dcmtk<EVR_UL>(element, dataset, tag);
            else if(evr == EVR_UN) this->_to_dcmtk<EVR_UN>(element, dataset, tag);
            else if(evr == EVR_US) this->_to_dcmtk<EVR_US>(element, dataset, tag);
            else if(evr == EVR_UT) this->_to_dcmtk<EVR_UT>(element, dataset, tag);
            
            // default
            else
            {
                throw std::runtime_error(std::string("Unhandled VR:") + vr.getValidVRName());
            }
        }
    }
    else
    {
        dataset.insertEmptyElement(tag);
    }
}

void
BSONToDataSet
::_to_text(mongo::BSONElement const & bson, bool use_utf8, char padding,
           DcmDataset & dataset, DcmTag const & tag) const
{
    std::vector<mongo::BSONElement> elements = bson.Array();

    OFString value;

    std::vector<mongo::BSONElement>::const_iterator const last_it = --elements.end();
    for(std::vector<mongo::BSONElement>::const_iterator it=elements.begin();
        it != elements.end(); ++it)
    {
        std::string const element = it->isNull() ? "" : it->String();
        
        if(use_utf8)
        {
            unsigned long size = element.size();
            unsigned long buffer_size = size*4; // worst case: UTF-8 with only ascii->UCS-32
            char* buffer = new char[buffer_size];
            std::fill(buffer, buffer+buffer_size, 0);

            size_t inbytesleft=size;
            size_t outbytesleft=buffer_size;
            char* inbuf = const_cast<char*>(&element[0]);
            char* outbuf = buffer;

            size_t const result = iconv(this->get_converter(),
                &inbuf, &inbytesleft, &outbuf, &outbytesleft);
            if(result == size_t(-1))
            {
                throw std::runtime_error(std::string("iconv error ") +
                                         strerror(errno));
            }

            value += OFString(buffer, buffer_size-outbytesleft);

            delete[] buffer;
        }
        else
        {
            value += element.c_str();
        }

        if(it != last_it)
        {
            value += "\\";
        }
    }

    if(value.size()%2!=0)
    {
        value += padding;
    }
    
    OFCondition condition = dataset.putAndInsertOFStringArray(tag, value);
    if (condition.bad())
    {
        std::stringstream streamerror;
        streamerror << "Cannot set element '" << tag.toString().c_str()
                    << "': " << condition.text();
        throw ExceptionPACS(streamerror.str());
    }
}

template<typename TDCMTKType, typename TBSONType>
void
BSONToDataSet
::_to_binary(mongo::BSONElement const & bson,
             typename BSONGetterType<TBSONType>::Type getter,
             DcmDataset & dataset, DcmTag const & tag,
             typename DCMTKSetterType<TDCMTKType>::Type setter) const
{
    DcmElement * element;
    OFCondition condition = dataset.findAndGetElement(tag.getXTag(), element);
    // If Tag does not exist, create it
    if (condition == EC_TagNotFound)
    {
        if (tag.getEVR() != EVR_xs)
        {
            condition = dataset.insertEmptyElement(tag);
        }
        else
        {
            // InsertEmptyElement cannot process with VR = "xs"
            // GetValidEVR => return "US" or "SS"
            DcmTag tagtemp(tag.getGroup(), tag.getElement(), tag.getVR().getValidEVR());
            condition = dataset.insertEmptyElement(tagtemp);
        }

        // Get the created element
        if (condition.good())
        {
            condition = dataset.findAndGetElement(tag.getXTag(), element);
        }
    }

    // Throw exception if element cannot not be retrieve
    if (condition.bad())
    {
        std::stringstream streamerror;
        streamerror << "Cannot get element '" << tag.toString().c_str()
                    << "': " << condition.text();
        throw ExceptionPACS(streamerror.str());
    }

    std::vector<mongo::BSONElement> const elements = bson.Array();
    unsigned long index=0;

    std::vector<TDCMTKType> array; array.reserve(elements.size());
    for(std::vector<mongo::BSONElement>::const_iterator it=elements.begin();
        it != elements.end(); ++it)
    {
        array.push_back(static_cast<TDCMTKType>(((*it).*getter)()));
    }
    condition = (element->*setter)(&array[0], array.size());

    if (condition.bad())
    {
        std::stringstream streamerror;
        streamerror << "Cannot set element '" << tag.toString().c_str()
                    << "': " << condition.text();
        throw ExceptionPACS(streamerror.str());
    }
}

void
BSONToDataSet
::_to_raw(mongo::BSONElement const & bson, DcmDataset & dataset,
          DcmTag const & tag) const
{
    int size=0;
    char const * begin = bson.binDataClean(size);
    OFCondition condition = dataset.putAndInsertUint8Array(tag,
                                                           reinterpret_cast<Uint8 const *>(begin),
                                                           size);
    if (condition.bad())
    {
        std::stringstream streamerror;
        streamerror << "Cannot set element '" << tag.toString().c_str()
                    << "': " << condition.text();
        throw ExceptionPACS(streamerror.str());
    }
}

void
BSONToDataSet
::_to_number_string(mongo::BSONElement const & bson, DcmDataset & dataset,
                    DcmTag const & tag) const
{
    std::ostringstream stream;
    stream.imbue(std::locale("C"));

    std::vector<mongo::BSONElement> elements = bson.Array();

    std::vector<mongo::BSONElement>::const_iterator const last_it = --elements.end();
    for(std::vector<mongo::BSONElement>::const_iterator it=elements.begin();
        it != elements.end(); ++it)
    {
        if(it->type() == mongo::NumberDouble)
        {
            double const number = it->Double();
            stream << number;
        }
        else if(it->type() == mongo::NumberInt)
        {
            int const number = it->Int();
            stream << number;
        }
        else if(it->type() == mongo::NumberLong)
        {
            long long const number = it->Long();
            stream << number;
        }

        if(it != last_it)
        {
            stream << "\\";
        }
    }
    
    OFString value(stream.str().c_str());

    if(value.size()%2!=0)
    {
        value += ' ';
    }
    OFCondition condition = dataset.putAndInsertOFStringArray(tag, value);
    if (condition.bad())
    {
        std::stringstream streamerror;
        streamerror << "Cannot set element '" << tag.toString().c_str()
                    << "': " << condition.text();
        throw ExceptionPACS(streamerror.str());
    }
}

} // namespace converterBSON

} // namespace dopamine
