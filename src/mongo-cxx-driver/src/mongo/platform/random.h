// random.h

/*    Copyright 2012 10gen Inc.
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

#pragma once

#include "mongo/platform/cstdint.h"

namespace mongo {

/**
 * Uses http://en.wikipedia.org/wiki/Xorshift
 */
class PseudoRandom {
public:
    PseudoRandom(int32_t seed);

    PseudoRandom(uint32_t seed);

    PseudoRandom(int64_t seed);

    int32_t nextInt32();

    int64_t nextInt64();

    /**
     * @return a number between 0 and max
     */
    int32_t nextInt32(int32_t max) {
        return static_cast<uint32_t>(nextInt32()) % static_cast<uint32_t>(max);
    }

    /**
     * @return a number between 0 and max
     */
    int64_t nextInt64(int64_t max) {
        return static_cast<uint64_t>(nextInt64()) % static_cast<uint64_t>(max);
    }

    /**
     * @return a number between 0 and max
     *
     * This makes PseudoRandom instances passable as the third argument to std::random_shuffle
     */
    intptr_t operator()(intptr_t max) {
        if (sizeof(intptr_t) == 4)
            return static_cast<intptr_t>(nextInt32(static_cast<int32_t>(max)));
        return static_cast<intptr_t>(nextInt64(static_cast<int64_t>(max)));
    }

private:
    void _init(uint32_t seed);

    uint32_t nextUInt32();

    uint32_t _x;
    uint32_t _y;
    uint32_t _z;
    uint32_t _w;
};

/**
 * More secure random numbers
 * Suitable for nonce/crypto
 * Slower than PseudoRandom, so only use when really need
 */
class SecureRandom {
public:
    virtual ~SecureRandom();

    virtual int64_t nextInt64() = 0;

    static SecureRandom* create();
};
}  // namespace mongo
