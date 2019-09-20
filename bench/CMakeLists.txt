add_executable(bench-tps
  "bench_tps.cpp"
  "clocker.cpp"
  "chainbase_interface.cpp"
  "chainrocks_interface.cpp"
  "generated_data.cpp"
  "logger.cpp"
  "system_metrics.cpp")
target_link_libraries(
  bench-tps
  chainbase
  chainrocks
  Boost::filesystem
  Boost::program_options
  ${ROCKSDB_LIBRARY}
  ${OPENSSL_LIBRARIES}
  ${PLATFORM_SPECIFIC_LIBS} )
