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

class JSONToBSON : public ConverterBSONJSON
{
public:
    JSONToBSON();

    ~JSONToBSON();

    mongo::BSONObj from_JSON(mongo::BSONObj const & json);

    mongo::BSONObj from_string(std::string const & json);

protected:

private:

};

} // namespace converterBSON

} // namespace dopamine

#endif // _6dfa87ad_a858_4063_93a0_24ed28a3a9df
