/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "fixtures/CSV.h"

#include <cstdio>
#include <fstream>
#include <string>

namespace fixtures
{

CSV
::CSV()
: filename("./tmp_test_moduleAuthenticatorCSV.csv")
{
    std::ofstream myfile;
    myfile.open(filename);
    myfile << "user1\tpassword1\n";
    myfile << "user2\tpassword2\n";
    myfile.close();
}

CSV
::~CSV()
{
    std::remove(filename.c_str());
}

} // namespace fixtures
