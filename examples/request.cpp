/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <memory>

#include <boost/filesystem.hpp>

#include "core/ConfigurationPACS.h"
#include "core/LoggerPACS.h"
#include "services/ServicesTools.h"

int main()
{
    char* conffile = getenv("DOPAMINE_TEST_CONFIG");
    std::string NetworkConfFILE;
    if (conffile != NULL)
    {
        NetworkConfFILE = std::string(conffile);
    }
    // Read configuration file
    dopamine::ConfigurationPACS& configuration =
            dopamine::ConfigurationPACS::get_instance();
    std::string const localconf = "../configuration/dopamine_conf.ini";
    if (NetworkConfFILE != "")
    {
        configuration.parse(NetworkConfFILE);
    }
    else if (boost::filesystem::exists(boost::filesystem::path(localconf)))
    {
        configuration.parse(localconf);
    }
    else
    {
        configuration.parse("/etc/dopamine/dopamine_conf.ini");
    }

    // Create and Initialize Logger
    dopamine::initialize_logger
    (
        dopamine::ConfigurationPACS::get_instance().get_value("logger.priority")
    );

    mongo::DBClientConnection connection;
    std::string db_name;
    dopamine::services::create_db_connection(connection, db_name);

    mongo::BSONObj const object = BSON("distinct" << "datasets" <<
                                       "key" << "00081030" <<
                                       "query" << mongo::BSONObj());

    mongo::BSONObj info;
    bool ret = connection.runCommand(db_name, object, info);

    dopamine::logger_info() << "Study number: " << info["values"].Array().size();
    for (mongo::BSONElement const value : info["values"].Array())
    {
        std::string const study = value.Obj()["Value"].Array()[0].String();
        dopamine::logger_info() << "  " << study;

        std::map<std::string, int> nbexam;
        {
        // Find Patient number
        mongo::BSONObj const objectp = BSON("distinct" << "datasets" <<
                                            "key" << "00100020" <<
                                            "query" << BSON("00081030.Value" <<
                                                            study));
        mongo::BSONObj infop;
        bool retp = connection.runCommand(db_name, objectp, infop);

        dopamine::logger_info() << "    patient number: "
                                << infop["values"].Array().size();
        for (mongo::BSONElement const valuep : infop["values"].Array())
        {
            mongo::BSONObj const objecte =
                    BSON("distinct" << "datasets" << "key" << "0008103e" <<
                         "query" << BSON("00081030.Value" << study <<
                                         "00100020.Value" << valuep.Obj()
                                            ["Value"].Array()[0].String()));
            mongo::BSONObj infoe;
            bool rete = connection.runCommand(db_name, objecte, infoe);

            std::stringstream stream;
            stream << infoe["values"].Array().size();

            if (nbexam.find(stream.str()) == nbexam.end())
            {
                nbexam.insert(std::pair<std::string, int>(stream.str(), 0));
            }
            ++nbexam.find(stream.str())->second;
        }
        }

        {
        // Find Series Description
        mongo::BSONObj const objectp = BSON("distinct" << "datasets" <<
                                            "key" << "0008103e" <<
                                            "query" << BSON("00081030.Value" <<
                                                            study));
        mongo::BSONObj infop;
        bool retp = connection.runCommand(db_name, objectp, infop);

        dopamine::logger_info() << "    Series number: "
                                << infop["values"].Array().size();
        for (mongo::BSONElement const valuep : infop["values"].Array())
        {
            std::string const series = valuep.Obj()["Value"].Array()[0].String();
            dopamine::logger_info() << "      " << series;
        }
        }

        for (auto it = nbexam.begin(); it != nbexam.end(); ++it)
        {
            dopamine::logger_info() << "    patient with "<< it->first
                                    << " exam: " << it->second;
        }
    }

    return EXIT_SUCCESS;
}
