#include <cstdlib>
#include <iostream>
#include <sstream>

#include <boost/program_options.hpp>
#include <gdcmReader.h>
#include <gdcmSystem.h>

#include "database.h"

namespace po = boost::program_options;

int store(int argc, char** argv)
{
    std::string const syntax("Syntax : store filename\n");

    if(argc != 1)
    {
        std::cerr << syntax;
        return EXIT_FAILURE;
    }

    std::string const filename(argv[0]);

    if(!gdcm::System::FileExists(filename.c_str()))
    {
        std::cerr << "Store : file " << filename << " does not exist\n";
        return EXIT_FAILURE;
    }

    gdcm::Reader reader;
    reader.SetFileName(argv[0]);
    if(!reader.Read())
    {
        std::cerr << "Store : could not read " << filename << "\n";
        return EXIT_FAILURE;
    }

    Database database("research_pacs");
    database.insert_dataset(reader.GetFile().GetDataSet());

    return EXIT_SUCCESS;
}

int query(int argc, char** argv)
{
    std::string const syntax("Syntax : query field value\n");

    if(argc != 2)
    {
        std::cerr << syntax;
        return EXIT_FAILURE;
    }

    std::string const field(argv[0]);
    std::string const value(argv[1]);

    Database database("research_pacs");
    mongo::auto_ptr<mongo::DBClientCursor> cursor = database.query(
        QUERY(field<<value));
    while(cursor->more())
    {
        mongo::BSONObj const item = cursor->next();
        std::cout << item.getStringField("(0008|0018)") << "\n";
    }

    return EXIT_SUCCESS;
}

int retrieve(int argc, char** argv)
{
    std::string const syntax("Syntax : retrieve sop_instance_uid filename\n");

    if(argc != 2)
    {
        std::cerr << syntax;
        return EXIT_FAILURE;
    }

    std::string const sop_instance_uid(argv[0]);
    std::string const filename(argv[1]);

    Database database("research_pacs");
    mongo::auto_ptr<mongo::DBClientCursor> cursor = database.query(
        QUERY("(0008|0018)"<<sop_instance_uid));
    if(!cursor->more())
    {
        std::cerr << "Retrieve : no such SOP instance UID " << sop_instance_uid << "\n";
        return EXIT_FAILURE;
    }

    mongo::BSONObj const item = cursor->next();
    std::ofstream stream(filename.c_str());
    stream << item;
    stream.close();

    return EXIT_SUCCESS;
}

int main(int argc, char** argv)
{
    std::string const syntax("Syntax research_pacs ACTION ARGS\n"
        "where ACTION may be :\n"
        "  * store\n"
        "  * query\n"
        "  * retrieve\n");

    if(argc<2)
    {
        std::cerr << syntax;
        return EXIT_FAILURE;
    }

    std::string const action=argv[1];

    if(action=="store")
    {
        return store(argc-2, argv+2);
    }
    if(action=="query")
    {
        return query(argc-2, argv+2);
    }
    if(action=="retrieve")
    {
        return retrieve(argc-2, argv+2);
    }
    else
    {
        std::cerr << syntax;
        return EXIT_FAILURE;
    }
}
