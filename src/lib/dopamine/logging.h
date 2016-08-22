/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _376754da_a501_4efc_a043_eb372e4e764c
#define _376754da_a501_4efc_a043_eb372e4e764c

#include <log4cpp/Category.hh>
#include <log4cpp/Priority.hh>

#define DOPAMINE_LOG(level) \
    log4cpp::Category::getInstance("dopamine") << log4cpp::Priority::level

#endif // _376754da_a501_4efc_a043_eb372e4e764c
