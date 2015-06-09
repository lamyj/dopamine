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

void initialize_logger(std::string const & priority);

log4cpp::CategoryStream get_logger(log4cpp::Priority::PriorityLevel const & level);

log4cpp::CategoryStream logger_debug();

log4cpp::CategoryStream logger_error();

log4cpp::CategoryStream logger_info();

log4cpp::CategoryStream logger_warning();

} // namespace dopamine

#endif // _99520904_5868_4a39_92fd_be0af81852ad
