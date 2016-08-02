/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _687adb1f_d7ad_4b06_ad4f_4b0a45995eb7
#define _687adb1f_d7ad_4b06_ad4f_4b0a45995eb7

#include <exception>
#include <string>

namespace dopamine
{

/// @brief Base class for dopamine exceptions.
class Exception: public std::exception
{
public:
    /// @brief Message string constructor.
    Exception(std::string const & message="");

    /// @brief Destructor.
    virtual ~Exception() noexcept;

    /// @brief Return the reason for the exception.
    virtual const char* what() const noexcept;

protected:
    /// @brief Message of the exception.
    std::string _message;
};

}

#endif // _687adb1f_d7ad_4b06_ad4f_4b0a45995eb7
