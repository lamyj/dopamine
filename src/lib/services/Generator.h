/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _7cbe5cf8_10a1_4e5c_ac53_44e38a2eeebf
#define _7cbe5cf8_10a1_4e5c_ac53_44e38a2eeebf

#include <boost/shared_ptr.hpp>

#include <dcmtkpp/DcmtkAssociation.h>
#include <dcmtkpp/DataSet.h>
#include <dcmtkpp/message/Message.h>
#include <dcmtkpp/Value.h>

namespace dopamine
{

namespace services
{

/**
 * @brief \class The Generator class
 * Base class for all response generator
 */
class Generator
{
public:
    typedef Generator Self;
    typedef boost::shared_ptr<Self> Pointer;

    /// @brief Default constructor.
    Generator();

    /// @brief Destructor.
    virtual ~Generator();

    virtual dcmtkpp::Value::Integer initialize(
            dcmtkpp::DcmtkAssociation const & association,
            dcmtkpp::message::Message const & message) = 0;

    virtual bool done() const = 0;

    virtual dcmtkpp::Value::Integer next() = 0;

    virtual std::pair<dcmtkpp::DataSet, dcmtkpp::DataSet> get() const;

    virtual std::pair<std::string, int> get_peer_information(std::string const & ae_title) = 0;

protected:
    dcmtkpp::DataSet _meta_information;
    dcmtkpp::DataSet _current_dataset;

};

} // namespace services

} // namespace dopamine

#endif // _7cbe5cf8_10a1_4e5c_ac53_44e38a2eeebf
