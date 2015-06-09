/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <dcmtk/config/osconfig.h>
#include <dcmtk/ofstd/ofstd.h>

#include "ConverterBase64.h"

namespace dopamine
{

namespace ConverterBase64
{

std::string
encode(std::string const & bindata, unsigned int linebreak)
{
    OFString result;
    OFStandard::encodeBase64((unsigned char*)bindata.c_str(),
                             bindata.size(), result);
    std::string retval(result.c_str());

    if (linebreak != 0)
    {
        std::stringstream buffer;
        unsigned int count = 0;
        while (count < retval.size())
        {
            if (buffer.str().size() != 0)
            {
                buffer << '\n'; // add endline
            }
            buffer << retval.substr(count, linebreak);

            count += linebreak;
        }

        retval = buffer.str();
    }

    return retval;
}

std::string
decode(std::string const & ascdata)
{
    unsigned char* result;
    size_t size = OFStandard::decodeBase64(OFString(ascdata.c_str()), result);
    return std::string((char*)result, size);
}

} // namespace ConverterBase64

} // namespace dopamine
