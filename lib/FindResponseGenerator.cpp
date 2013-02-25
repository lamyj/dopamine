#include "FindResponseGenerator.h"

#include <fstream>
#include <set>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <vector>

#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dcdatset.h>
#include <dcmtk/dcmnet/dicom.h>
#include <dcmtk/dcmnet/dimse.h>
#include <dcmtk/ofstd/ofcond.h>

#include <mongo/bson/bson.h>
#include <mongo/client/dbclient.h>

#include "BSONToDataSet.h"
#include "DataSetToBSON.h"

FindResponseGenerator
::FindResponseGenerator(DcmDataset /*const*/ & query, // DcmDataset is not const-correct
                        mongo::DBClientConnection & connection,
                        std::string const & db_name)
{
    // Convert the dataset to BSON, excluding Query/Retrieve Level.
    DataSetToBSON dataset_to_bson;
    dataset_to_bson.set_filter(DataSetToBSON::Filter::EXCLUDE);
    dataset_to_bson.add_filtered_tag(DcmTag(0x0008, 0x0052));
    mongo::BSONObjBuilder query_builder;
    dataset_to_bson(&query, query_builder);
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
    OFString ofstring;
    query.findAndGetOFString(DCM_QueryRetrieveLevel, ofstring);
    this->_query_retrieve_level = std::string(ofstring.c_str());
    if(this->_query_retrieve_level=="PATIENT " && !fields_builder.hasField("00100020"))
    {
        fields_builder << "00100020" << 1;
    }

    // Exclude the id from the query results.
    fields_builder << "_id" << 0;

    // Perform the DB query.
    mongo::BSONObj const fields = fields_builder.obj();
    this->_cursor = connection.query(
        db_name+".datasets", db_query.obj(), 0, 0, &fields);

    this->_status = STATUS_Pending;
}

DIC_US
FindResponseGenerator
::status() const
{
    return this->_status;
}

OFCondition
FindResponseGenerator
::next(DcmDataset ** responseIdentifiers)
{
    OFCondition cond;

    bool next_item_found=false;
    mongo::BSONObj item;
    while(this->_cursor->more())
    {
        item = this->_cursor->next().copy();
        if(this->_distinct_items.find(item) == this->_distinct_items.end())
        {
            next_item_found = true;
            break;
        }
    }

    if(!next_item_found)
    {
        // We're done.
        this->_status = STATUS_Success;
    }
    else
    {
        BSONToDataSet bson_to_dataset;
        DcmDataset dataset = bson_to_dataset(item);
        dataset.putAndInsertOFStringArray(DCM_QueryRetrieveLevel,
                                          this->_query_retrieve_level.c_str());

        (*responseIdentifiers) = new DcmDataset(dataset);

        this->_status = STATUS_Pending;

        this->_distinct_items.insert(item.copy());
    }

    return cond;
}

void
FindResponseGenerator
::cancel()
{
    std::cout << "TODO : FindResponseGenerator::cancel()" << std::endl;
}
