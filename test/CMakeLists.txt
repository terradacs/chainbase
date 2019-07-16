add_executable( chainbase_test "test.cpp" )
add_executable( chainrocks_test "chainrocks_test.cpp" )
add_executable( token_transfer_emulation_test "token_transfer_emulation.cpp" )

target_link_libraries(
  chainbase_test
  chainbase Boost::unit_test_framework
  ${OPENSSL_LIBRARIES}
  ${PLATFORM_SPECIFIC_LIBS} )

target_link_libraries(
  chainrocks_test
  chainrocks Boost::unit_test_framework
  ${OPENSSL_LIBRARIES}
  ${PLATFORM_SPECIFIC_LIBS} )

target_link_libraries(
  token_transfer_emulation_test
  chainrocks Boost::unit_test_framework
  ${OPENSSL_LIBRARIES}
  ${PLATFORM_SPECIFIC_LIBS} )
