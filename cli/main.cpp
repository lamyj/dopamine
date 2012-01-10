#include <cstdlib>
#include <iostream>
#include <sstream>

#include <boost/program_options.hpp>
#include <gdcmAttribute.h>
#include <gdcmReader.h>
#include <gdcmSystem.h>

#include "database.h"

namespace po = boost::program_options;

// TODO
// For each command :
//   * add [-b DATABASE], defaults to research_pacs
//   * add credentials
// For store :
//   * add clinical trial informations (--sponsor, --protocol, --subject)
// For query :
//   * display original MIME type and original file name if not DICOM

int store(int argc, char** argv)
{
    po::options_description description("Allowed options for \"store\"");
    description.add_options()
        ("help", "produce help message")
        ("sponsor", po::value<std::string>(), "Clinical Trial Sponsor Name")
        ("protocol", po::value<std::string>(), "Clinical Trial Protcol ID")
        ("subject", po::value<std::string>(), "Clinical Trial Subject ID")
        ("input-file", po::value<std::vector<std::string> >(), "input file");
    
    po::positional_options_description positional_options;
    positional_options.add("input-file", -1);
    
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).
              options(description).positional(positional_options).run(), vm);
    po::notify(vm);
    
    research_pacs::Database database("research_pacs");
    
    if(vm.count("sponsor") > 0)
    {
        std::string const sponsor = vm["sponsor"].as<std::string>();
        if(database.query_users(QUERY("id" << sponsor)).size() == 0)
        {
            database.insert_user(research_pacs::User(sponsor));
        }
    }
    if(vm.count("protocol") > 0)
    {
        if(vm.count("sponsor") == 0)
        {
            std::cerr << "Cannot specify protocol without sponsor\n";
            return EXIT_FAILURE;
        }
        std::string const sponsor = vm["sponsor"].as<std::string>();
        std::string const protocol = vm["protocol"].as<std::string>();
        
        if(database.query_protocols(QUERY("id" << protocol)).size() == 0)
        {
            database.insert_protocol(research_pacs::Protocol(protocol, "", sponsor));
        }
    }
    
    std::vector<std::string> input_files = vm["input-file"].as<std::vector<std::string> >();
    
    for(std::vector<std::string>::const_iterator it=input_files.begin();
        it!=input_files.end(); ++it)
    {
        if(!gdcm::System::FileExists(it->c_str()))
        {
            std::cerr << "Cannot store : file " << *it << " does not exist\n";
            continue;
        }
        
        gdcm::Reader reader;
        reader.SetFileName(it->c_str());
        if(!reader.Read())
        {
            if(vm.count("sponsor") == 0 || vm.count("protocol") == 0 ||
               vm.count("subject") == 0)
            {
                std::cerr << "Cannot insert non-DICOM file "
                          << "\"" << *it << "\" : sponsor, protocol, or subject missing\n";
                continue;
            }
            
            std::string const sponsor = vm["sponsor"].as<std::string>();
            std::string const protocol = vm["protocol"].as<std::string>();
            std::string const subject = vm["subject"].as<std::string>();
            
            try
            {
                database.insert_file(*it, sponsor, protocol, subject);
            }
            catch(research_pacs::exception const & e)
            {
                std::cerr << "Cannot insert non-DICOM file "
                          << "\"" << *it << "\"" << " : " << e.what() << "\n";
                continue;
            }
        }
        else
        {
            gdcm::DataSet dataset = reader.GetFile().GetDataSet();
            
            if(vm.count("sponsor")>0)
            {
                std::string const sponsor = vm["sponsor"].as<std::string>();
                gdcm::Attribute<0x0012,0x0010> attribute;
                attribute.SetValue(sponsor);
                dataset.Insert(attribute.GetAsDataElement());
            }
            
            if(vm.count("protocol")>0)
            {
                std::string const protocol = vm["protocol"].as<std::string>();
                gdcm::Attribute<0x0012,0x0020> attribute;
                attribute.SetValue(protocol);
                dataset.Insert(attribute.GetAsDataElement());
            }
            
            if(vm.count("subject")>0)
            {
                std::string const subject = vm["subject"].as<std::string>();
                gdcm::Attribute<0x0012,0x0040> attribute;
                attribute.SetValue(subject);
                dataset.Insert(attribute.GetAsDataElement());
            }
            
            try
            {
                database.insert_dataset(dataset);
            }
            catch(research_pacs::exception const & e)
            {
                std::cerr << "Cannot insert DICOM file "
                          << "\"" << *it << "\"" << " : " << e.what() << "\n";
                continue;
            }
        }
        std::cout << "File \"" << *it << "\" inserted\n";
    }

    return EXIT_SUCCESS;
}

int query(int argc, char** argv)
{
    std::string const syntax("Syntax : query QUERY\n");

    if(argc != 1)
    {
        std::cerr << syntax;
        return EXIT_FAILURE;
    }

    std::string const query(argv[0]);

    research_pacs::Database database("research_pacs");
    mongo::auto_ptr<mongo::DBClientCursor> cursor = database.query_documents(query);
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

    research_pacs::Database database("research_pacs");

    try
    {
        std::ofstream stream(filename.c_str());
        database.get_file(sop_instance_uid, stream);
        stream.close();
    }
    catch(std::exception const & e)
    {
        std::cerr << e.what() << "\n";
        return EXIT_FAILURE;
    }

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
        // boost::program_options needs a fake argument as argv[0]
        return store(argc-1, argv+1);
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
