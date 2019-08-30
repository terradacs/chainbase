# add_executable( chainrocks_database_type_rocksdb_test "chainrocks_database_type_rocksdb_test.cpp" )
# add_executable( chainrocks_database_type_test "chainrocks_database_type_test.cpp" )
# add_executable( chainrocks_index_type_test "chainrocks_index_type_test.cpp" )
# add_executable( token_transfer_emulation_boost_interprocess_test "token_transfer_emulation_boost_interprocess_test.cpp" )
# add_executable( token_transfer_emulation_chainbase_test "token_transfer_emulation_chainbase_test.cpp" )
add_executable( token_transfer_emulation_chainbase_large_test "token_transfer_emulation_chainbase_large_test.cpp" )
# add_executable( token_transfer_emulation_rocksdb_test "token_transfer_emulation_rocksdb_test.cpp" )
# add_executable( token_transfer_emulation_rocksdb_batch_test "token_transfer_emulation_rocksdb_batch_test.cpp" )
add_executable( token_transfer_emulation_rocksdb_large_batch_test "token_transfer_emulation_rocksdb_large_batch_test.cpp" )
# add_executable( token_transfer_emulation_test "token_transfer_emulation_test.cpp" )

# target_link_libraries(
#   chainrocks_database_type_rocksdb_test
#   chainrocks Boost::unit_test_framework
#   ${ROCKSDB_LIBRARY}
#   ${OPENSSL_LIBRARIES}
#   ${PLATFORM_SPECIFIC_LIBS} )

# target_link_libraries(
#   chainrocks_database_type_test
#   chainrocks Boost::unit_test_framework
#   ${ROCKSDB_LIBRARY}
#   ${OPENSSL_LIBRARIES}
#   ${PLATFORM_SPECIFIC_LIBS} )

# target_link_libraries(
#   chainrocks_index_type_test
#   chainrocks Boost::unit_test_framework
#   ${ROCKSDB_LIBRARY}
#   ${OPENSSL_LIBRARIES}
#   ${PLATFORM_SPECIFIC_LIBS} )

# target_link_libraries(
#   token_transfer_emulation_boost_interprocess_test
#   chainbase Boost::unit_test_framework
#   ${ROCKSDB_LIBRARY}
#   ${OPENSSL_LIBRARIES}
#   ${PLATFORM_SPECIFIC_LIBS} )

# target_link_libraries(
#   token_transfer_emulation_chainbase_test
#   chainbase Boost::unit_test_framework
#   ${ROCKSDB_LIBRARY}
#   ${OPENSSL_LIBRARIES}
#   ${PLATFORM_SPECIFIC_LIBS} )

target_link_libraries(
  token_transfer_emulation_chainbase_large_test
  chainbase Boost::unit_test_framework
  ${ROCKSDB_LIBRARY}
  ${OPENSSL_LIBRARIES}
  ${PLATFORM_SPECIFIC_LIBS} )

# target_link_libraries(
#   token_transfer_emulation_rocksdb_test
#   chainrocks Boost::unit_test_framework
#   ${ROCKSDB_LIBRARY}
#   ${OPENSSL_LIBRARIES}
#   ${PLATFORM_SPECIFIC_LIBS} )

# target_link_libraries(
#   token_transfer_emulation_rocksdb_batch_test
#   chainrocks Boost::unit_test_framework
#   ${ROCKSDB_LIBRARY}
#   ${OPENSSL_LIBRARIES}
#   ${PLATFORM_SPECIFIC_LIBS} )

target_link_libraries(
  token_transfer_emulation_rocksdb_large_batch_test
  chainrocks Boost::unit_test_framework
  ${ROCKSDB_LIBRARY}
  ${OPENSSL_LIBRARIES}
  ${PLATFORM_SPECIFIC_LIBS} )

# target_link_libraries(
#   token_transfer_emulation_test
#   chainrocks Boost::unit_test_framework
#   ${ROCKSDB_LIBRARY}
#   ${OPENSSL_LIBRARIES}
#   ${PLATFORM_SPECIFIC_LIBS} )
