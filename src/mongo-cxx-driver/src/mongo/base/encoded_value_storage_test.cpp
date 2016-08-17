/**
 *    Copyright (C) 2014 MongoDB Inc.
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

#include "mongo/base/encoded_value_storage.h"

#include <cstring>

#include "mongo/platform/endian.h"
#include "mongo/unittest/unittest.h"

namespace mongo {

// Simplistic encoded value view and value classes to test EncodedValueStorage
namespace EncodedValueStorageTest {

#pragma pack(1)
struct Layout {
    uint32_t native;
    uint32_t le;
    uint32_t be;
};
#pragma pack()

class ConstView {
public:
    typedef ConstDataView view_type;

    ConstView(const char* data) : _data(data) {}

    const char* view2ptr() const {
        return data().view();
    }

    uint32_t getNative() {
        return data().readNative<uint32_t>(offsetof(Layout, native));
    }

    uint32_t getLE() {
        return data().readLE<uint32_t>(offsetof(Layout, le));
    }

    uint32_t getBE() {
        return data().readBE<uint32_t>(offsetof(Layout, be));
    }

protected:
    const view_type& data() const {
        return _data;
    }

private:
    view_type _data;
};

class View : public ConstView {
public:
    typedef DataView view_type;

    View(char* data) : ConstView(data) {}

    using ConstView::view2ptr;
    char* view2ptr() {
        return data().view();
    }

    void setNative(uint32_t value) {
        data().writeNative(value, offsetof(Layout, native));
    }

    void setLE(uint32_t value) {
        data().writeLE(value, offsetof(Layout, le));
    }

    void setBE(uint32_t value) {
        data().writeBE(value, offsetof(Layout, be));
    }

private:
    view_type data() const {
        return const_cast<char*>(ConstView::view2ptr());
    }
};

class Value : public EncodedValueStorage<Layout, ConstView, View> {
public:
    Value() {
        BOOST_STATIC_ASSERT(sizeof(Value) == sizeof(Layout));
    }

    Value(ZeroInitTag_t zit) : EncodedValueStorage<Layout, ConstView, View>(zit) {}
};
}

TEST(EncodedValueStorage, EncodedValueStorage) {
    EncodedValueStorageTest::Value raw;
    EncodedValueStorageTest::Value zerod(kZeroInitTag);
    char buf[sizeof(EncodedValueStorageTest::Layout)] = {0};

    ASSERT_EQUALS(raw.view().view2ptr(), raw.constView().view2ptr());

    // ensure zeroing with the init tag works
    ASSERT_EQUALS(std::memcmp(zerod.view().view2ptr(), buf, sizeof(buf)), 0);

    // see if value assignment and view() works
    zerod.view().setNative(1234);
    EncodedValueStorageTest::View(buf).setNative(1234);
    raw = zerod;
    ASSERT_EQUALS(std::memcmp(raw.view().view2ptr(), buf, sizeof(buf)), 0);

    // see if view() and constView() work appropriately
    raw.view().setNative(1);
    raw.view().setLE(2);
    raw.view().setBE(3);
    ASSERT_EQUALS(static_cast<uint32_t>(1), raw.constView().getNative());
    ASSERT_EQUALS(static_cast<uint32_t>(2), raw.constView().getLE());
    ASSERT_EQUALS(static_cast<uint32_t>(3), raw.constView().getBE());
}

}  // namespace mongo
