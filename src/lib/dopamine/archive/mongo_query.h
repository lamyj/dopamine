/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _96c07a87_ac86_43e3_b965_52ff040b25e6
#define _96c07a87_ac86_43e3_b965_52ff040b25e6

#include <functional>
#include <string>

#include <mongo/bson/bson.h>
#include <odil/DataSet.h>

namespace dopamine
{

namespace archive
{

/// @brief Convert DICOM query to a MongoDB query.
void as_mongo_query(
    odil::DataSet const & data_set,
    mongo::BSONArrayBuilder & query_terms,
    mongo::BSONObjBuilder & query_fields);

/// @brief DICOM match type, see PS 3.4, C.2.2.2
enum class MatchType
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

/**
 * @brief Convert a BSON element from the DICOM query language to the
 * MongoDB query language.
 *
 * This function must be specialized for each value of MatchType.
 */
template<MatchType VType>
void as_mongo_query(
    std::string const & field, std::string const & vr,
    mongo::BSONElement const & value, mongo::BSONObjBuilder & builder);

/// @brief Return the DICOM Match Type of an element in BSON form.
MatchType
get_match_type(std::string const & vr, mongo::BSONElement const & element);

/// @brief DICOM-to-Mongo query converter.
typedef std::function<void(
    std::string const &, std::string const &,
    mongo::BSONElement const &, mongo::BSONObjBuilder &)> QueryConverter;

/// @brief Return the query converter associated with the match type.
QueryConverter get_query_converter(MatchType match_type);

}

}

#endif // _96c07a87_ac86_43e3_b965_52ff040b25e6
