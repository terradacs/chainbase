file(GLOB CHAINBASE_TEST       "test.cpp")
file(GLOB ROCKSDB_DB_FILE_TEST "rocksdb_db_file.test.cpp")

add_executable(chainbase_test       ${CHAINBASE_TEST})
add_executable(rocksdb_db_file_test ${ROCKSDB_DB_FILE_TEST})

target_link_libraries(chainbase_test chainbase Boost::unit_test_framework ${OPENSSL_LIBRARIES} ${PLATFORM_SPECIFIC_LIBS})
target_link_libraries(rocksdb_db_file_test chainbase rocksdb Boost::unit_test_framework ${OPENSSL_LIBRARIES} ${PLATFORM_SPECIFIC_LIBS})
