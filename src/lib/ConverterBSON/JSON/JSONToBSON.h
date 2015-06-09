/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _6dfa87ad_a858_4063_93a0_24ed28a3a9df
#define _6dfa87ad_a858_4063_93a0_24ed28a3a9df

#include "ConverterBSONJSON.h"

namespace dopamine
{

namespace converterBSON
{

/**
 * @brief The JSONToBSON class
 * Convert JSON Object into BSON Object
 */
class JSONToBSON : public ConverterBSONJSON
{
public:
    /// Create an instance of JSONToBSON
    JSONToBSON();

    /// Destroy the instance of JSONToBSON
    ~JSONToBSON();

    /**
     * @brief from_json
     * @param json: JSON Object to convert
     * @return Converted BSON Object
     */
    mongo::BSONObj from_json(mongo::BSONObj const & json);

    /**
     * @brief from_string
     * @param json: JSON Object as string to convert
     * @return Converted BSON Object
     */
    mongo::BSONObj from_string(std::string const & json);

protected:

private:

};

} // namespace converterBSON

} // namespace dopamine

#endif // _6dfa87ad_a858_4063_93a0_24ed28a3a9df
