/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "LoggerPACS.h"

#include <iostream>
#include <string>

#include <log4cpp/Appender.hh>
#include <log4cpp/BasicLayout.hh>
#include <log4cpp/Category.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/Priority.hh>

#include "core/ExceptionPACS.h"

namespace dopamine
{

void initialize_logger(
    std::string const & priority, std::string const & destination,
    std::string const & path)
{
    log4cpp::Appender * appender;
    if(destination == "console")
    {
        appender = new log4cpp::OstreamAppender("console", &std::cout);
    }
    else if(destination == "file")
    {
        appender = new log4cpp::FileAppender("TODO", path);
    }
    else
    {
        throw ExceptionPACS("Unknown log destination");
    }

    appender->setLayout(new log4cpp::BasicLayout());

    log4cpp::Category& root = log4cpp::Category::getRoot();

    if (priority == "ERROR")
    {
        root.setPriority(log4cpp::Priority::ERROR);
    }
    else if (priority == "WARNING")
    {
        root.setPriority(log4cpp::Priority::WARN);
    }
    else if (priority == "INFO")
    {
        root.setPriority(log4cpp::Priority::INFO);
    }
    else
    {
        root.setPriority(log4cpp::Priority::DEBUG);
    }

    root.addAppender(appender);
}

log4cpp::CategoryStream
get_logger(log4cpp::Priority::PriorityLevel const & level)
{
    log4cpp::Category& root = log4cpp::Category::getRoot();

    return root << level;
}

log4cpp::CategoryStream
logger_debug()
{
    return get_logger(log4cpp::Priority::DEBUG);
}

log4cpp::CategoryStream
logger_error()
{
    return get_logger(log4cpp::Priority::ERROR);
}

log4cpp::CategoryStream
logger_info()
{
    return get_logger(log4cpp::Priority::INFO);
}

log4cpp::CategoryStream
logger_warning()
{
    return get_logger(log4cpp::Priority::WARN);
}

} // namespace dopamine
