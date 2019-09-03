add_executable(chainbase_test "chainbase_test.cpp")
target_link_libraries(
  chainbase_test
  chainbase Boost::unit_test_framework
  ${ROCKSDB_LIBRARY}
  ${OPENSSL_LIBRARIES}
  ${PLATFORM_SPECIFIC_LIBS})

add_executable(rocksdb_test "rocksdb_test.cpp")
target_link_libraries(
  rocksdb_test
  chainrocks Boost::unit_test_framework
  ${ROCKSDB_LIBRARY}
  ${OPENSSL_LIBRARIES}
  ${PLATFORM_SPECIFIC_LIBS})
