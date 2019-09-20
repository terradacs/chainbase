add_executable(bench-tps "bench_tps.cpp")
target_link_libraries(
  bench-tps
  chainbase
  chainrocks
  Boost::filesystem
  Boost::program_options
  ${ROCKSDB_LIBRARY}
  ${OPENSSL_LIBRARIES}
  ${PLATFORM_SPECIFIC_LIBS} )
