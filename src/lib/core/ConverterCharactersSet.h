/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _4ec5af69_c2f1_4fb6_b70c_d14d2ecf3fdb
#define _4ec5af69_c2f1_4fb6_b70c_d14d2ecf3fdb

#include <map>
#include <string>
#include <vector>

namespace dopamine
{

namespace characterset
{

/// @brief Generate a map from DICOM encoding to IConv encoding
static std::map<std::string, std::pair<std::string, std::string> > _create_encoding_map()
{
    std::map<std::string, std::pair<std::string, std::string> > result;

    // PS 3.3-2013, C.12.1.1.2 - Specific Character Set

    // Single-byte character sets without code extensions (PS 3.3, Table C.12-2)
    result[""] = std::make_pair("ISO-IR-6", "\u001B(B");             // Default
    result["ISO_IR 100"] = std::make_pair("ISO-IR-100", "");         // Latin alphabet No. 1
    result["ISO_IR 101"] = std::make_pair("ISO-IR-101", "");         // Latin alphabet No. 2
    result["ISO_IR 109"] = std::make_pair("ISO-IR-109", "");         // Latin alphabet No. 3
    result["ISO_IR 110"] = std::make_pair("ISO-IR-110", "");         // Latin alphabet No. 4
    result["ISO_IR 144"] = std::make_pair("ISO-IR-144", "");         // Cyrillic
    result["ISO_IR 127"] = std::make_pair("ISO-IR-127", "");         // Arabic
    result["ISO_IR 126"] = std::make_pair("ISO-IR-126", "");         // Greek
    result["ISO_IR 138"] = std::make_pair("ISO-IR-138", "");         // Hebrew
    result["ISO_IR 148"] = std::make_pair("ISO-IR-148", "");         // Latin alphabet No. 5
    result["ISO_IR 13"] = std::make_pair("ISO-2022-JP", "");         // Japanese
    result["ISO_IR 166"] = std::make_pair("ISO-IR-166", "");         // Thai

    // Single-byte character sets with code extensions (PS 3.3, Table C.12-3)
    result["ISO 2022 IR 6"] = std::make_pair("ISO-IR-6", "\u001B(B");         // Default repertoire
    result["ISO 2022 IR 100"] = std::make_pair("ISO-IR-100", "\u001B-A");     // Latin alphabet No. 1
    result["ISO 2022 IR 101"] = std::make_pair("ISO-IR-101", "\u001B-B");     // Latin alphabet No. 2
    result["ISO 2022 IR 109"] = std::make_pair("ISO-IR-109", "\u001B-C");     // Latin alphabet No. 3
    result["ISO 2022 IR 110"] = std::make_pair("ISO-IR-110", "\u001B-D");     // Latin alphabet No. 4
    result["ISO 2022 IR 144"] = std::make_pair("ISO-IR-144", "\u001B-L");     // Cyrillic
    result["ISO 2022 IR 127"] = std::make_pair("ISO-IR-127", "\u001B-G");     // Arabic
    result["ISO 2022 IR 126"] = std::make_pair("ISO-IR-126", "\u001B-F");     // Greek
    result["ISO 2022 IR 138"] = std::make_pair("ISO-IR-138", "\u001B-H");     // Hebrew
    result["ISO 2022 IR 148"] = std::make_pair("ISO-IR-148", "\u001B-M");     // Latin alphabet No. 5
    result["ISO 2022 IR 13"] = std::make_pair("Shift_JIS", "\u001B)I");       // Japanese
    result["ISO 2022 IR 166"] = std::make_pair("ISO-IR-166", "\u001B-T");     // Thai

    // Multi-byte character sets with code extensions (PS 3.3, Table C.12-4)
    result["ISO 2022 IR 87"] = std::make_pair("ISO-2022-JP", "\u001B$B");     // Japanese Kanji
    // result["ISO 2022 IR 159"]                                        // Japanese Supplementary Kanji
    result["ISO 2022 IR 149"] = std::make_pair("EUC-KR", "\u001B$)C");        // Korean Hangul, Hanja
    result["ISO 2022 IR 58"] = std::make_pair("ISO-2022-CN", "\u001B$)A"),    // Simplified Chinese GB 2312-80 China Association for Standardization

    // Multi-byte character sets without code extensions (PS 3.3, Table C.12-5)
    result["ISO_IR 192"] = std::make_pair("UTF-8", "");
    result["GB18030"] = std::make_pair("GB18030", "");
    result["GBK"] = std::make_pair("GBK", "");

    return result;
}

static const std::map<std::string, std::pair<std::string, std::string> >
    _dicom_to_iconv = _create_encoding_map();

size_t find_next_escape(std::string const & input,
                        size_t begin,
                        std::vector<std::string> character_sets,
                        std::string & find_character_set);

std::string convert(std::string const & input, std::string const & from,
                    std::string const & to);

std::string convert_to_utf8_with_escape(std::string const & input,
                                        std::string const & current_character_set,
                                        std::string const & default_character_set);

std::string convert_to_utf8(std::string const & input,
                            std::vector<std::string> const & from,
                            unsigned int position = 0);

bool is_control_character(char character);

} // namespace characterset

} // namespace dopamine

#endif // _4ec5af69_c2f1_4fb6_b70c_d14d2ecf3fdb
