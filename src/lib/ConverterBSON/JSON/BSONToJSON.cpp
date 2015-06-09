/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "BSONToJSON.h"
#include "core/ConverterBase64.h"

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
::to_json(mongo::BSONObj const & bson)
{
    mongo::BSONObjBuilder builder;
    for(mongo::BSONObj::iterator it = bson.begin(); it.more();)
    {
        mongo::BSONElement const element_bson = it.next();

        if (element_bson.type() == mongo::Object)
        {
            mongo::BSONObj object = this->to_json(element_bson.Obj());
            builder << element_bson.fieldName() << object;
        }
        else if (element_bson.type() == mongo::Array)
        {
            mongo::BSONArrayBuilder arraybuilder;
            for (auto element : element_bson.Array())
            {
                mongo::BSONObj const tempobject = this->to_json(BSON("data" << element));
                arraybuilder << tempobject.getField("data");
            }
            builder << element_bson.fieldName() << arraybuilder.arr();
        }
        else if (element_bson.type() == mongo::BinData)
        {
            // Transform Binary field into Base64String

            int size = 0;
            char const * begin = element_bson.binDataClean(size);

            std::string encode = ConverterBase64::encode(std::string(begin, size));
            builder << element_bson.fieldName() << encode;
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
::to_string(mongo::BSONObj const & bson)
{
    // Convert BSON to JSON
    mongo::BSONObj const objectjson = this->to_json(bson);
    // Write json
    return objectjson.jsonString();
}

} // namespace converterBSON

} // namespace dopamine
