#include <dcmtk/dcmdata/dcdict.h>
#include <dcmtk/dcmqrdb/dcmqrcnf.h>
#include <dcmtk/dcmqrdb/dcmqropt.h>

#include <iostream>

#include "DcmQueryRetrieveSCP.h"

int main(int argc, char** argv)
{
    if(argc != 3)
    {
        std::cerr << "Syntax: " << argv[0] << " <config_file> <storage>\n";
        return EXIT_FAILURE;
    }

    OFCondition cond;
    OFString temp_str;

    DcmQueryRetrieveConfig config;
    config.init(argv[1]);

    DcmQueryRetrieveOptions options;

    options.maxAssociations_ = config.getMaxAssociations();
    int opt_port = config.getNetworkTCPPort();
    if (opt_port == 0) opt_port = 104; /* not set, use default */

    options.maxPDU_ = config.getMaxPDUSize();
    if (options.maxPDU_ == 0) options.maxPDU_ = ASC_DEFAULTMAXPDU; /* not set, use default */
    if (options.maxPDU_ < ASC_MINIMUMPDUSIZE || options.maxPDU_ > ASC_MAXIMUMPDUSIZE)
    {
        std::cerr << "invalid MaxPDUSize in config file\n";
        return 10;
    }

    if (!dcmDataDict.isDictionaryLoaded())
    {
        std::cerr << "no data dictionary loaded, check environment variable: "
                  << DCM_DICT_ENVIRONMENT_VARIABLE << "\n";
    }

    cond = ASC_initializeNetwork(NET_ACCEPTORREQUESTOR, (int)opt_port, options.acse_timeout_, &options.net_);
    if (cond.bad()) {
        std::cerr << "cannot initialize network: " << DimseCondition::dump(temp_str, cond) << "\n";
        return 10;
    }

    DcmQueryRetrieveSCP::DbConnection db_connection;
    db_connection.host = "localhost";
    db_connection.port = 27017;
    db_connection.db_name = "pacs";
    DcmQueryRetrieveSCP scp(config, options, db_connection, argv[2]);

    while (cond.good())
    {
        cond = scp.waitForAssociation(options.net_);
        if (!options.singleProcess_)
        {
            scp.cleanChildren();  /* clean up any child processes */
        }
    }

    cond = ASC_dropNetwork(&options.net_);
    if (cond.bad())
    {
        //OFLOG_FATAL(dcmqrscpLogger, "cannot drop network: " << DimseCondition::dump(temp_str, cond));
        std::cerr << "cannot drop network: " << DimseCondition::dump(temp_str, cond) << std::endl;
        return 10;
    }

    return EXIT_SUCCESS;

    // TODO : use DCM4CHEE-like ACLSs, i.e. DICOM matching rules to allow or
    // deny access to a dataset.
    // User-originated actions are :
    // Q (Query) – Query Study
    // R (Read) – Retrieve Study
    // E (Export) – Export Study to XDS/TCE
    // A (Append) – Append data to study
    // U (Updated) – Update Attributes in Webinterface
    // D (Delete) – Delete Study in Webinterface
    // cf. http://www.dcm4che.org/confluence/display/ee2/Configuration+of+Study+Permissions+%28Role+Based+Access+Control%29
}

