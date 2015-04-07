/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _76b2ab31_bb60_4366_ad0b_55c6eb286fdf
#define _76b2ab31_bb60_4366_ad0b_55c6eb286fdf

#include <dcmtk/config/osconfig.h> /* make sure OS specific configuration is included first */
#include <dcmtk/dcmdata/dcdatset.h>
#include <dcmtk/dcmdata/dctagkey.h>

#include "ResponseGenerator.h"
#include "ServicesTools.h"

namespace dopamine
{

namespace services
{

class QueryRetrieveGenerator : public ResponseGenerator
{
public:
    QueryRetrieveGenerator(std::string const & username,
                           std::string const & service_name);

    virtual Uint16 set_query(DcmDataset * dataset);

    DcmDataset* bson_to_dataset(mongo::BSONObj object);

    DcmTagKey get_instance_count_tag() const;

    bool get_convert_modalities_in_study() const;

    std::string get_query_retrieve_level() const;

protected:
    std::string _service_name;

    std::string _query_retrieve_level;

    DcmTagKey _instance_count_tag;

    /// flag indicating if modalities should be convert
    bool _convert_modalities_in_study;

};

} // namespace services

} // namespace dopamine

#endif // _76b2ab31_bb60_4366_ad0b_55c6eb286fdf
