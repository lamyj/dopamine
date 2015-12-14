/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _3e6e62f3_9f0c_4f61_9907_bc6e62b3fc46
#define _3e6e62f3_9f0c_4f61_9907_bc6e62b3fc46

#include <string>

#include <mongo/bson/bson.h>

namespace dopamine
{

namespace services
{

std::string const authentication_string = "This server could not verify that \
you are authorized to access the \
document requested. Either you supplied \
the wrong credentials (e.g., bad password), \
or your browser doesn't understand how to \
supply the credentials required.";

std::string const ATTRIBUT_BOUNDARY = "boundary=";
std::string const ATTRIBUT_FILENAME = "filename=";
std::string const CONTENT_DISPOSITION_ATTACHMENT =
        "Content-Disposition: attachment;";
std::string const CONTENT_TRANSFER_ENCODING = "Content-Transfer-Encoding: ";
std::string const CONTENT_TYPE = "Content-Type: ";
std::string const MIME_TYPE_APPLICATION_DICOM = "application/dicom";
std::string const MIME_TYPE_APPLICATION_DICOMXML = "application/dicom+xml";
std::string const MIME_TYPE_APPLICATION_JSON = "application/json";
std::string const MIME_TYPE_MULTIPART_RELATED = "multipart/related";
std::string const MIME_VERSION = "MIME-Version: 1.0";
std::string const TRANSFER_ENCODING_BINARY = "binary";

/**
 * @brief \class The Webservices class
 */
class Webservices
{
public:
    /**
     * @brief Create an instance of Webservices
     * @param pathinfo
     * @param querystring
     */
    Webservices(std::string const & pathinfo,
                std::string const & querystring);

    /// Destroy the instance of Webservices
    virtual ~Webservices();

    std::string get_response() const;

    std::string get_boundary() const;

protected:
    std::string _pathinfo;
    std::string _querystring;
    std::string _response;
    std::string _boundary;

    std::string _query_retrieve_level;

    void _create_boundary();

    virtual mongo::BSONObj _parse_string() = 0;

private:

};

} // namespace services

} // namespace dopamine

#endif // _3e6e62f3_9f0c_4f61_9907_bc6e62b3fc46
