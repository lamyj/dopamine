/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _118bbc30_77c6_4065_b8c9_056bf7b8ad43
#define _118bbc30_77c6_4065_b8c9_056bf7b8ad43

#include "core/ExceptionPACS.h"

namespace dopamine
{

namespace services
{

/**
 * @brief Base class for PACS Exception
 */
class WebServiceException: public ExceptionPACS
{
public:
    /**
     * Create an exception
     * @param message: content of the exception
     */
    WebServiceException(int status, std::string const & statusmessage,
                        std::string const & message = ""):
        ExceptionPACS(message), _status(status), _statusmessage(statusmessage) {}

    /// Destroy the exception
    ~WebServiceException() throw() {}

    /**
     * Get exception Status
     * @return status
     */
    virtual const int status() const throw() {
        return this->_status;
    }

    /**
     * Get exception Status Message
     * @return status description
     */
    virtual const std::string statusmessage() const throw() {
        return this->_statusmessage;
    }

protected:
    /// HTTP Exception Status
    int _status;

    /// HTTP Exception Status description
    std::string _statusmessage;

private:

};

} // namespace services

} // namespace dopamine

#endif // _118bbc30_77c6_4065_b8c9_056bf7b8ad43

