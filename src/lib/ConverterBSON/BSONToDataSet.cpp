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

namespace dopamine
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

// TODO : AT

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
    this->_to_binary(bson, &mongo::BSONElement::Double, dataset, tag,
                     &DcmDataset::putAndInsertFloat64);
}

template<>
void
BSONToDataSet
::_to_dcmtk<EVR_FL>(mongo::BSONElement const & bson, DcmDataset & dataset,
                    DcmTag const & tag) const
{
    this->_to_binary(bson, &mongo::BSONElement::Double, dataset, tag,
                     &DcmDataset::putAndInsertFloat32);
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
    this->_to_text(bson, true, ' ', dataset, tag);
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
    this->_to_binary(bson, &mongo::BSONElement::Int, dataset, tag,
                     &DcmDataset::putAndInsertSint32);
}

// SQ is not processed here

template<>
void
BSONToDataSet
::_to_dcmtk<EVR_SS>(mongo::BSONElement const & bson, DcmDataset & dataset,
                    DcmTag const & tag) const
{
    this->_to_binary(bson, &mongo::BSONElement::Int,  dataset, tag,
                     &DcmDataset::putAndInsertSint16);
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
    this->_to_binary(bson, &mongo::BSONElement::Int, dataset, tag,
                     &DcmDataset::putAndInsertUint32);
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
    this->_to_binary(bson, &mongo::BSONElement::Int, dataset, tag,
                     &DcmDataset::putAndInsertUint16);
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
    std::vector<mongo::BSONElement> const array = bson.Array();

    // Get the VR : first item of value
    DcmVR const vr(array[0].String().c_str());
    DcmEVR const evr(vr.getEVR());

    if(!array[1].isNull())
    {
        if(evr == EVR_SQ)
        {
            if (array[1].isABSONObj())
            {
                BSONToDataSet converter;
                converter.set_specific_character_set(this->get_specific_character_set());

                DcmItem * item = new DcmItem(converter(array[1].Obj()));
                dataset.insertSequenceItem(tag, item);
            }
            else
            {
                std::vector<mongo::BSONElement> elements = array[1].Array();
                for(std::vector<mongo::BSONElement>::const_iterator it=elements.begin();
                    it != elements.end(); ++it)
                {
                    BSONToDataSet converter;
                    converter.set_specific_character_set(this->get_specific_character_set());

                    DcmDataset * item = new DcmDataset(converter(it->Obj()));
                    dataset.insertSequenceItem(tag, item);
                }
            }
        }
        else
        {
            if(evr == EVR_AE) this->_to_dcmtk<EVR_AE>(array[1], dataset, tag);
            else if(evr == EVR_AS) this->_to_dcmtk<EVR_AS>(array[1], dataset, tag);
            // else if(evr == EVR_AT) this->_to_dcmtk<EVR_AT>(array[1], dataset, tag);
            else if(evr == EVR_CS) this->_to_dcmtk<EVR_CS>(array[1], dataset, tag);
            else if(evr == EVR_DA) this->_to_dcmtk<EVR_DA>(array[1], dataset, tag);
            else if(evr == EVR_DS) this->_to_dcmtk<EVR_DS>(array[1], dataset, tag);
            else if(evr == EVR_DT) this->_to_dcmtk<EVR_DT>(array[1], dataset, tag);
            else if(evr == EVR_FD) this->_to_dcmtk<EVR_FD>(array[1], dataset, tag);
            else if(evr == EVR_FL) this->_to_dcmtk<EVR_FL>(array[1], dataset, tag);
            else if(evr == EVR_IS) this->_to_dcmtk<EVR_IS>(array[1], dataset, tag);
            else if(evr == EVR_LO) this->_to_dcmtk<EVR_LO>(array[1], dataset, tag);
            else if(evr == EVR_LT) this->_to_dcmtk<EVR_LT>(array[1], dataset, tag);
            else if(evr == EVR_OB) this->_to_dcmtk<EVR_OB>(array[1], dataset, tag);
            else if(evr == EVR_OF) this->_to_dcmtk<EVR_OF>(array[1], dataset, tag);
            else if(evr == EVR_OW) this->_to_dcmtk<EVR_OW>(array[1], dataset, tag);
            else if(evr == EVR_PN) this->_to_dcmtk<EVR_PN>(array[1], dataset, tag);
            else if(evr == EVR_SH) this->_to_dcmtk<EVR_SH>(array[1], dataset, tag);
            // // SQ is not processed here
            else if(evr == EVR_SL) this->_to_dcmtk<EVR_SL>(array[1], dataset, tag);
            else if(evr == EVR_SS) this->_to_dcmtk<EVR_SS>(array[1], dataset, tag);
            else if(evr == EVR_ST) this->_to_dcmtk<EVR_ST>(array[1], dataset, tag);
            else if(evr == EVR_TM) this->_to_dcmtk<EVR_TM>(array[1], dataset, tag);
            else if(evr == EVR_UI) this->_to_dcmtk<EVR_UI>(array[1], dataset, tag);
            else if(evr == EVR_UL) this->_to_dcmtk<EVR_UL>(array[1], dataset, tag);
            else if(evr == EVR_UN) this->_to_dcmtk<EVR_UN>(array[1], dataset, tag);
            else if(evr == EVR_US) this->_to_dcmtk<EVR_US>(array[1], dataset, tag);
            else if(evr == EVR_UT) this->_to_dcmtk<EVR_UT>(array[1], dataset, tag);
            
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
    std::vector<mongo::BSONElement> elements;

    if(bson.isABSONObj())
    {
        elements = bson.Array();
    }
    else
    {
        elements.push_back(bson);
    }

    OFString value;

    std::vector<mongo::BSONElement>::const_iterator const last_it = --elements.end();
    for(std::vector<mongo::BSONElement>::const_iterator it=elements.begin();
        it != elements.end(); ++it)
    {
        std::string const element = it->String();
        
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
    
    dataset.putAndInsertOFStringArray(tag, value);
}

template<typename TInserter, typename TBSONGetter>
void
BSONToDataSet
::_to_binary(mongo::BSONElement const & bson, TBSONGetter getter,
             DcmDataset & dataset, DcmTag const & tag, TInserter inserter) const
{
    if(bson.isABSONObj())
    {
        std::vector<mongo::BSONElement> const elements = bson.Array();
        unsigned long index=0;
        for(std::vector<mongo::BSONElement>::const_iterator it=elements.begin();
            it != elements.end(); ++it, ++index)
        {
            (dataset.*inserter)(tag, ((*it).*getter)(), index, OFTrue);
        }
    }
    else
    {
        (dataset.*inserter)(tag, (bson.*getter)(), 0, OFTrue);
    }
}

void
BSONToDataSet
::_to_raw(mongo::BSONElement const & bson, DcmDataset & dataset,
          DcmTag const & tag) const
{
    int size=0;
    char const * begin = bson.binDataClean(size);
    dataset.putAndInsertUint8Array(tag, reinterpret_cast<Uint8 const *>(begin), size);
}

void
BSONToDataSet
::_to_number_string(mongo::BSONElement const & bson, DcmDataset & dataset,
                    DcmTag const & tag) const
{
    std::ostringstream stream;
    stream.imbue(std::locale("C"));

    std::vector<mongo::BSONElement> elements;

    if(bson.isABSONObj())
    {
        elements = bson.Array();
    }
    else
    {
        elements.push_back(bson);
    }

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
    dataset.putAndInsertOFStringArray(tag, value);
}

} // namespace dopamine
