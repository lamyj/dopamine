/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _a764d5b8_42ae_4f90_9ec2_cf377e3015a8
#define _a764d5b8_42ae_4f90_9ec2_cf377e3015a8

#include <string>

#include <mongo/client/dbclient.h>

#include <odil/DataSet.h>

namespace dopamine
{

namespace archive
{

/// @brief Data set storage.
class Storage
{
public:
    /// @brief Constructor.
    Storage(
        mongo::DBClientConnection & connection,
        std::string const & database, std::string const & bulk_database="");

    /// @brief Return the name of the main database.
    std::string const & get_database() const;

    /// @brief Set the name of the principal database.
    void set_database(std::string const & database);

    /// @brief Return the name of the bulk database or "" if none is used.
    std::string const & get_bulk_database() const;

    /// @brief Set the name of the bulk database or none if "" is passed.
    void set_bulk_database(std::string const & bulk_database);

    /// @brief Return the minimum size for a data set to be stored in GridFS.
    unsigned int get_gridfs_limit() const;

    /**
     * @brief Set the minimum size for a data set to be stored in GridFS,
     * default to 16MB.
     */
    void set_gridfs_limit(unsigned int limit);

    /// @brief Store the data set.
    void store(odil::DataSet const & data_set);

    /**
     * @brief Return the data set with given SOP instance UID; throw an
     * exception if no such data set is stored.
     */
    odil::DataSet retrieve(std::string const & sop_instance_uid) const;

private:
    mongo::DBClientConnection & _connection;
    std::string _database;
    std::string _bulk_database;
    unsigned int _gridfs_limit;
};

} // namespace archive

} // namespace dopamine

#endif // _a764d5b8_42ae_4f90_9ec2_cf377e3015a8
