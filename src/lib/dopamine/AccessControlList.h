/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _33bf2cd0_3580_4b1c_8335_7accf9832f26
#define _33bf2cd0_3580_4b1c_8335_7accf9832f26

#include <string>
#include <vector>

#include <mongo/client/dbclient.h>

namespace dopamine
{

/**
 * @brief Access control list of principals, services, and optional constraints.
 */
class AccessControlList
{
public:

    /**
     * @brief ACL entry, composed of a principal name, a service name, and a
     * constraint.
     *
     * The principal name can have two special values:
     * * "*", targetting all principals
     * * "" (the empty string) to target unauthenticated users
     *
     * The service name must be one of "Echo", "Query", "Retrieve", "Store".
     *
     * The constraint acts as a filter for services returning responses.
     */
    struct Entry
    {
        std::string principal;
        std::string service;
        mongo::BSONObj constraint;

        Entry(
            std::string const & principal, std::string const & service,
            mongo::BSONObj const & constraint);
    };

    /// @brief Constructor.
    AccessControlList(
        mongo::DBClientConnection & connection,
        std::string const & database);

    /// @brief Destructor.
    ~AccessControlList();

    /// @brief Return the database name.
    std::string const & get_database() const;

    /// @brief Set the database name.
    void set_database(std::string const & database);

    /// @brief Return the access control list entries.
    std::vector<Entry> get_entries() const;

    /// @brief Set the access control list entries.
    void set_entries(std::vector<Entry> const & entries);

    /**
     * @brief Test whether the principal has at least one matching entry for the
     * service (possibly as anonymous).
     */
    bool is_allowed(
        std::string const & principal, std::string const & service) const;

    /**
     * @brief Return the constraints associated with principal and service in
     * a form that can match a DICOM data set in BSON representation.
     */
    mongo::BSONObj get_constraints(
        std::string const & principal, std::string const & service) const;

private:
    mongo::DBClientConnection & _connection;

    std::string _database;
    std::string _namespace;

    static mongo::BSONObj _get_query(
        std::string const & principal, std::string const & service);
};

}

#endif // _33bf2cd0_3580_4b1c_8335_7accf9832f26
