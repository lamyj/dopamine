#ifndef _52e6be88_3059_4d03_9bc5_c20eb0cdf175
#define _52e6be88_3059_4d03_9bc5_c20eb0cdf175

#include <string>
#include <vector>
#include <gdcmDataSet.h>
#include <mongo/client/dbclient.h>

class Query
{
public :
    Query(mongo::DBClientConnection & connection, std::string const & db_name,
          gdcm::DataSet const & dataset);
    void operator()();
    std::vector<gdcm::DataSet> const & getResults() const;
private :
    mongo::DBClientConnection & _connection;
    std::string _db_name;
    gdcm::DataSet _dataset;
    std::vector<gdcm::DataSet> _results;
};

#endif // _52e6be88_3059_4d03_9bc5_c20eb0cdf175
