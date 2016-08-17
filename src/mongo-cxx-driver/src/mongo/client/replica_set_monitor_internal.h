/*    Copyright 2014 MongoDB Inc.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 * This is an internal header.
 * This should only be included by replica_set_monitor.cpp and replica_set_monitor_test.cpp.
 * This should never be included by any header.
 */

#pragma once

#include <boost/thread/condition_variable.hpp>
#include <deque>
#include <set>
#include <string>
#include <vector>

#include "mongo/base/disallow_copying.h"
#include "mongo/client/dbclient_rs.h"  // for TagSet and ReadPreferenceSettings
#include "mongo/client/replica_set_monitor.h"
#include "mongo/db/jsobj.h"
#include "mongo/platform/cstdint.h"
#include "mongo/platform/random.h"
#include "mongo/platform/unordered_map.h"
#include "mongo/util/net/hostandport.h"

namespace mongo {
// Internal connection cache for isMaster. Used to amortize the cost of establishing connections
// to a remote host. Not really a pool because we only keep one connection per host.
class ConnectionCache {
public:
    // Uses a cached connection (may need to reconnect) to execute a remote isMaster call
    // on the given host. Returns microseconds taken for remote call to complete. Throws.
    uint64_t timedIsMaster(const HostAndPort& host, BSONObj* out);

private:
    typedef unordered_map<HostAndPort, boost::shared_ptr<DBClientConnection> > ConnectionMap;

    // Guard that creates a connection if needed, or removes it from
    // the cache. This is so multiple isMaster calls can be in flight in the same
    // set simultaneously.
    class ConnectionGuard {
    public:
        // Lifetime of guard must be subset of lifetime of host
        ConnectionGuard(ConnectionCache& cache, const HostAndPort& host)
            : _cache(cache), _host(host) {
            // this could throw.
            _conn = _cache.getConnectionTo(_host);
        }

        ~ConnectionGuard() {
            _cache.returnConnection(_host, _conn);
        }

        boost::shared_ptr<DBClientConnection> _conn;
        // Reference to owner.
        ConnectionCache& _cache;
        const HostAndPort& _host;
    };

    // If we don't have a connection to this host cached,
    // or we have one that is dead, we reconnect and return
    // If we have one, we remove it from the cache and return it
    boost::shared_ptr<DBClientConnection> getConnectionTo(const HostAndPort& host);

    void returnConnection(const HostAndPort& host, boost::shared_ptr<DBClientConnection> conn);

    inline bool connectionOk(const boost::shared_ptr<DBClientConnection>& conn) {
        // checks are ordered from cheap to expensive
        return conn && !conn->isFailed() && conn->isStillConnected();
    }

    ConnectionMap _cacheStorage;
    boost::mutex _cacheLock;
};

struct ReplicaSetMonitor::IsMasterReply {
    IsMasterReply() : ok(false) {}
    IsMasterReply(const HostAndPort& host, int64_t latencyMicros, const BSONObj& reply)
        : ok(false), host(host), latencyMicros(latencyMicros) {
        parse(reply);
    }

    /**
     * Never throws. If parsing fails for any reason, sets ok to false.
     */
    void parse(const BSONObj& obj);

    bool ok;      // if false, ignore all other fields
    BSONObj raw;  // Always owned. Other fields are allowed to be a view into this.
    std::string setName;
    bool isMaster;
    bool secondary;
    bool hidden;
    OID electionId;                     // Set if this isMaster reply is from the primary
    HostAndPort primary;                // empty if not present
    std::set<HostAndPort> normalHosts;  // both "hosts" and "passives"
    BSONObj tags;

    // remaining fields aren't in isMaster reply, but are known to caller.
    HostAndPort host;
    int64_t latencyMicros;  // ignored if negative
};

struct ReplicaSetMonitor::SetState {
    MONGO_DISALLOW_COPYING(SetState);

public:
    // A single node in the replicaSet
    struct Node {
        explicit Node(const HostAndPort& host) : host(host), latencyMicros(unknownLatency) {
            markFailed();
        }

        void markFailed() {
            isUp = false;
            isMaster = false;
        }

        bool matches(const ReadPreference& pref) const;

        /**
         * Checks if the given tag matches the tag attached to this node.
         *
         * Example:
         *
         * Tag of this node: { "dc": "nyc", "region": "na", "rack": "4" }
         *
         * match: {}
         * match: { "dc": "nyc", "rack": 4 }
         * match: { "region": "na", "dc": "nyc" }
         * not match: { "dc": "nyc", "rack": 2 }
         * not match: { "dc": "sf" }
         */
        bool matches(const BSONObj& tag) const;

        /**
         * Updates this Node based on information in reply. The reply must be from this host.
         */
        void update(const IsMasterReply& reply);

        // Intentionally chosen to compare worse than all known latencies.
        static const int64_t unknownLatency;  // = numeric_limits<int64_t>::max()

        HostAndPort host;
        bool isUp;
        bool isMaster;          // implies isUp
        int64_t latencyMicros;  // unknownLatency if unknown
        BSONObj tags;           // owned
    };
    typedef std::vector<Node> Nodes;

    /**
     * seedNodes must not be empty
     */
    SetState(StringData name, const std::set<HostAndPort>& seedNodes);

    /**
     * Returns a host matching criteria or an empty host if no known host matches.
     *
     * Note: Uses only local data and does not go over the network.
     */
    HostAndPort getMatchingHost(const ReadPreferenceSetting& criteria) const;

    /**
     * Returns the Node with the given host, or NULL if no Node has that host.
     */
    Node* findNode(const HostAndPort& host);

    /**
     * Returns the Node with the given host, or creates one if no Node has that host.
     * Maintains the sorted order of nodes.
     */
    Node* findOrCreateNode(const HostAndPort& host);

    void updateNodeIfInNodes(const IsMasterReply& reply);

    std::string getServerAddress() const;

    /**
     * Before unlocking, do DEV checkInvariants();
     */
    void checkInvariants() const;

    static ConfigChangeHook configChangeHook;

    boost::mutex mutex;  // must hold this to access any other member or method (except name).

    // If Refresher::getNextStep returns WAIT, you should wait on the condition_variable,
    // releasing mutex. It will be notified when either getNextStep will return something other
    // than WAIT, or a new host is available for consideration by getMatchingHost. Essentially,
    // this will be hit whenever the _refreshUntilMatches loop has the potential to make
    // progress.
    // TODO consider splitting cv into two: one for when looking for a master, one for all other
    // cases.
    boost::condition_variable cv;

    const std::string name;  // safe to read outside lock since it is const
    int consecutiveFailedScans;
    std::set<HostAndPort> seedNodes;  // updated whenever a master reports set membership changes
    OID maxElectionId;                // largest election id observed by this ReplicaSetMonitor
    HostAndPort lastSeenMaster;  // empty if we have never seen a master. can be same as current
    Nodes nodes;                 // maintained sorted and unique by host
    ScanStatePtr currentScan;    // NULL if no scan in progress
    int64_t latencyThresholdMicros;
    mutable PseudoRandom rand;  // only used for host selection to balance load
    mutable int roundRobin;     // used when useDeterministicHostSelection is true
    mutable ConnectionCache connectionCache;
};

struct ReplicaSetMonitor::ScanState {
    MONGO_DISALLOW_COPYING(ScanState);

public:
    ScanState() : foundUpMaster(false), foundAnyUpNodes(false) {}

    /**
     * Adds all hosts in container that aren't in triedHosts to hostsToScan, then shuffles the
     * queue.
     */
    template <typename Container>
    void enqueAllUntriedHosts(const Container& container, PseudoRandom& rand);

    // Access to fields is guarded by associated SetState's mutex.
    bool foundUpMaster;
    bool foundAnyUpNodes;
    std::deque<HostAndPort> hostsToScan;  // Work queue.
    std::set<HostAndPort> possibleNodes;  // Nodes reported by non-primary hosts.
    std::set<HostAndPort> waitingFor;     // Hosts we have dispatched but haven't replied yet.
    std::set<HostAndPort> triedHosts;     // Hosts that have been returned from getNextStep.

    // All responses go here until we find a master.
    typedef std::vector<IsMasterReply> UnconfirmedReplies;
    UnconfirmedReplies unconfirmedReplies;
};
}
