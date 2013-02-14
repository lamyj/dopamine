#include "Query.h"

// assert is required by gdcmAttribute ...
#include <assert.h>
#include <set>
#include <string>
#include <vector>

#include <gdcmAttribute.h>
#include <gdcmDataSet.h>
#include <mongo/bson/bson.h>
#include <mongo/client/dbclient.h>

#include "BSONToDataSet.h"
#include "DataSetToBSON.h"

Query
::Query(mongo::DBClientConnection & connection, std::string const & db_name,
        gdcm::DataSet const & dataset)
: _connection(connection), _db_name(db_name), _dataset(dataset)
{
    // Nothing else.
}

void
Query
::operator()()
{
    // Convert the GDCM dataset to BSON, excluding Query/Retrieve Level.
    DataSetToBSON dataset_to_bson;
    dataset_to_bson.set_filter(DataSetToBSON::Filter::EXCLUDE);
    dataset_to_bson.add_filtered_tag(0x00080052);
    mongo::BSONObjBuilder query_builder;
    dataset_to_bson(this->_dataset, query_builder);
    mongo::BSONObj const query_dataset = query_builder.obj();

    // Build the MongoDB query and query fields from the query dataset.
    mongo::BSONObjBuilder db_query;
    mongo::BSONObjBuilder fields_builder;
    for(mongo::BSONObj::iterator it=query_dataset.begin(); it.more();)
    {
        mongo::BSONElement const element = it.next();
        std::vector<mongo::BSONElement> const array = element.Array();

        // Always include the field in the results
        fields_builder << element.fieldName() << 1;

        // TODO : transform DICOM query language to MongoDB
        // Add the field to the query only if non-null
        if(!array[1].isNull())
        {
            db_query << element.fieldName() << array[1];
        }
    }

    // Always include Specific Character Set in results.
    if(!fields_builder.hasField("00080005"))
    {
        fields_builder << "00080005" << 1;
    }

    // Always include the keys for the query level.
    gdcm::Attribute<0x0008,0x0052> level_element;
    level_element.SetFromDataElement(this->_dataset.GetDataElement(0x00080052));
    std::string level(level_element.GetValue());
    if(level=="PATIENT " && !fields_builder.hasField("00100020"))
    {
        fields_builder << "00100020" << 1;
    }

    // Exclude the id from the query results.
    fields_builder << "_id" << 0;

    // Perform the DB query.
    mongo::BSONObj const fields = fields_builder.obj();
    mongo::auto_ptr<mongo::DBClientCursor> cursor =
    this->_connection.query(this->_db_name+".datasets",
                            db_query.obj(), 0, 0, &fields);

    // Get the distinct results: since _id is not in the results, just store
    // the results in a set, thus avoiding duplicates.
    std::set<mongo::BSONObj> distinct_items;
    while(cursor->more())
    {
        mongo::BSONObj const item = cursor->next();
        distinct_items.insert(item.copy());
    }

    // Convert the distinct BSON objects to DataSets.
    this->_results.clear();
    this->_results.reserve(distinct_items.size());
    BSONToDataSet bson_to_dataset;
    for(std::set<mongo::BSONObj>::const_iterator it=distinct_items.begin();
        it!=distinct_items.end(); ++it)
    {
        gdcm::DataSet dataset = bson_to_dataset(*it);
        dataset.Insert(level_element.GetAsDataElement());
        this->_results.push_back(dataset);
    }
}

std::vector<gdcm::DataSet> const &
Query
::getResults() const
{
    return this->_results;
}
