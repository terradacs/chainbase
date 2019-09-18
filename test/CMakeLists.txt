add_executable(bench-tps "bench_tps.cpp")
target_link_libraries(
  bench-tps
  chainbase
  chainrocks
  Boost::program_options
  Boost::unit_test_framework
  ${ROCKSDB_LIBRARY}
  ${OPENSSL_LIBRARIES}
  ${PLATFORM_SPECIFIC_LIBS})

add_executable(rocksdb_test "rocksdb_test.cpp")
target_link_libraries(
  rocksdb_test
  chainrocks
  Boost::unit_test_framework
  ${ROCKSDB_LIBRARY}
  ${OPENSSL_LIBRARIES}
  ${PLATFORM_SPECIFIC_LIBS})
