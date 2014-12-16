/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _99520904_5868_4a39_92fd_be0af81852ad
#define _99520904_5868_4a39_92fd_be0af81852ad

#include "log4cpp/Appender.hh"
#include "log4cpp/BasicLayout.hh"
#include "log4cpp/Category.hh"
#include "log4cpp/OstreamAppender.hh"
#include "log4cpp/Priority.hh"

namespace dopamine
{

void InitializeLogger(std::string const & priority);

log4cpp::CategoryStream getLogger(log4cpp::Priority::PriorityLevel const & level);

log4cpp::CategoryStream loggerDebug();

log4cpp::CategoryStream loggerError();

log4cpp::CategoryStream loggerInfo();

log4cpp::CategoryStream loggerWarning();

} // namespace dopamine

#endif // _99520904_5868_4a39_92fd_be0af81852ad
