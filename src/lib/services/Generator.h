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

/* make sure OS specific configuration is included first */
#include <dcmtk/config/osconfig.h>
#include <dcmtk/ofstd/oftypes.h>
#include <dcmtk/dcmdata/dcdatset.h>

#include <mongo/client/dbclient.h>

namespace dopamine
{

namespace services
{

/**
 * @brief \class The Generator class
 * Base class for all response generator
 */
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

    /**
     * @brief Create an instance of Generator
     * @param username
     */
    Generator(std::string const & username);

    /// Destroy the instance of Generator
    virtual ~Generator();

    /**
     * Cancel response generation
     * NOT IMPLEMENTED YET
     */
    void cancel();

    /**
     * @brief Execute operation on given Dataset
     * @param dataset: Dataset to process
     * @param storageflags: indicate if operation is Store operation
     * @return Status of the operation
     */
    Uint16 process_dataset(DcmDataset* dataset, bool storageflags);

    /**
     * @brief Execute operation on given BSON Object
     * @param query: Query to process
     * @return Status of the operation
     */
    Uint16 process_bson(mongo::BSONObj const & query);

    /**
     * @brief Execute operation
     * @return Status of the operation
     */
    virtual Uint16 process() = 0;

    /**
     * @brief Get the next object
     * @return next object if exists, empty object otherwise
     */
    mongo::BSONObj next();

    /**
     * @brief Accessor for the attribute _allow
     * @return attribute _allow
     */
    bool is_allow() const;

    /**
     * @brief Accessor for the attribute _dataset
     * @return attribute _dataset
     */
    DcmDataset * get_dataset() const;

    /**
     * @brief Accessor for the attribute _bsonquery
     * @return attribute _bsonquery
     */
    mongo::BSONObj get_bsonquery() const;

protected:
    /// Connection to the Database
    mongo::DBClientConnection _connection;

    /// Database name
    std::string _db_name;

    /// Flag indicating if connection is established
    bool _isconnected;

    /// User name
    std::string _username;

    /// Cursor to the database
    mongo::unique_ptr<mongo::DBClientCursor> _cursor;

    /// Indicate if user is allowed to process operation
    bool _allow;

    /// Processed dataset
    DcmDataset * _dataset;

    /// Processed BSON Object
    mongo::BSONObj _bsonquery;

    /// @brief Return the DICOM Match Type of an element in BSON form.
    Match::Type _get_match_type(std::string const & vr,
                                mongo::BSONElement const & element) const;


    /// @brief Type of DICOM query -> MongoDB query conversion functions.
    typedef void (Self::*DicomQueryToMongoQuery)(
                        std::string const & field,
                        std::string const & vr,
                        mongo::BSONElement const & value,
                        mongo::BSONObjBuilder & builder) const;

    /**
     * @brief Return the DICOM query -> MongoDB query conversion function
     *        corresponding to the specified match type.
     */
    DicomQueryToMongoQuery _get_query_conversion(
                        Match::Type const & match_type) const;

private:
    /**
     * @brief _add_value_to_builder
     * @param builder
     * @param field
     * @param value
     */
    template<typename TType>
    void _add_value_to_builder(mongo::BSONObjBuilder &builder,
                               std::string const & field,
                               std::string const & value) const;

    /**
     * @brief Convert a BSON element from the DICOM query language to the
     *        MongoDB query language.
     *
     * This function must be specialized for each value of Self::Match.
     */
    template<Match::Type VType>
    void _dicom_query_to_mongo_query(std::string const & field,
                                     std::string const & vr,
                                     mongo::BSONElement const & value,
                                     mongo::BSONObjBuilder & builder) const;

};

} // namespace services

} // namespace dopamine

#endif // _7cbe5cf8_10a1_4e5c_ac53_44e38a2eeebf
