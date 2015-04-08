/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _23d72937_8451_4cd1_a64d_77b774672145
#define _23d72937_8451_4cd1_a64d_77b774672145

#include "Wado.h"

namespace dopamine
{

namespace services
{

const std::string ATTRIBUT_BOUNDARY = "boundary=";
const std::string ATTRIBUT_FILENAME = "filename=";
const std::string CONTENT_DISPOSITION_ATTACHMENT = "Content-Disposition: attachment;";
const std::string CONTENT_TRANSFER_ENCODING = "Content-Transfer-Encoding: ";
const std::string CONTENT_TYPE = "Content-Type: ";
const std::string MIME_TYPE_APPLICATION_DICOM = "application/dicom";
const std::string MIME_TYPE_MULTIPART_RELATED = "multipart/related";
const std::string MIME_VERSION = "MIME-Version: 1.0";
const std::string TRANSFER_ENCODING_BINARY = "binary";

class Wado_rs: public Wado
{
public:
    Wado_rs(std::string const & pathinfo,
            std::string const & remoteuser = "");

    ~Wado_rs();

    std::string get_boundary() const;

protected:

private:
    std::string _boundary;

    virtual mongo::BSONObj parse_string();

    void create_boundary();

};

} // namespace services

} // namespace dopamine

#endif // _23d72937_8451_4cd1_a64d_77b774672145
