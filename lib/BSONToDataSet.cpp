#include "BSONToDataSet.h"

#include <locale>
#include <sstream>
#include <stdexcept>

#include <errno.h>
#include <iconv.h>

#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dctk.h>

BSONToDataSet
::BSONToDataSet()
: _specific_character_set(""), _converter(0)
{
    this->set_specific_character_set("");
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

DcmDataset
BSONToDataSet
::operator()(mongo::BSONObj const & bson)
{
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
    DcmEVR const evr(vr.getValidEVR());

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

        if(evr == EVR_SQ)
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

            size_t const result = iconv(this->_converter,
                &inbuf, &inbytesleft, &outbuf, &outbytesleft);
            if(result == size_t(-1))
            {
                throw std::runtime_error(std::string("iconv error ")+strerror(errno));
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
