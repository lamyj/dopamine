/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _737cc322_0e2e_4fbb_aac6_b7df5e4f2d09
#define _737cc322_0e2e_4fbb_aac6_b7df5e4f2d09

#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dctk.h>

// We should only include mongo/bson/bson.h, but this might cause compile-time
// errors, cf. https://jira.mongodb.org/browse/SERVER-1273
#include <mongo/db/jsobj.h>

#include "Condition.h"
#include "ConverterBSONDataSet.h"

namespace dopamine
{

namespace converterBSON
{

/**
 * @brief \class Convert a DCMTK DataSet to a BSON object.
 */
class DataSetToBSON : public ConverterBSONDataSet
{
public:

    struct FilterAction
    {
        enum Type
        {
            INCLUDE=0,
            EXCLUDE,
            UNKNOWN
        };
    };

    typedef std::pair<Condition::Pointer, FilterAction::Type> Filter;

    /// Create an instance of DataSetToBSON
    DataSetToBSON();

    /// Destroy the instance of DataSetToBSON
    ~DataSetToBSON();

    /// @brief Filter action applied if no filter matches, defaults to include.
    FilterAction::Type const & get_default_filter() const;
    void set_default_filter(FilterAction::Type const & action);

    /**
     * @brief Filters to specify which elements are converted.
     * Processing stops after the first matching condition.
     */
    std::vector<Filter> & get_filters();
    void set_filters(std::vector<Filter> const & filters);

    mongo::BSONObj from_dataset(DcmObject * dataset);

private:
    std::vector<Filter> _filters;
    FilterAction::Type _default_filter;

    /// @brief Convert binary data from a DICOM element to BSON.
    template<DcmEVR VVR>
    void _to_bson(DcmObject * element, mongo::BSONObjBuilder & builder) const;

    /**
     * @brief Convert binary data from a text DICOM element.
     *
     * This is used for AE, AS, CS, DA, DT, LO, LT, PN, SH, ST, TM, UI, UT
     */
    void _to_bson_text(DcmByteString * element,
                       mongo::BSONObjBuilder & builder, bool use_utf8) const;

    /**
     * @brief Convert binary data from a DICOM element to BSON binary data.
     *
     * This is used for OB, OF, OW, UN
     */
    void _to_bson_binary(DcmElement * element,
                         mongo::BSONObjBuilder & builder) const;

    /**
     * @brief Convert binary data from a DICOM element to a BSON number.
     *
     * This is used for FD, FL, SL, SS, UL, US
     */
    template<typename TDICOMValue, typename TBSONValue>
    void _to_bson_number(DcmElement * element,
        OFCondition (DcmElement::*getter)(TDICOMValue &, unsigned long),
        mongo::BSONObjBuilder & builder) const;

    void _to_bson_at(DcmAttributeTag * element,
                     mongo::BSONObjBuilder & builder) const;

    // Since _to_bson is specialized and instantiated in _add_element,
    // this function must be declared after the the specializations.
    void _add_element(DcmObject * element,
                      mongo::BSONObjBuilder & builder) const;

};

} // namespace converterBSON

} // namespace dopamine

#endif // _737cc322_0e2e_4fbb_aac6_b7df5e4f2d09
