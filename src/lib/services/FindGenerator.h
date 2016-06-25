/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _ee9915a2_a504_4b21_8d43_7938c66c526e
#define _ee9915a2_a504_4b21_8d43_7938c66c526e

#include "services/GeneratorPACS.h"

namespace dopamine
{

namespace services
{

/**
 * @brief \class Response Generator for C-FIND services.
 */
class FindGenerator : public GeneratorPACS
{
public:
    typedef FindGenerator Self;
    typedef boost::shared_ptr<Self> Pointer;

    /// Create pointer to new instance of FindGenerator
    static Pointer New();
    
    /// Destroy the find response generator
    virtual ~FindGenerator();

    virtual odil::Value::Integer initialize(
            odil::Association const & association,
            odil::message::Message const & message);

    virtual odil::Value::Integer next();

    virtual odil::Value::Integer initialize(mongo::BSONObj const & request);

    bool get_convert_modalities_in_study() const;

    void set_fuzzy_matching(bool fuzzy_matching);

    bool get_fuzzy_matching() const;

protected:
    /// Create a default find response generator
    FindGenerator();

private:

    /// flag indicating if modalities should be convert
    bool _convert_modalities_in_study;

    bool _fuzzy_matching;

};

} // namespace services

} // namespace dopamine

#endif // _ee9915a2_a504_4b21_8d43_7938c66c526e
