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
#include <dcmtk/ofstd/oftypes.h>
#include <dcmtk/dcmdata/dcdatset.h>

#include <mongo/client/dbclient.h>

namespace dopamine
{

namespace services
{

class Generator
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

    typedef Generator Self;

    Generator(std::string const & username);

    virtual ~Generator();

    /**
     * Cancel response generation
     * NOT IMPLEMENTED YET
     */
    void cancel();

    Uint16 process_dataset(DcmDataset* dataset, bool storageflags);

    Uint16 process_bson(mongo::BSONObj const & query);

    virtual Uint16 process() = 0;

    mongo::BSONObj next();

    bool is_allow() const;

    DcmDataset * get_dataset() const;

    mongo::BSONObj get_bsonquery() const;

protected:

    mongo::DBClientConnection _connection;

    std::string _db_name;

    bool _isconnected;

    std::string _username;

    mongo::unique_ptr<mongo::DBClientCursor> _cursor;

    bool _allow;

    DcmDataset * _dataset;

    mongo:: BSONObj _bsonquery;

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

    template<typename TType>
    void add_value_to_builder(mongo::BSONObjBuilder &builder,
                              const std::string &field,
                              const std::string &value) const;

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
