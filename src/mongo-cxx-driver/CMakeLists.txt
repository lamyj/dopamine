include(CheckCSourceCompiles)
include(CheckCXXSourceCompiles)
include(CheckIncludeFile)
include(TestBigEndian)

include(packages_finder)

set(packages Boost SSL)
set(Boost_components filesystem regex system thread)

packages_finder("${packages}" include_dirs libraries library_dirs)

set(mongoclient_version_major 1)
set(mongoclient_version_minor 1)
set(mongoclient_version_patch 2)
configure_file("src/mongo/version.h.in" "src/mongo/version.h")

set(mongoclient_ssl "#define MONGO_SSL 1")
set(mongoclient_sasl "#define MONGO_SASL 1")

macro(set_config_variable name)
    string(TOUPPER "${name}" upper_case_name)
    if("${name}")
        set(mongoclient_${name} "#define MONGO_${upper_case_name} 1")
    else()
        set(mongoclient_${name} "// #undef MONGO_${upper_case_name}")
    endif()
endmacro()

check_include_file("unistd.h" have_header_unistd_h)
set_config_variable(have_header_unistd_h)

set(CMAKE_REQUIRED_FLAGS "-std=c++0x")
check_cxx_source_compiles("
#include <atomic>
int main(int argc, char **argv) { std::atomic<int> a(0); return a.fetch_add(1); }
" have_cxx11_atomics)
set_config_variable(have_cxx11_atomics)

#check_cxx_source_compiles("
#int main(int argc, char **argv) {
#    int a = 0; int b = 0; int c = 0;
#    __atomic_compare_exchange(&a, &b, &c, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
#    return 0;
#}" have_gcc_atomic_builtins)
#set_config_variable(have_gcc_atomic_builtins)

#check_cxx_source_compiles("int main(int argc, char **argv) {
#int a = 0;
#return __sync_fetch_and_add(&a, 1);
#}

#//
#// Figure out if we are using gcc older than 4.2 to target 32-bit x86. If so, error out
#// even if we were able to compile the __sync statement, due to
#// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=40693
#//
##if defined(__i386__)
##if !defined(__clang__)
##if defined(__GNUC__) && (__GNUC__ == 4) && (__GNUC_MINOR__ < 2)
##error \"Refusing to use __sync in 32-bit mode with gcc older than 4.2\"
##endif
##endif
##endif
#" have_gcc_sync_builtins)
#set_config_variable(have_gcc_sync_builtins)

# C++-11: use C++11 atomics
set_config_variable(have_gcc_atomic_builtins)
set_config_variable(have_gcc_sync_builtins)

test_big_endian(isbig_endian)
if(is_big_endian)
    set(mongoclient_byte_order "#define MONGO_BYTE_ORDER 4321")
else()
    set(mongoclient_byte_order "#define MONGO_BYTE_ORDER 1234")
endif()

check_cxx_source_compiles("#include <type_traits>
int main(int argc, char **argv) {
    class Trivial {
        int trivial1;
        double trivial2;
        struct {
            float trivial3;
            short trivial4;
        } trivial_member;
    };

    class NotTrivial {
        int x, y;
        NotTrivial(const NotTrivial& o) : x(o.y), y(o.x) {}
    };

    static_assert(std::is_trivially_copyable<Trivial>::value,
                  \"I should be trivially copyable\");
    static_assert(!std::is_trivially_copyable<NotTrivial>::value,
                  \"I should not be trivially copyable\");
    return 0;
}" have_std_is_trivially_copyable)
set_config_variable(have_std_is_trivially_copyable)

check_c_source_compiles("#include <string.h>
    int main() { strnlen; }" have_strnlen)
set_config_variable(have_strnlen)

configure_file(src/mongo/config.h.in src/mongo/config.h)

set(sources
    "src/mongo/base/error_codes.cpp"
    "src/mongo/base/global_initializer.cpp"
    "src/mongo/base/global_initializer_registerer.cpp"
    "src/mongo/base/init.cpp"
    "src/mongo/base/initializer.cpp"
    "src/mongo/base/initializer_context.cpp"
    "src/mongo/base/initializer_dependency_graph.cpp"
    "src/mongo/base/make_string_vector.cpp"
    "src/mongo/base/parse_number.cpp"
    "src/mongo/base/status.cpp"
    "src/mongo/base/string_data.cpp"
    "src/mongo/bson/bson_validate.cpp"
    "src/mongo/bson/bsonelement.cpp"
    "src/mongo/bson/bsonmisc.cpp"
    "src/mongo/bson/bsonobj.cpp"
    "src/mongo/bson/bsonobjbuilder.cpp"
    "src/mongo/bson/bsonobjiterator.cpp"
    "src/mongo/bson/bsontypes.cpp"
    "src/mongo/bson/oid.cpp"
    "src/mongo/bson/util/bson_extract.cpp"
    "src/mongo/client/bulk_operation_builder.cpp"
    "src/mongo/client/bulk_update_builder.cpp"
    "src/mongo/client/bulk_upsert_builder.cpp"
    "src/mongo/client/command_writer.cpp"
    "src/mongo/client/dbclient.cpp"
    "src/mongo/client/dbclient_rs.cpp"
    "src/mongo/client/dbclientcursor.cpp"
    "src/mongo/client/dbclientcursorshimarray.cpp"
    "src/mongo/client/dbclientcursorshimcursorid.cpp"
    "src/mongo/client/dbclientcursorshimtransform.cpp"
    "src/mongo/client/delete_write_operation.cpp"
    "src/mongo/client/exceptions.cpp"
    "src/mongo/client/gridfs.cpp"
    "src/mongo/client/index_spec.cpp"
    "src/mongo/client/init.cpp"
    "src/mongo/client/insert_write_operation.cpp"
    "src/mongo/client/options.cpp"
    "src/mongo/client/replica_set_monitor.cpp"
    "src/mongo/client/sasl_client_authenticate.cpp"
    "src/mongo/client/update_write_operation.cpp"
    "src/mongo/client/wire_protocol_writer.cpp"
    "src/mongo/client/write_concern.cpp"
    "src/mongo/client/write_operation_base.cpp"
    "src/mongo/client/write_result.cpp"
    "src/mongo/db/dbmessage.cpp"
    "src/mongo/db/json.cpp"
    "src/mongo/geo/coordinates2d.cpp"
    "src/mongo/geo/coordinates2dgeographic.cpp"
    "src/mongo/logger/component_message_log_domain.cpp"
    "src/mongo/logger/log_component.cpp"
    "src/mongo/logger/log_component_settings.cpp"
    "src/mongo/logger/log_manager.cpp"
    "src/mongo/logger/log_severity.cpp"
    "src/mongo/logger/logger.cpp"
    "src/mongo/logger/logstream_builder.cpp"
    "src/mongo/logger/message_event_utf8_encoder.cpp"
    "src/mongo/logger/message_log_domain.cpp"
    "src/mongo/platform/random.cpp"
    "src/mongo/platform/strnlen.cpp"
    "src/mongo/util/assert_util.cpp"
    "src/mongo/util/background.cpp"
    "src/mongo/util/base64.cpp"
    "src/mongo/util/concurrency/synchronization.cpp"
    "src/mongo/util/fail_point.cpp"
    "src/mongo/util/fail_point_registry.cpp"
    "src/mongo/util/fail_point_service.cpp"
    "src/mongo/util/hex.cpp"
    "src/mongo/util/log.cpp"
    "src/mongo/util/md5.cpp"
    "src/mongo/util/net/hostandport.cpp"
    "src/mongo/util/net/message.cpp"
    "src/mongo/util/net/message_port.cpp"
    "src/mongo/util/net/sock.cpp"
    "src/mongo/util/net/socket_poll.cpp"
    "src/mongo/util/net/ssl_manager.cpp"
    "src/mongo/util/password_digest.cpp"
    "src/mongo/util/stringutils.cpp"
    "src/mongo/util/text.cpp"
    "src/mongo/util/time_support.cpp"
    "src/mongo/util/timer.cpp"
    "src/third_party/murmurhash3/MurmurHash3.cpp"

    "src/mongo/client/native_sasl_client_session.cpp"
    "src/mongo/client/sasl_client_authenticate_impl.cpp"
    "src/mongo/client/sasl_client_conversation.cpp"
    "src/mongo/client/sasl_client_session.cpp"
    "src/mongo/client/sasl_plain_client_conversation.cpp"
    "src/mongo/client/sasl_scramsha1_client_conversation.cpp"
    "src/mongo/crypto/crypto_openssl.cpp"
    "src/mongo/crypto/mechanism_scram.cpp"

    "src/mongo/client/cyrus_sasl_client_session.cpp"
    "src/mongo/client/sasl_sspi.cpp")

set(headers
    "src/mongo/base/data_cursor.h"
    "src/mongo/base/data_view.h"
    "src/mongo/base/disallow_copying.h"
    "src/mongo/base/encoded_value_storage.h"
    "src/mongo/base/error_codes.h"
    "src/mongo/base/parse_number.h"
    "src/mongo/base/status-inl.h"
    "src/mongo/base/status.h"
    "src/mongo/base/status_with.h"
    "src/mongo/base/string_data-inl.h"
    "src/mongo/base/string_data.h"
    "src/mongo/bson/bson-inl.h"
    "src/mongo/bson/bson.h"
    "src/mongo/bson/bson_db.h"
    "src/mongo/bson/bson_field.h"
    "src/mongo/bson/bson_validate.h"
    "src/mongo/bson/bsonelement.h"
    "src/mongo/bson/bsonmisc.h"
    "src/mongo/bson/bsonobj.h"
    "src/mongo/bson/bsonobjbuilder.h"
    "src/mongo/bson/bsonobjiterator.h"
    "src/mongo/bson/bsontypes.h"
    "src/mongo/bson/inline_decls.h"
    "src/mongo/bson/oid.h"
    "src/mongo/bson/ordering.h"
    "src/mongo/bson/timestamp.h"
    "src/mongo/bson/util/builder.h"
    "src/mongo/client/autolib.h"
    "src/mongo/client/bulk_operation_builder.h"
    "src/mongo/client/bulk_update_builder.h"
    "src/mongo/client/bulk_upsert_builder.h"
    "src/mongo/client/dbclient.h"
    "src/mongo/client/dbclient_rs.h"
    "src/mongo/client/dbclientcursor.h"
    "src/mongo/client/dbclientinterface.h"
    "src/mongo/client/exceptions.h"
    "src/mongo/client/export_macros.h"
    "src/mongo/client/gridfs.h"
    "src/mongo/client/index_spec.h"
    "src/mongo/client/init.h"
    "src/mongo/client/options.h"
    "src/mongo/client/redef_macros.h"
    "src/mongo/client/sasl_client_authenticate.h"
    "src/mongo/client/undef_macros.h"
    "src/mongo/client/write_concern.h"
    "src/mongo/client/write_options.h"
    "src/mongo/client/write_result.h"
    "src/mongo/db/jsobj.h"
    "src/mongo/db/json.h"
    "src/mongo/geo/boundingbox.h"
    "src/mongo/geo/constants.h"
    "src/mongo/geo/coordinates.h"
    "src/mongo/geo/coordinates2d.h"
    "src/mongo/geo/coordinates2dgeographic.h"
    "src/mongo/geo/geometry.h"
    "src/mongo/geo/geometrycollection.h"
    "src/mongo/geo/geoobj.h"
    "src/mongo/geo/interface.h"
    "src/mongo/geo/linestring.h"
    "src/mongo/geo/multilinestring.h"
    "src/mongo/geo/multipoint.h"
    "src/mongo/geo/multipolygon.h"
    "src/mongo/geo/namespaces.h"
    "src/mongo/geo/parser-impl.h"
    "src/mongo/geo/parser.h"
    "src/mongo/geo/point.h"
    "src/mongo/geo/polygon.h"
    "src/mongo/geo/queryutils.h"
    "src/mongo/logger/appender.h"
    "src/mongo/logger/component_message_log_domain.h"
    "src/mongo/logger/labeled_level.h"
    "src/mongo/logger/log_component.h"
    "src/mongo/logger/log_component_settings.h"
    "src/mongo/logger/log_domain.h"
    "src/mongo/logger/log_manager.h"
    "src/mongo/logger/log_severity-inl.h"
    "src/mongo/logger/log_severity.h"
    "src/mongo/logger/logger.h"
    "src/mongo/logger/logstream_builder.h"
    "src/mongo/logger/message_event.h"
    "src/mongo/logger/message_log_domain.h"
    "src/mongo/platform/atomic_intrinsics.h"
    "src/mongo/platform/atomic_intrinsics_gcc_atomic.h"
    "src/mongo/platform/atomic_intrinsics_gcc_intel.h"
    "src/mongo/platform/atomic_intrinsics_gcc_sync.h"
    "src/mongo/platform/atomic_intrinsics_win32.h"
    "src/mongo/platform/atomic_word.h"
    "src/mongo/platform/atomic_word_cxx11.h"
    "src/mongo/platform/atomic_word_intrinsics.h"
    "src/mongo/platform/compiler.h"
    "src/mongo/platform/compiler_gcc.h"
    "src/mongo/platform/compiler_msvc.h"
    "src/mongo/platform/cstdint.h"
    "src/mongo/platform/endian.h"
    "src/mongo/platform/float_utils.h"
    "src/mongo/platform/hash_namespace.h"
    "src/mongo/platform/strnlen.h"
    "src/mongo/platform/unordered_map.h"
    "src/mongo/platform/windows_basic.h"
    "src/mongo/stdx/functional.h"
    "src/mongo/util/assert_util.h"
    "src/mongo/util/mongoutils/str.h"
    "src/mongo/util/net/hostandport.h"
    "src/mongo/util/net/message.h"
    "src/mongo/util/net/message_port.h"
    "src/mongo/util/net/operation.h"
    "src/mongo/util/net/sock.h"
    "src/mongo/util/shared_buffer.h"
    "src/mongo/util/time_support.h"
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_BINARY_DIR}/src
    ${include_dirs})

add_definitions(
    -pthread -pipe -Wno-deprecated-declarations
    -DMONGO_EXPOSE_MACROS -D_FILE_OFFSET_BITS=64 -DLIBMONGOCLIENT_BUILDING
    -DBOOST_THREAD_USES_DATETIME -DMONGO_HAVE_HEADER_UNISTD_H)

check_c_source_compiles("#include <openssl/crypto.h>
#include <openssl/evp.h>
int main() {}" fips_mode)
if(fips_mode)
    add_definitions(-DMONGO_HAVE_FIPS_MODE_SET)
endif()

check_c_source_compiles("#include <unistd.h>
#if !(defined(_POSIX_TIMERS) && _POSIX_TIMERS > 0)
#error POSIX clock_gettime not supported
#elif !(defined(_POSIX_MONOTONIC_CLOCK) && _POSIX_MONOTONIC_CLOCK >= 0)
#error POSIX monotonic clock not supported
#endif
int main() {}" have_posix_monotonic_clock)
if(have_posix_monotonic_clock)
    add_definitions(-DMONGO_HAVE_POSIX_MONOTONIC_CLOCK)
endif()

check_c_source_compiles("#include <time.h>
    int main() { timegm; }" have_timegm)
if(have_timegm)
    add_definitions(-DMONGO_HAVE_TIMEGM)
endif()

check_c_source_compiles("__thread int tsp_int;
int main(int argc, char* argv[]) { tsp_int = argc; return 0; }" have___thread)
if(have___thread)
    add_definitions(-DMONGO_HAVE___THREAD)
endif()

link_directories(${library_dirs})

add_library(libdopamine-mongo SHARED ${sources} ${headers})
set_target_properties(libdopamine-mongo PROPERTIES OUTPUT_NAME dopamine-mongo)
target_link_libraries(libdopamine-mongo pthread rt sasl2 ${libraries})

#install(
#    DIRECTORY dopamine/ DESTINATION include/
#    FILES_MATCHING PATTERN "*.h" PATTERN "*.txx")
install(
    TARGETS libdopamine-mongo
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin)
