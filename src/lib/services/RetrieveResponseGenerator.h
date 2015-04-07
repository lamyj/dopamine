/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _67fe6116_89f6_4b1c_b2b4_8046e63b1537
#define _67fe6116_89f6_4b1c_b2b4_8046e63b1537

#include "QueryRetrieveGenerator.h"
#include "services/StoreSubOperation.h"

namespace dopamine
{

namespace services
{

class RetrieveResponseGenerator : public QueryRetrieveGenerator
{
public:
    RetrieveResponseGenerator(std::string const & username);

    virtual ~RetrieveResponseGenerator();

protected:

private:

};

struct RetrieveContext
{
    RetrieveResponseGenerator* _generator;
    StoreSubOperation* _storeprovider;

    RetrieveContext(RetrieveResponseGenerator * generator,
                    StoreSubOperation * storeprovider):
        _generator(generator), _storeprovider(storeprovider) {}
};

} // namespace services

} // namespace dopamine

#endif // _67fe6116_89f6_4b1c_b2b4_8046e63b1537
