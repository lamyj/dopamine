/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _ac3a73f2_efa7_43bc_ba3b_5aa4a869ae80
#define _ac3a73f2_efa7_43bc_ba3b_5aa4a869ae80

#include "ConverterBSONXML.h"

namespace dopamine
{

namespace converterBSON
{

/**
 * @brief \class The XMLToBSON class
 * Convert a XML object into BSON Object
 */
class XMLToBSON : public ConverterBSONXML
{
public:
    /// Create an instance of
    XMLToBSON();

    /// Destroy the instance of
    ~XMLToBSON();

    /**
     * @brief from_ptree
     * @param tree: XML object to convert
     * @return Converted BSON Object
     */
    mongo::BSONObj from_ptree(boost::property_tree::ptree tree) const;

    /**
     * @brief from_string
     * @param xml: XML object as string to convert
     * @return Converted BSON Object
     */
    mongo::BSONObj from_string(std::string const & xml) const;

protected:

private:
    template<typename TType>
    void _add_value(boost::property_tree::ptree tree,
                    mongo::BSONArrayBuilder & array_builder,
                    unsigned int count) const;

    void _add_person_name(boost::property_tree::ptree tree,
                          mongo::BSONArrayBuilder & array_builder,
                          unsigned int count) const;

    void _add_sequence(boost::property_tree::ptree tree,
                       mongo::BSONArrayBuilder & array_builder,
                       unsigned int count) const;

    void _add_component_name(boost::property_tree::ptree tree,
                             mongo::BSONObjBuilder & object_builder,
                             std::string const & tag_name) const;

    void _add_binary_data(boost::property_tree::ptree tree,
                          mongo::BSONObjBuilder & object_builder) const;

    void _add_element(boost::property_tree::ptree tree,
                      mongo::BSONObjBuilder & builder) const;

};

} // namespace converterBSON

} // namespace dopamine

#endif // _ac3a73f2_efa7_43bc_ba3b_5aa4a869ae80
