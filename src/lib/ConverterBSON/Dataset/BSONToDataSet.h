/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _a97264d3_b54b_494b_93a8_1a595dd06f8a
#define _a97264d3_b54b_494b_93a8_1a595dd06f8a

#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dctk.h>

#include <mongo/bson/bson.h>
#include <mongo/db/json.h>

#include "ConverterBSONDataSet.h"

namespace dopamine
{

/**
 * @brief Convert a BSON object to a DCMTK DataSet.
 */
class BSONToDataSet : public ConverterBSONDataSet
{
public :
    /// Create an instance of BSONToDataSet
    BSONToDataSet();
    
    /// Destroy the instance of BSONDataSet
    ~BSONToDataSet();

    /**
     * Operator ()
     * @param bson: BSON object to convert
     * @return converted DICOM dataset
     */
    DcmDataset operator()(mongo::BSONObj const & bson);
    
protected:
    
private :
    template<typename TBSONType>
    struct BSONGetterType
    {
        typedef TBSONType (mongo::BSONElement::*Type)() const;
    };

    template<typename TDCMTKType>
    struct DCMTKSetterType
    {
        typedef OFCondition (DcmElement::*Type)(TDCMTKType const *, const unsigned long);
    };

    void _add_element(mongo::BSONElement const & bson,
                      DcmDataset & dataset);

    template<DcmEVR VVR>
    void _to_dcmtk(mongo::BSONElement const & bson, 
                   DcmDataset & dataset, DcmTag const & tag) const;

    void _to_text(mongo::BSONElement const & bson, bool use_utf8, char padding,
                  DcmDataset & dataset, DcmTag const & tag) const;

    template<typename TDCMTKType, typename TBSONType>
    void _to_binary(mongo::BSONElement const & bson,
                    typename BSONGetterType<TBSONType>::Type getter,
                    DcmDataset & dataset, DcmTag const & tag,
                    typename DCMTKSetterType<TDCMTKType>::Type setter) const;

    void _to_raw(mongo::BSONElement const & bson, DcmDataset & dataset,
                 DcmTag const & tag) const;

    void _to_number_string(mongo::BSONElement const & bson, 
                           DcmDataset & dataset, DcmTag const & tag) const;
};

} // namespace dopamine

#endif // _a97264d3_b54b_494b_93a8_1a595dd06f8a
