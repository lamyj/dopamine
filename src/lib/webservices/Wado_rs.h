/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _23d72937_8451_4cd1_a64d_77b774672145
#define _23d72937_8451_4cd1_a64d_77b774672145

#include <string>

#include <mongo/client/dbclient.h>

namespace dopamine
{

namespace webservices
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

std::string wado_rs(std::string const & pathinfo, std::string & filename);

class Wado_rs
{
public:
    Wado_rs(std::string const & pathinfo);

    ~Wado_rs();

    std::string get_filename() const { return this->_filename; }

    std::string get_boundary() const { return this->_boundary; }

    std::string get_response() const { return this->_response; }

protected:
    void set_filename(std::string const & filename) { this->_filename = filename; }

private:
    std::string _filename;
    std::string _study_instance_uid;
    std::string _series_instance_uid;
    std::string _sop_instance_uid;
    std::vector<mongo::BSONElement> _results;
    std::string _response;
    std::string _boundary;

    void parse_pathfinfo(std::string const & pathinfo);

    void search_database();

    void create_response();

    void create_boundary();

};

} // namespace webservices

} // namespace dopamine

#endif // _23d72937_8451_4cd1_a64d_77b774672145
