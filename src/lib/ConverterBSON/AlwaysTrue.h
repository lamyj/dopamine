/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _0df310f1_c98d_4b26_a02f_be846908e094
#define _0df310f1_c98d_4b26_a02f_be846908e094

#include "Condition.h"

class DcmElement;

namespace dopamine
{

namespace converterBSON
{

/**
 * @brief \class Always True Condition
 */
class AlwaysTrue : public Condition_DEBUG_RLA
{
public:
    typedef boost::shared_ptr<AlwaysTrue> Pointer;
    
    /// Create pointer to new instance of AlwaysTrue
    static Pointer New();
    
    /// Destroy the instance of AlwaysTrue
    virtual ~AlwaysTrue();

    /**
     * Operator ()
     * @param element: tested element
     * @return true
     */
    virtual bool operator()(dcmtkpp::Tag const & tag,
                            dcmtkpp::Element const & element)
            const throw(dopamine::ExceptionPACS);
    
protected:

private:
    /// Create an instance of AlwaysTrue
    AlwaysTrue();

};

} // namespace converterBSON

} // namespace dopamine

#endif // _0df310f1_c98d_4b26_a02f_be846908e094
