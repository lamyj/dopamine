#ifndef _ee9915a2_a504_4b21_8d43_7938c66c526e
#define _ee9915a2_a504_4b21_8d43_7938c66c526e

#include <string>
#include <vector>

#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dcdatset.h>
#include <dcmtk/dcmnet/dicom.h>
#include <dcmtk/ofstd/ofcond.h>

#include <mongo/client/dbclient.h>

class FindResponseGenerator
{
public :
    typedef FindResponseGenerator Self;

    FindResponseGenerator(DcmDataset /*const*/ & query, // DcmDataset is not const-correct
                          mongo::DBClientConnection & connection,
                          std::string const & db_name);
    DIC_US status() const;
    OFCondition next(DcmDataset ** responseIdentifiers);
    void cancel();

private :
    /// @brief DICOM match type, see PS 3.4-2011, C.2.2.2
    struct Match
    {
        enum Type
        {
            SingleValue,
            ListOfUID,
            Universal,
            WildCard,
            Range,
            Sequence,
            MultipleValues,
            Unknown
        };
    };
    
    DIC_US _status;
    std::string _query_retrieve_level;
    mongo::BSONObj _info;
    std::vector<mongo::BSONElement> _results;
    unsigned long _index;
    DcmTagKey _instance_count_tag;
    bool _convert_modalities_in_study;
    
    /// @brief Return the DICOM Match Type of an element in BSON form.
    Match::Type _get_match_type(std::string const & vr, 
                                mongo::BSONElement const & element) const;

    /**
     * @brief Convert a BSON element from the DICOM query language to the
     *        MongoDB query language.
     *
     * This function must be specialized for each value of Self::Match.
     */
    template<Match::Type VType>
    void _dicom_query_to_mongo_query(
        std::string const & field, std::string const & vr,
        mongo::BSONElement const & value,
        mongo::BSONObjBuilder & builder) const;

    /// @brief Type of DICOM query -> MongoDB query conversion functions.
    typedef void (Self::*DicomQueryToMongoQuery)(
        std::string const & field, std::string const & vr,
        mongo::BSONElement const & value,
        mongo::BSONObjBuilder & builder) const;

    /**
     * @brief Return the DICOM query -> MongoDB query conversion function
     *        corresponding to the specified match type.
     */
    DicomQueryToMongoQuery _get_query_conversion(Match::Type const & match_type) const;
};

#endif // _ee9915a2_a504_4b21_8d43_7938c66c526e
