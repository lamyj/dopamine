#ifndef _a97264d3_b54b_494b_93a8_1a595dd06f8a
#define _a97264d3_b54b_494b_93a8_1a595dd06f8a

#include <vector>
#include <stdint.h>

#include <gdcmDataSet.h>

#include <iconv.h>

#include <mongo/bson/bson.h>

/**
 * Convert a BSON object to a GDCM DataSet.
 */
class BSONToDataSet
{
public :

    BSONToDataSet();
    ~BSONToDataSet();

    std::string get_specific_character_set() const;
    void set_specific_character_set(std::string const & specific_character_set);

    gdcm::DataSet operator()(mongo::BSONObj const & bson);
private :
    std::string _specific_character_set;
    iconv_t _converter;

    void _add_element(mongo::BSONElement const & bson,
                      gdcm::DataSet & data_set);

    template<gdcm::VR::VRType VVR>
    std::vector<uint8_t> _to_gdcm(mongo::BSONElement const & bson) const;

    std::vector<uint8_t> _to_gdcm_text(mongo::BSONElement const & bson,
                                       bool use_utf8, char padding, bool add_padding=true) const;
    template<typename T>
    std::vector<uint8_t> _to_gdcm_binary(mongo::BSONElement const & bson) const;

    std::vector<uint8_t> _to_gdcm_raw(mongo::BSONElement const & bson) const;

    std::vector<uint8_t> _to_gdcm_number_string(mongo::BSONElement const & bson, bool add_padding=true) const;
};

#endif // _a97264d3_b54b_494b_93a8_1a595dd06f8a
