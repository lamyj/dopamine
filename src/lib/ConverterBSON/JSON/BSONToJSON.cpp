/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/ostream_iterator.hpp>

#include "BSONToJSON.h"

namespace dopamine
{

namespace converterBSON
{

BSONToJSON
::BSONToJSON():
    ConverterBSONJSON()
{
    // Nothing to do
}

BSONToJSON
::~BSONToJSON()
{
    // Nothing to do
}

mongo::BSONObj
BSONToJSON
::to_JSON(const mongo::BSONObj &bson)
{
    mongo::BSONObjBuilder builder;
    for(mongo::BSONObj::iterator it = bson.begin(); it.more();)
    {
        mongo::BSONElement const element_bson = it.next();

        if (element_bson.type() == mongo::Object)
        {
            mongo::BSONObj object = this->to_JSON(element_bson.Obj());
            builder << element_bson.fieldName() << object;
        }
        else if (element_bson.type() == mongo::Array)
        {
            mongo::BSONArrayBuilder arraybuilder;
            for (auto element : element_bson.Array())
            {
                mongo::BSONObj const tempobject = this->to_JSON(BSON("data" << element));
                arraybuilder << tempobject.getField("data");
            }
            builder << element_bson.fieldName() << arraybuilder.arr();
        }
        else if (element_bson.type() == mongo::BinData)
        {
            // Transform Binary field into Base64String

            int size=0;
            char const * begin = element_bson.binDataClean(size);

            typedef boost::archive::iterators::base64_from_binary<
                        boost::archive::iterators::transform_width<
                            const char *, 6, 8>
                        >
                    base64_t;

            std::stringstream os;
            std::copy(
                    base64_t(begin),
                    base64_t(begin + size),
                    boost::archive::iterators::ostream_iterator<char>(os)
                );

            builder << element_bson.fieldName() << os.str();
        }
        else
        {
            // Nothing to do
            builder.append(element_bson);
        }
    }

    return builder.obj();
}

std::string
BSONToJSON
::to_string(const mongo::BSONObj &bson)
{
    // Convert BSON to JSON
    mongo::BSONObj objectjson = this->to_JSON(bson);
    // Write json
    return objectjson.jsonString();
}

} // namespace converterBSON

} // namespace dopamine
