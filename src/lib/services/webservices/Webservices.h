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

#include <mongo/client/dbclient.h>

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

const std::string ATTRIBUT_BOUNDARY = "boundary=";
const std::string ATTRIBUT_FILENAME = "filename=";
const std::string CONTENT_DISPOSITION_ATTACHMENT = "Content-Disposition: attachment;";
const std::string CONTENT_TRANSFER_ENCODING = "Content-Transfer-Encoding: ";
const std::string CONTENT_TYPE = "Content-Type: ";
const std::string MIME_TYPE_APPLICATION_DICOM = "application/dicom";
const std::string MIME_TYPE_APPLICATION_DICOMXML = "application/dicom+xml";
const std::string MIME_TYPE_APPLICATION_JSON = "application/json";
const std::string MIME_TYPE_MULTIPART_RELATED = "multipart/related";
const std::string MIME_VERSION = "MIME-Version: 1.0";
const std::string TRANSFER_ENCODING_BINARY = "binary";

class Webservices
{
public:
    Webservices(std::string const & pathinfo,
                std::string const & querystring,
                std::string const & username);

    virtual ~Webservices();

    std::string get_response() const;

    std::string get_boundary() const;

protected:
    std::string _pathinfo;
    std::string _querystring;
    std::string _username;
    std::string _response;
    std::string _boundary;

    int _maximumResults;
    int _skippedResults;
    bool _fuzzymatching;

    void create_boundary();

    virtual mongo::BSONObj parse_string() = 0;

private:

};

} // namespace services

} // namespace dopamine

#endif // _3e6e62f3_9f0c_4f61_9907_bc6e62b3fc46
