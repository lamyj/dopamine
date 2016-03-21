/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <cstdio>
#include <stdlib.h>

#include <boost/filesystem.hpp>

#include <cgicc/Cgicc.h>
#include <cgicc/HTMLClasses.h>
#include <cgicc/HTMLDoctype.h>
#include <cgicc/HTTPResponseHeader.h>

#include "core/ConfigurationPACS.h"
#include "services/webservices/Stow_rs.h"
#include "services/webservices/WebServiceException.h"

int main(int argc, char** argv)
{
    try
    {
        std::string const syntax = "stow-rs -f CONFIG_FILE";
        if(argc != 3 || std::string(argv[1]) != std::string("-f"))
        {
            std::cerr << "Syntax: " << syntax << "\n";
            return 1;
        }

        // Read configuration file
        dopamine::ConfigurationPACS& configuration =
                dopamine::ConfigurationPACS::get_instance();
        std::string const config_file(argv[2]);
        if(!boost::filesystem::exists(config_file))
        {
            std::cerr << "No such file: '" << config_file << "'\n";
            std::cerr << "Syntax: " << syntax << "\n";
            return 1;
        }
        configuration.parse(config_file);

        cgicc::Cgicc cgi;

        // Get the Environment Variables
        cgicc::CgiEnvironment const & environment = cgi.getEnvironment();

        // read standard input
        char buf[1024]; // size is not known, read block by block
        std::stringstream data;
        while(!std::cin.eof())
        {
            std::cin.read(buf, 1024);
            if(!std::cin.bad())
            {
                data << std::string(&buf[0], 1024);
            }
        }

        // Create the response
        dopamine::services::Stow_rs stowrs(environment.getPathInfo(),
                                           environment.getQueryString(),
                                           data.str(),
                                           environment.getRemoteUser());

        std::cout << cgicc::HTTPResponseHeader("HTTP/1.1", stowrs.get_status(),
                                               stowrs.get_code())
                .addHeader("Content-Type",
                           dopamine::services::MIME_TYPE_APPLICATION_DICOMXML);
        std::cout << stowrs.get_response() << "\n";
    }
    catch (dopamine::services::WebServiceException &exc)
    {
        if (exc.status() == 401)
        {
            std::cout << cgicc::HTTPResponseHeader("HTTP/1.1", exc.status(),
                                                   exc.statusmessage())
                    .addHeader("WWW-Authenticate", "Basic realm=\"cgicc\"");
        }
        else
        {
            std::cout << cgicc::HTTPResponseHeader("HTTP/1.1", exc.status(),
                                                   exc.statusmessage())
                    .addHeader("Content-Type", "text/html; charset=UTF-8");
        }

        std::stringstream stream;
        stream << exc.status() << " " << exc.statusmessage();

        std::cout << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict)
                  << std::endl;
        std::cout << cgicc::html().set("lang", "EN").set("dir", "LTR")
                  << std::endl;

        std::cout << cgicc::head() << std::endl;
        std::cout << "\t" << cgicc::title(stream.str()) << std::endl;
        std::cout << cgicc::head() << std::endl;

        std::cout << cgicc::body() << std::endl;
        std::cout << "\t" << cgicc::h1(stream.str()) << std::endl;
        std::cout << "\t" << cgicc::p() << exc.what() << cgicc::p()
                  << std::endl;

        std::cout << cgicc::body() << std::endl;

        std::cout << cgicc::html() << std::endl;
    }
    catch (std::exception &e)
    {
        std::cout << cgicc::HTTPResponseHeader("HTTP/1.1", 500,
                                               "Internal Server Error")
                        .addHeader("Content-Type", "text/html; charset=UTF-8");

        std::cout << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict)
                  << std::endl;
        std::cout << cgicc::html().set("lang", "EN").set("dir", "LTR")
                  << std::endl;

        std::cout << cgicc::head() << std::endl;
        std::cout << "\t" << cgicc::title("500 Internal Server Error")
                  << std::endl;
        std::cout << cgicc::head() << std::endl;

        std::cout << cgicc::body() << std::endl;
        std::cout << "\t" << cgicc::h1("500 Internal Server Error") << std::endl;
        std::cout << "\t" << cgicc::p() << e.what() << cgicc::p() << std::endl;
        std::cout << cgicc::body() << std::endl;

        std::cout << cgicc::html() << std::endl;
    }

    return EXIT_SUCCESS;
}
