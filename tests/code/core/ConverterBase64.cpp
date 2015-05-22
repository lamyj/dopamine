/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleJSONToBSON
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/test/unit_test.hpp>

#include <string>
#include <vector>

#include "core/ConverterBase64.h"

std::vector<std::string> const binaires =
{
    "",
    "M",
    "Ma",
    "Man",
    "pleasure.",
    "leasure.",
    "easure.",
    "asure.",
    "sure.",
    "ure."
};
std::vector<std::string> const base64s =
{
    "",
    "TQ==",
    "TWE=",
    "TWFu",
    "cGxlYXN1cmUu",
    "bGVhc3VyZS4=",
    "ZWFzdXJlLg==",
    "YXN1cmUu",
    "c3VyZS4=",
    "dXJlLg=="
};

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Encode
 */
BOOST_AUTO_TEST_CASE(Encode)
{
    for (unsigned int i = 0; i < binaires.size(); ++i)
    {
        std::string const binaire = binaires[i];
        std::string const base64 = dopamine::ConverterBase64::encode(binaire);

        BOOST_CHECK_EQUAL(base64, base64s[i]);
    }
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Decode
 */
BOOST_AUTO_TEST_CASE(Decode)
{
    for (unsigned int i = 0; i < base64s.size(); ++i)
    {
        std::string const base64 = base64s[i];
        std::string const binaire = dopamine::ConverterBase64::decode(base64);

        BOOST_CHECK_EQUAL(binaire, binaires[i]);
    }
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: binaire -> Base64 -> binaire
 */
BOOST_AUTO_TEST_CASE(EncodeDecode)
{
    for (std::string const binaire : binaires)
    {
        std::string const base64 = dopamine::ConverterBase64::encode(binaire);
        std::string const newbinaire = dopamine::ConverterBase64::decode(base64);

        BOOST_CHECK_EQUAL(binaire, newbinaire);
    }
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Base64 -> binaire -> Base64
 */
BOOST_AUTO_TEST_CASE(DecodeEncode)
{
    for (std::string const base64 : base64s)
    {
        std::string const binaire = dopamine::ConverterBase64::decode(base64);
        std::string const newbase64 = dopamine::ConverterBase64::encode(binaire);

        BOOST_CHECK_EQUAL(base64, newbase64);
    }
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Long String
 */
BOOST_AUTO_TEST_CASE(LongString)
{
    // Create buffer
    std::stringstream buffer;
    for (unsigned int i = 0; i < 5000; ++i)
    {
        buffer << (char)(i%255 + 1); // +1 to not add '\0'
    }

    std::string const base64 = dopamine::ConverterBase64::encode(buffer.str());
    std::string const binaire = dopamine::ConverterBase64::decode(base64);

    BOOST_CHECK_EQUAL(buffer.str(), binaire);
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Line Break
 */
BOOST_AUTO_TEST_CASE(LineBreak)
{
    // Create buffer
    std::stringstream buffer;
    for (unsigned int i = 0; i < 5000; ++i)
    {
        buffer << (char)(i%255 + 1); // +1 to not add '\0'
    }

    std::string const base64 =
            dopamine::ConverterBase64::encode(buffer.str(),
                                              dopamine::ConverterBase64::DEFAULT_LINEBREAK);
    std::string const binaire = dopamine::ConverterBase64::decode(base64);

    BOOST_CHECK_EQUAL(buffer.str(), binaire);

    // Check size of lines
    std::vector<std::string> lines;
    boost::split(lines, base64, boost::is_any_of("\n"));

    BOOST_CHECK_NE(lines.size(), 1);
    BOOST_CHECK_GT(lines[lines.size()-1].size(), 0); // check last line
    BOOST_CHECK_LT(lines[lines.size()-1].size(), dopamine::ConverterBase64::DEFAULT_LINEBREAK); // check last line
    lines.pop_back(); // erase last line (size != DEFAULT_LINEBREAK)
    for (auto line : lines)
    {
        BOOST_CHECK_EQUAL(line.size(), dopamine::ConverterBase64::DEFAULT_LINEBREAK);
    }
}
