/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <iostream>
#include <stdlib.h>
#include <sstream>

#include <boost/filesystem.hpp>

#include <cgicc/Cgicc.h>
#include <cgicc/HTTPContentHeader.h>
#include <cgicc/HTTPResponseHeader.h>
#include <cgicc/HTTPStatusHeader.h>
#include "cgicc/HTMLClasses.h"

#include "core/ConfigurationPACS.h"
#include "services/webservices/Wado_uri.h"
#include "services/webservices/WebServiceException.h"

int main(int argc, char** argv)
{
    try
    {
        std::string const syntax = "wado-uri -f CONFIG_FILE";
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

        // Create the response
        dopamine::services::Wado_uri wadouri(environment.getQueryString(),
                                             environment.getRemoteUser());

        // send response
        std::stringstream headerstream;
        headerstream << dopamine::services::MIME_TYPE_APPLICATION_DICOM
                     << std::endl
                     << "Content-Disposition: attachment; filename="
                     << wadouri.get_filename();

        std::cout << cgicc::HTTPResponseHeader("HTTP/1.1", 200, "OK") << "\n";
        std::cout << cgicc::HTTPContentHeader(headerstream.str());
        std::string data = wadouri.get_response();
        std::cout.write(&data[0], data.size());
    }
    catch (dopamine::services::WebServiceException const & exc)
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
    catch (std::exception const & e)
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
