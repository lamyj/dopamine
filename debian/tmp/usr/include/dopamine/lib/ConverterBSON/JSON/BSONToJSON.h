/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _edad868f_b3db_4bd1_b3e1_f642f4823f44
#define _edad868f_b3db_4bd1_b3e1_f642f4823f44

#include "ConverterBSONJSON.h"

namespace dopamine
{

namespace converterBSON
{

/**
 * @brief \class The BSONToJSON class
 * Convert a BSON Object into JSON object
 */
class BSONToJSON : public ConverterBSONJSON
{
public:
    /// Create an instance of BSONToJSON
    BSONToJSON();

    /// Destroy the instance of BSONToJSON
    ~BSONToJSON();

    /**
     * to_json
     * @param bson: BSON object to convert
     * @return converted JSON Object
     */
    mongo::BSONObj to_json(mongo::BSONObj const & bson);

    /**
     * to_string
     * @param bson: BSON object to convert
     * @return converted JSON Object as String
     */
    std::string to_string(mongo::BSONObj const & bson);

protected:

private:

};

} // namespace converterBSON

} // namespace dopamine

#endif // _edad868f_b3db_4bd1_b3e1_f642f4823f44
