project(ChainRocks)
cmake_minimum_required(VERSION 3.5)

set(CMAKE_EXPORT_COMPILE_COMMANDS "ON")

SET(Boost_USE_STATIC_LIBS ON CACHE STRING "ON or OFF")

include(GNUInstallDirs)

FIND_PACKAGE(Boost 1.57 REQUIRED COMPONENTS filesystem unit_test_framework)

SET(PLATFORM_LIBRARIES)

if(CMAKE_CXX_STANDARD EQUAL 98)
   message(FATAL_ERROR "Chainrocks requires c++14 or newer")
elseif(NOT CMAKE_CXX_STANDARD)
   set(CMAKE_CXX_STANDARD 14)
   set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

if(APPLE)
  message( STATUS "Configuring ChainRocks on OS X" )
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-conversion" )
else(APPLE)
  message(STATUS "Configuring ChainRocks on Linux")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
  if(FULL_STATIC_BUILD)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++ -static-libgcc")
  endif(FULL_STATIC_BUILD)
  LIST(APPEND PLATFORM_LIBRARIES pthread)
endif(APPLE)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-builtin-memcmp")
endif()

if("${CMAKE_GENERATOR}" STREQUAL "Ninja")
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcolor-diagnostics")
  endif()
endif()

# based on http://www.delorie.com/gnu/docs/gdb/gdb_70.html
# uncomment this line to tell GDB about macros (slows compile times)
# set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -gdwarf-2 -g3" )

set(ENABLE_COVERAGE_TESTING FALSE CACHE BOOL "Build ChainRocks for code coverage analysis")

if(ENABLE_COVERAGE_TESTING)
    SET(CMAKE_CXX_FLAGS "--coverage ${CMAKE_CXX_FLAGS}")
endif()


file(GLOB HEADERS "include/chainrocks/*.hpp")
add_library(chainrocks ${HEADERS})
target_link_libraries(chainrocks Boost::filesystem ${PLATFORM_LIBRARIES} -lrocksdb)
target_include_directories(chainrocks PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

add_subdirectory(test)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/chainrocks DESTINATION ${CMAKE_INSTALL_FULL_INCLUDEDIR})

install(TARGETS chainrocks
   LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR} 
ARCHIVE DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR})

