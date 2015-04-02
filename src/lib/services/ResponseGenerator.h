/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _7cbe5cf8_10a1_4e5c_ac53_44e38a2eeebf
#define _7cbe5cf8_10a1_4e5c_ac53_44e38a2eeebf

#include <string>

#include <dcmtk/config/osconfig.h> /* make sure OS specific configuration is included first */
#include <dcmtk/dcmdata/dcdatset.h>
#include <dcmtk/dcmnet/assoc.h>
#include <dcmtk/dcmnet/dimse.h>

#include <mongo/client/dbclient.h>

namespace dopamine
{

namespace services
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

    ResponseGenerator(T_ASC_Association * request_association,
                      std::string const & service_name);

    virtual ~ResponseGenerator();

    /**
     * Cancel response generation
     * NOT IMPLEMENTED YET
     */
    virtual void cancel();

protected:

    T_ASC_Association * _request_association;

    std::string _service_name;

    /// Local AETitle
    std::string _ourAETitle;

    mongo::DBClientConnection _connection;

    std::string _db_name;

    DIC_US _status;

    std::string _query_retrieve_level;

    DcmTagKey _instance_count_tag;

    /// flag indicating if modalities should be convert
    bool _convert_modalities_in_study;

    virtual Uint16 set_query(DcmDataset * dataset);

    mongo::unique_ptr<mongo::DBClientCursor> _cursor;

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

} // namespace services

} // namespace dopamine

#endif // _7cbe5cf8_10a1_4e5c_ac53_44e38a2eeebf
