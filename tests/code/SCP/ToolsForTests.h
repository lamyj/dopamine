/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _1ea946b5_2d78_4999_b8a2_10d7dea75b25
#define _1ea946b5_2d78_4999_b8a2_10d7dea75b25

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>

#include <qt4/Qt/qstring.h>
#include <qt4/Qt/qstringlist.h>
#include <qt4/Qt/qprocess.h>

#include "core/ConfigurationPACS.h"
#include "core/DBConnection.h"
#include "core/ExceptionPACS.h"
#include "core/NetworkPACS.h"

const std::string NetworkConfFILE = "./tmp_test_ModuleSCP_conf.ini";

void launchNetwork()
{
    research_pacs::ConfigurationPACS::get_instance().Parse(NetworkConfFILE);

    // Get all indexes
    std::string indexlist =
        research_pacs::ConfigurationPACS::get_instance().GetValue("database.indexlist");
    std::vector<std::string> indexlistvect;
    boost::split(indexlistvect, indexlist, boost::is_any_of(";"));

    // Create and Initialize DB connection
    research_pacs::DBConnection::get_instance().Initialize
        (
            research_pacs::ConfigurationPACS::get_instance().GetValue("database.dbname"),
            research_pacs::ConfigurationPACS::get_instance().GetValue("database.hostname"),
            research_pacs::ConfigurationPACS::get_instance().GetValue("database.port"),
            indexlistvect
        );

    // Connect Database
    research_pacs::DBConnection::get_instance().connect();

    research_pacs::NetworkPACS& networkpacs =
            research_pacs::NetworkPACS::get_instance();
    networkpacs.run();

    research_pacs::ConfigurationPACS::delete_instance();
    research_pacs::NetworkPACS::delete_instance();
}

void terminateNetwork()
{
    // Call Terminate SCU
    QString command = "termscu";
    QStringList args;
    args << "localhost" << "11112";

    QProcess *myProcess = new QProcess();
    myProcess->start(command, args);
    myProcess->waitForFinished(5000);
}

#endif // _1ea946b5_2d78_4999_b8a2_10d7dea75b25
