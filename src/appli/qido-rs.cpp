/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <stdlib.h>

#include <boost/filesystem.hpp>

#include <cgicc/Cgicc.h>
#include <cgicc/HTMLClasses.h>
#include <cgicc/HTMLDoctype.h>
#include <cgicc/HTTPResponseHeader.h>
#include <cgicc/HTTPStatusHeader.h>

#include "core/ConfigurationPACS.h"
#include "services/webservices/Qido_rs.h"
#include "services/webservices/WebServiceException.h"

int main(int argc, char** argv)
{
    try
    {
        std::string const localconf = "../../../configuration/dopamine_conf.ini";
        // Read configuration file
        if (boost::filesystem::exists(boost::filesystem::path(localconf)))
        {
            dopamine::ConfigurationPACS::get_instance().parse(localconf);
        }
        else
        {
            dopamine::ConfigurationPACS::
                    get_instance().parse("/etc/dopamine/dopamine_conf.ini");
        }

        cgicc::Cgicc cgi;

        // Get the Environment Variables
        cgicc::CgiEnvironment const & environment = cgi.getEnvironment();

        // Create the response
        dopamine::services::Qido_rs qidors(environment.getPathInfo(),
                                           environment.getQueryString(),
                                           environment.getContentType(),
                                           environment.getRemoteUser());

        // send response
        std::ostringstream headerstream;
        headerstream << dopamine::services::MIME_VERSION << "\n"
                     << dopamine::services::CONTENT_TYPE;

        if (environment.getContentType() ==
            dopamine::services::MIME_TYPE_APPLICATION_DICOMXML)
        {
            headerstream << dopamine::services::MIME_TYPE_MULTIPART_RELATED
                         << "; "
                         << dopamine::services::ATTRIBUT_BOUNDARY
                         << qidors.get_boundary() << "\n";
        }

        std::cout << cgicc::HTTPResponseHeader("HTTP/1.1", 200, "OK") << "\n";
        std::cout << headerstream.str() << "\n";
        std::cout << qidors.get_response() << "\n";
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
        std::cout << "\t" << cgicc::p() << exc.what() << cgicc::p() << std::endl;

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
