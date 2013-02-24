#ifndef _a97264d3_b54b_494b_93a8_1a595dd06f8a
#define _a97264d3_b54b_494b_93a8_1a595dd06f8a

#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dctk.h>

#include <iconv.h>

#include <mongo/bson/bson.h>

/**
 * Convert a BSON object to a DCMTK DataSet.
 */
class BSONToDataSet
{
public :

    BSONToDataSet();
    ~BSONToDataSet();

    std::string get_specific_character_set() const;
    void set_specific_character_set(std::string const & specific_character_set);

    DcmDataset operator()(mongo::BSONObj const & bson);
private :
    std::string _specific_character_set;
    iconv_t _converter;

    void _add_element(mongo::BSONElement const & bson,
                      DcmDataset & dataset);

    template<DcmEVR VVR>
    void _to_dcmtk(mongo::BSONElement const & bson, 
        DcmDataset & dataset, DcmTag const & tag) const;

    void _to_text(mongo::BSONElement const & bson, bool use_utf8, char padding,
                  DcmDataset & dataset, DcmTag const & tag) const;

    template<typename TInserter, typename TBSONGetter>
    void _to_binary(mongo::BSONElement const & bson, TBSONGetter getter,
        DcmDataset & dataset, DcmTag const & tag, TInserter inserter) const;

    void _to_raw(mongo::BSONElement const & bson, DcmDataset & dataset,
                 DcmTag const & tag) const;

    void _to_number_string(mongo::BSONElement const & bson, 
                                DcmDataset & dataset, DcmTag const & tag) const;
};

#endif // _a97264d3_b54b_494b_93a8_1a595dd06f8a
