#ifndef _737cc322_0e2e_4fbb_aac6_b7df5e4f2d09
#define _737cc322_0e2e_4fbb_aac6_b7df5e4f2d09

#include <map>
#include <set>
#include <string>

#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dctk.h>

#include <iconv.h>

// We should only include mongo/bson/bson.h, but this might cause compile-time
// errors, cf. https://jira.mongodb.org/browse/SERVER-1273
#include <mongo/db/jsobj.h>

/**
 * @brief Convert a DCMTK DataSet to a BSON object.
 */
class DataSetToBSON
{
public :

    // cf. https://httpd.apache.org/docs/2.2/en/mod/mod_authz_host.html#order
    struct Filter
    {
        enum Type
        {
            INCLUDE=0,
            EXCLUDE,
            MAX
        };
    };

    DataSetToBSON();
    ~DataSetToBSON();

    std::string get_specific_character_set() const;
    void set_specific_character_set(std::string const & specific_character_set);

    Filter::Type const & get_filter() const;
    void set_filter(Filter::Type const & order);

    void add_filtered_tag(DcmTag const & tag);
    void remove_filtered_tag(DcmTag const & tag);
    void clear_filtered_tags();
    bool is_tag_filtered(DcmTag const & tag) const;

    void operator()(DcmObject * dataset, mongo::BSONObjBuilder & builder);

private :
    static const std::map<std::string, std::string> _dicom_to_iconv;
    std::string _specific_character_set;
    iconv_t _converter;

    Filter::Type _filter;
    std::set<DcmTag> _filtered_tags;

    /// @brief Generate a map from DICOM encoding to IConv encoding
    static std::map<std::string, std::string> _create_encoding_map();

    /// @brief Convert binary data from a DICOM element to BSON.
    template<DcmEVR VVR>
    void _to_bson(DcmObject * element, mongo::BSONArrayBuilder & builder) const;

    /**
     * @brief Convert binary data from a text DICOM element.
     *
     * This is used for AE, AS, CS, DA, DT, LO, LT, PN, SH, ST, TM, UI, UT
     */
    void _to_bson_text(DcmByteString * element, mongo::BSONArrayBuilder & builder,
                       bool use_utf8) const;

    /**
     * @brief Convert binary data from a DICOM element to BSON binary data.
     *
     * This is used for OB, OF, OW, UN
     */
    void _to_bson_binary(DcmElement * element, mongo::BSONArrayBuilder & builder) const;

    /**
     * @brief Convert binary data from a DICOM element to a BSON number.
     *
     * This is used for FD, FL, SL, SS, UL, US
     */
    template<typename TValue>
    void _to_bson_number(DcmElement * element, 
        OFCondition (DcmElement::*getter)(TValue &, unsigned long),
        mongo::BSONArrayBuilder & builder) const;

    // Since _to_bson is specialized and instantiated in _add_element,
    // this function must be declared after the the specializations.
    void _add_element(DcmObject * element, mongo::BSONObjBuilder & builder) const;

};

#endif // _737cc322_0e2e_4fbb_aac6_b7df5e4f2d09
