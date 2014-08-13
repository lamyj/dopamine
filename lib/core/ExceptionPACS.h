/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _180213b0_abe1_4715_83bd_fb22b04d5204
#define _180213b0_abe1_4715_83bd_fb22b04d5204

#include <exception>
#include <string>

namespace research_pacs
{
    
class ExceptionPACS: public std::exception
{
public:
    ExceptionPACS(const std::string& message):
        m_message(message) {}

    ~ExceptionPACS() throw() {}
    
    virtual const char* what() const throw() {
        return m_message.c_str();
    }
    
protected:
   std::string m_message;

private:

};

} // namespace research_pacs

#endif // _180213b0_abe1_4715_83bd_fb22b04d5204

