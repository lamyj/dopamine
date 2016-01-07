/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _d11a0665_fab9_4ad0_8287_043e7617d0f1
#define _d11a0665_fab9_4ad0_8287_043e7617d0f1

#include <mongo/client/dbclient.h>

#include "dbconnection/MongoDBConnection.h"
#include "services/Generator.h"

namespace dopamine
{

namespace services
{

/**
 * @brief \class The Generator class
 * Base class for all response generator
 */
class GeneratorPACS : public Generator
{
public:
    typedef GeneratorPACS Self;
    typedef boost::shared_ptr<Self> Pointer;

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

    /// @brief Default Constructor for GeneratorPACS
    GeneratorPACS();

    /// Destroy the instance of GeneratorPACS
    virtual ~GeneratorPACS();

    virtual dcmtkpp::Value::Integer initialize(
            dcmtkpp::DcmtkAssociation const & association,
            dcmtkpp::message::Message const & message);

    virtual bool done() const;

    virtual dcmtkpp::Value::Integer initialize(mongo::BSONObj const & request);

    virtual std::pair<std::string, int> get_peer_information(std::string const & ae_title);

    void set_username(std::string const & name);

    std::string get_username() const;

    bool is_connected() const;

    void set_query_retrieve_level(std::string const & query_retrieve_level);

    std::string get_query_retrieve_level() const;

    std::vector<std::string> get_instance_count_tags() const;

    void set_include_fields(std::vector<std::string> const & include_fields);

    std::vector<std::string>& get_include_fields();

    void set_maximum_results(int maximum_results);

    int get_maximum_results() const;

    void set_skipped_results(int skipped_results);

    int get_skipped_results() const;

    dcmtkpp::Element compute_attribute(dcmtkpp::Tag const & tag,
                                       dcmtkpp::VR const & vr,
                                       std::string const & value);

    /**
     * Replace all given pattern by another
     * @param value: given input string
     * @param old: search pattern to be replaced
     * @param new_: new pattern
     * @return string
     */
    static std::string replace(std::string const & value,
                               std::string const & old,
                               std::string const & new_);

protected:
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

    bool extract_query_retrieve_level(mongo::BSONObj const & mongo_object);

    /// Connection to the Database
    MongoDBConnection * _connection;

    /// Flag indicating if connection is established
    bool _isconnected;

    /// User name
    std::string _username;

    /// Cursor to the database
    mongo::unique_ptr<mongo::DBClientCursor> _cursor;

    /// QueryRetrieveLevel
    std::string _query_retrieve_level;

    ///
    std::vector<std::string> _instance_count_tags;

    /// Fields to retrieve
    std::vector<std::string> _include_fields;

    /// Maximum number of dataset to retrieve
    int _maximum_results;

    /// Number of response to ignore
    int _skipped_results;

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

    dcmtkpp::Value::Integer _get_count(std::string const & relatedElement,
                                       std::string const & ofElement,
                                       std::string const & value);

};

} // namespace services

} // namespace dopamine

#endif // _d11a0665_fab9_4ad0_8287_043e7617d0f1
