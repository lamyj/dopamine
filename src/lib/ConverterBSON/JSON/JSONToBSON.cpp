/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <mongo/bson/bson.h>
#include <mongo/db/json.h>

#include "core/ConverterBase64.h"
#include "JSONToBSON.h"

namespace dopamine
{

namespace converterBSON
{

JSONToBSON
::JSONToBSON():
    ConverterBSONJSON()
{
    // Nothing to do
}

JSONToBSON
::~JSONToBSON()
{
    // Nothing to do
}

mongo::BSONObj
JSONToBSON
::from_JSON(const mongo::BSONObj &json)
{
    mongo::BSONObjBuilder builder;
    for(mongo::BSONObj::iterator it = json.begin(); it.more();)
    {
        mongo::BSONElement const element_bson = it.next();

        if (element_bson.type() == mongo::Object)
        {
            mongo::BSONObj object = this->from_JSON(element_bson.Obj());
            builder << element_bson.fieldName() << object;
        }
        else if (element_bson.type() == mongo::Array)
        {
            mongo::BSONArrayBuilder arraybuilder;
            for (auto element : element_bson.Array())
            {
                mongo::BSONObj const tempobject = this->from_JSON(BSON("data" << element));
                arraybuilder << tempobject.getField("data");
            }
            builder << element_bson.fieldName() << arraybuilder.arr();
        }
        else if (element_bson.type() == mongo::String &&
                 std::string(element_bson.fieldName()) == std::string("InlineBinary"))
        {
            // Transform Base64String field into Binary

            std::string const data = element_bson.String();
            std::string dec = ConverterBase64::decode(data);

            builder.appendBinData(element_bson.fieldName(), dec.size(),
                                  mongo::BinDataGeneral, dec.c_str());
        }
        else
        {
            // Nothing to do
            builder.append(element_bson);
        }
    }

    return builder.obj();
}

mongo::BSONObj
JSONToBSON
::from_string(const std::string &json)
{
    // Read string
    mongo::BSONObj objectjson = mongo::fromjson(json);
    // Convert JSON to BSON
    return this->from_JSON(objectjson);
}

} // namespace converterBSON

} // namespace dopamine
