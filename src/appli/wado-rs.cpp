/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <iostream>
#include <sstream>
#include <stdlib.h>

#include <boost/filesystem.hpp>

#include <cgicc/Cgicc.h>
#include <cgicc/HTTPContentHeader.h>
#include <cgicc/HTTPStatusHeader.h>
#include <cgicc/HTMLClasses.h>
#include <cgicc/HTTPResponseHeader.h>

#include "core/ConfigurationPACS.h"
#include "webservices/Wado_rs.h"
#include "webservices/WebServiceException.h"

int main(int argc, char** argv)
{
    try
    {
        // Read configuration file
        if (boost::filesystem::exists(boost::filesystem::path("../../../configuration/dopamine_conf.ini")))
        {
            dopamine::ConfigurationPACS::get_instance().Parse("../../../configuration/dopamine_conf.ini");
        }
        else
        {
            dopamine::ConfigurationPACS::get_instance().Parse("/etc/dopamine/dopamine_conf.ini");
        }

        cgicc::Cgicc cgi;

        // Get the Environment Variables
        cgicc::CgiEnvironment const & environment = cgi.getEnvironment();

        // Create the response
        dopamine::webservices::Wado_rs wadors(environment.getPathInfo(), environment.getRemoteUser());

        // send response
        std::ostringstream headerstream;
        headerstream << dopamine::webservices::MIME_VERSION << "\n"
                     << dopamine::webservices::CONTENT_TYPE
                     << dopamine::webservices::MIME_TYPE_MULTIPART_RELATED << "; "
                     << dopamine::webservices::ATTRIBUT_BOUNDARY
                     << wadors.get_boundary() << "\n";

        std::cout << headerstream.str() << "\n";
        std::cout << wadors.get_response() << "\n";
    }
    catch (dopamine::webservices::WebServiceException &exc)
    {
        if (exc.status() == 401)
        {
            std::cout << cgicc::HTTPResponseHeader("HTTP/1.1", exc.status(), exc.statusmessage())
                            .addHeader("WWW-Authenticate", "Basic realm=\"cgicc\"");
        }
        else
        {
            std::cout << cgicc::HTTPStatusHeader(exc.status(), exc.statusmessage()) << std::endl;
        }

        std::stringstream stream;
        stream << exc.status() << " " << exc.statusmessage();

        std::cout << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
        std::cout << cgicc::html().set("lang", "EN").set("dir", "LTR") << std::endl;

        std::cout << cgicc::head() << std::endl;
        std::cout << "\t" << cgicc::title(stream.str()) << std::endl;
        std::cout << cgicc::head() << std::endl;

        std::cout << cgicc::body() << std::endl;
        std::cout << "\t" << cgicc::h1(stream.str()) << std::endl;
        std::cout << "\t" << cgicc::p() << exc.what() << cgicc::p() << std::endl;

        std::cout << cgicc::body() << std::endl;

        std::cout << cgicc::html() << std::endl;
    }
    catch (std::exception &e)
    {
        std::cout << cgicc::HTTPStatusHeader(500, "Internal Server Error") << std::endl;

        std::cout << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
        std::cout << cgicc::html().set("lang", "EN").set("dir", "LTR") << std::endl;

        std::cout << cgicc::head() << std::endl;
        std::cout << "\t" << cgicc::title("500 Internal Server Error") << std::endl;
        std::cout << cgicc::head() << std::endl;

        std::cout << cgicc::body() << std::endl;
        std::cout << "\t" << cgicc::h1("500 Internal Server Error") << std::endl;
        std::cout << "\t" << cgicc::p() << e.what() << cgicc::p() << std::endl;
        std::cout << cgicc::body() << std::endl;

        std::cout << cgicc::html() << std::endl;
    }

    return EXIT_SUCCESS;
}
