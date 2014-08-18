/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _51950e73_7a21_4362_ba53_4aaf6257e0a6
#define _51950e73_7a21_4362_ba53_4aaf6257e0a6

#include <vector>

#include <dcmtk/config/osconfig.h>  /* make sure OS specific configuration is included first */
#include <dcmtk/dcmdata/dctk.h>     /* Covers most common dcmdata classes */

#include <mongo/client/dbclient.h>

#include "SCP.h"

namespace research_pacs
{
    
class ResponseGenerator
{
public:
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
    
    typedef ResponseGenerator Self;

    ResponseGenerator(SCP * scp, std::string const & ouraetitle);
    
    virtual ~ResponseGenerator();
    
    static std::string replace(std::string const & value, 
                               std::string const & old, 
                               std::string const & new_);

protected:
    SCP * _scp;
    
    std::string _ourAETitle;
    
    DIC_US _status;

    std::string _query_retrieve_level;
    
    /// Store results
    std::vector<mongo::BSONElement> _results;
    
    /// current index for _results vector
    unsigned long _index;
    
    mongo::BSONObj _info;
    
    DcmTagKey _instance_count_tag;
    
    /// Priority of request
    T_DIMSE_Priority _priority;
    
    virtual void cancel();
    
    virtual void next(DcmDataset ** responseIdentifiers) = 0;
    
    /// @brief Return the DICOM Match Type of an element in BSON form.
    Match::Type _get_match_type(std::string const & vr, 
                                mongo::BSONElement const & element) const;
                                
    
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
    
private:
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
    
};
    
} // namespace research_pacs

#endif // _51950e73_7a21_4362_ba53_4aaf6257e0a6
