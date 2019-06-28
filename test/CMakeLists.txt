file(GLOB UNIT_TESTS "*.cpp" )

add_executable(chainrocks_test ${UNIT_TESTS})

target_link_libraries(chainrocks_test chainrocks Boost::unit_test_framework ${OPENSSL_LIBRARIES} ${PLATFORM_SPECIFIC_LIBS})

