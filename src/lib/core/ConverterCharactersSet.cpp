/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <sstream>
#include <string.h>

#include <unicode/errorcode.h>
#include <unicode/uchar.h>
#include <unicode/ucnv.h>
#include <unicode/unistr.h>

#include "ConverterCharactersSet.h"
#include "core/ExceptionPACS.h"

namespace dopamine
{

namespace characterset
{

std::string
convert(const std::string &input,
        const std::string &from,
        const std::string &to)
{
    if (to == "UTF-8")
    {
        std::string result;
        UnicodeString(input.c_str(), input.size(), from.c_str()).toUTF8String(result);
        return result;
    }

    UErrorCode status = U_ZERO_ERROR;

    // Create the converter from input character set to Unicode
    UConverter *conv_to_unicode;
    conv_to_unicode = ucnv_open(from.c_str(), &status);
    if (U_FAILURE(status))
    {
        std::stringstream streamerror;
        streamerror << "Could not create converter from " << from
                    << " to Unicode - code: " << u_errorName(status);
        throw ExceptionPACS(streamerror.str());
    }

    UChar * convDest = new UChar[2*input.length()]; //ucnv_toUChars will use up to 2*length

    // convert to Unicode
    int resultLen = (int)ucnv_toUChars(conv_to_unicode, convDest, 2*input.length(),
                                       input.c_str(), input.length(), &status);
    if (U_FAILURE(status))
    {
        std::stringstream streamerror;
        streamerror << "Could not convert " << input << " from " << from
                    << " to Unicode - code: " << u_errorName(status);
        throw ExceptionPACS(streamerror.str());
    }

    // Destroy the converter
    ucnv_close(conv_to_unicode);

    // Create the converter from Unicode to output character set
    UConverter *conv_from_unicode;
    conv_from_unicode = ucnv_open(to.c_str(), &status);
    if (U_FAILURE(status))
    {
        std::stringstream streamerror;
        streamerror << "Could not create converter from Unicode to " << to
                    << " - code: " << u_errorName(status);
        throw ExceptionPACS(streamerror.str());
    }

    char result[input.length()*4]; // make sure the buffer is big enough
    // Convert to chosen character set
    int size = ucnv_fromUChars(conv_from_unicode, &result[0], input.length()*4,
                               convDest, resultLen, &status);
    if (U_FAILURE(status))
    {
        std::stringstream streamerror;
        streamerror << "Could not convert " << input << " from Unicode to " << to
                    << " - code: " << u_errorName(status);
        throw ExceptionPACS(streamerror.str());
    }

    // Destroy the converter
    ucnv_close(conv_from_unicode);

    std::string output(&result[0], size);

    delete [] convDest;

    return output;
}

std::string
convert_to_utf8_with_escape(std::string const & input,
                            std::string const & current_character_set,
                            std::string const & default_character_set)
{
    std::stringstream returnstream;

    std::string character_set = current_character_set;

    std::stringstream to_translate;
    // look for each character
    for (std::string::const_iterator it = input.begin(); it != input.end(); ++it)
    {
        if (is_control_character(*it))
        {
            // translate
            if (to_translate.str().size() > 0)
            {
                returnstream << convert(to_translate.str(),
                                        _dicom_to_iconv.find(character_set)->second.first,
                                        "UTF-8");
            }
            to_translate.str(std::string()); // clear the stringstream

            // control character => use default_character_set until the end
            character_set = default_character_set;
        }

        to_translate << *it;
    }

    // translate the end of buffer
    if (to_translate.str().size() > 0)
    {
        returnstream << convert(to_translate.str(),
                                _dicom_to_iconv.find(character_set)->second.first,
                                "UTF-8");
    }

    return returnstream.str();
}

std::string
convert_to_utf8(const std::string &input,
                const std::vector<std::string> &from,
                unsigned int position)
{
    std::vector<std::string> input_character_sets = from;
    if (from.size() == 0)
    {
        input_character_sets.push_back(""); // Default
    }
    unsigned int default_charset = 0;
    if (position == 1 && from.size() > 1)
    {
        // for PN ideographic
        default_charset = 1;
    }

    std::string default_character_set = input_character_sets[default_charset];
    std::string current_character_set = default_character_set;
    std::string next_character_set = "";
    size_t current_begin = 0;
    size_t next_escape = find_next_escape(input, current_begin,
                                          input_character_sets, next_character_set);
    size_t delta = 0;

    if (next_escape = 0)
    {
        if (next_character_set != "")
        {
            current_character_set = next_character_set;
            delta = _dicom_to_iconv.find(next_character_set)->second.second.size();
        }
    }

    std::stringstream returnstream;
    while (current_begin < input.size())
    {
        // Translate
        std::string to_translate = input.substr(current_begin,
                                                next_escape - current_begin);
        if (delta != 0 &&
            (current_character_set == "ISO 2022 IR 6" ||
             current_character_set == "") &&
            to_translate.size() >= delta)
        {
            to_translate = to_translate.substr(delta, to_translate.size() - delta);
        }

        if (to_translate.size() > 0)
        {
            if (_dicom_to_iconv.find(current_character_set) == _dicom_to_iconv.end())
            {
                std::stringstream streamerror;
                streamerror << "Could not convert to " << current_character_set
                            << ". Unkown character set.";
                throw ExceptionPACS(streamerror.str());
            }

            returnstream << convert_to_utf8_with_escape(to_translate,
                                                        current_character_set,
                                                        default_character_set);
        }

        // Modify the current character set used
        if (next_character_set != "")
        {
            current_character_set = next_character_set;
            delta = _dicom_to_iconv.find(next_character_set)->second.second.size();
        }
        else
        {
            delta = 0;
        }

        // Next
        current_begin = next_escape;
        next_escape = find_next_escape(input, current_begin+1, input_character_sets, next_character_set);
    }

    return returnstream.str();
}

bool is_control_character(char character)
{
    // see PS3.5  6.1.2.5.3 Requirements
    return (((int)character & 0xFF) < 32 && ((int)character & 0xFF) != 27);
}

size_t
find_next_escape(std::string const & input,
                 size_t begin,
                 std::vector<std::string> character_sets,
                 std::string & find_character_set)
{
    find_character_set = "";
    size_t pos = input.size();

    for (std::string const character_set : character_sets)
    {
        std::string search_pattern = _dicom_to_iconv.find(character_set)->second.second;

        if (search_pattern.size() == 0)
        {
            // no pattern => not with escape
            continue;
        }

        if (input.size() < search_pattern.size())
        {
            continue;
        }

        for (unsigned int i = begin; i <= input.size()-search_pattern.size(); ++i)
        {
            bool find = true;
            for (unsigned int j = 0; j < search_pattern.size(); ++j)
            {
                if (search_pattern[j] != input[i+j])
                {
                    find = false;
                    break;
                }
            }

            if (find && pos > i)
            {
                pos = i;
                find_character_set = character_set;
                if (character_set == "") // default
                {
                    find_character_set = "ISO 2022 IR 6"; // use default with code extensions
                }
            }
            if (find)
            {
                break;
            }
        }
    }

    return pos;
}

} // namespace characterset

} // namespace dopamine
