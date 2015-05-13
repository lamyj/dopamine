/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <cassert>
#include <cctype>
#include <limits>
#include <sstream>
#include <stdexcept>

#include "ConverterBase64.h"

namespace dopamine
{

namespace ConverterBase64
{

std::string encode(const std::string &bindata, unsigned int linebreak)
{
    if (bindata.size() > (std::numeric_limits<std::string::size_type>::max() / 4u) * 3u)
    {
       throw std::length_error("Converting too large a string to base64.");
    }

    const std::size_t binlen = bindata.size();
    // Use = signs so the end is properly padded.
    std::string retval((((binlen + 2) / 3) * 4), '=');
    std::size_t outpos = 0;
    int bits_collected = 0;
    unsigned int accumulator = 0;

    for (std::string::const_iterator i = bindata.begin(); i != bindata.end(); ++i)
    {
       accumulator = (accumulator << 8) | (*i & 0xffu);
       bits_collected += 8;
       while (bits_collected >= 6)
       {
          bits_collected -= 6;
          retval[outpos++] = b64_table[(accumulator >> bits_collected) & 0x3fu];
       }
    }
    if (bits_collected > 0)
    { // Any trailing bits that are missing.
       accumulator <<= 6 - bits_collected;
       retval[outpos++] = b64_table[accumulator & 0x3fu];
    }

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

std::string decode(const std::string &ascdata)
{
    std::string retval;
    int bits_collected = 0;
    unsigned int accumulator = 0;

    for (std::string::const_iterator i = ascdata.begin(); i != ascdata.end(); ++i)
    {
       const int c = *i;
       if (std::isspace(c) || c == '=')
       {
          // Skip whitespace and padding. Be liberal in what you accept.
          continue;
       }
       if ((c > 127) || (c < 0) || (reverse_table[c] > 63))
       {
          throw std::invalid_argument("This contains characters not legal in a base64 encoded string.");
       }
       accumulator = (accumulator << 6) | reverse_table[c];
       bits_collected += 6;
       if (bits_collected >= 8)
       {
          bits_collected -= 8;
          retval += (char)((accumulator >> bits_collected) & 0xffu);
       }
    }
    return retval;
}

} // namespace ConverterBase64

} // namespace dopamine
