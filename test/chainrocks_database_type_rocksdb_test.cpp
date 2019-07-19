#define BOOST_TEST_MODULE chainrocks_database_type_rocksdb_test

#include <iostream> // std::cout

#include <boost/test/unit_test.hpp>
#include <chainbase/chainrocks.hpp>

/// Test data.
static const uint64_t keys1[10]{ 0ULL, 1ULL, 2ULL, 3ULL, 4ULL, 5ULL, 6ULL, 7ULL, 8ULL, 9ULL};
static const std::string values1[10]{"a","b","c","d","e","f","g","h","i","j"};
static const std::string values2[10]{"k","l","m","n","o","p","q","r","s","t"};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/// Testing `rocksdb` functionality

BOOST_AUTO_TEST_CASE(test_one) {
   chainrocks::database database{"/Users/john.debord/chainbase2/build/test/data"};
   
   for (size_t i{0}; i < 10; ++i) {
      database.rocksdb_put(keys1[i], values1[i]);
   }

   for (size_t i{}; i < 10; ++i) {
      BOOST_TEST_REQUIRE( (database.rocksdb_does_key_exist(keys1[i])) == (true) );
   }

   boost::filesystem::remove_all("/Users/john.debord/chainbase2/build/test/data");
}

BOOST_AUTO_TEST_CASE(test_two) {
   chainrocks::database database{"/Users/john.debord/chainbase2/build/test/data"};
   
   for (size_t i{0}; i < 10; ++i) {
      database.rocksdb_put(keys1[i], values1[i]);
   }

   for (size_t i{0}; i < 10; ++i) {
      database.rocksdb_remove(keys1[i]);
   }

   for (size_t i{}; i < 10; ++i) {
      BOOST_TEST_REQUIRE( (database.rocksdb_does_key_exist(keys1[i])) == (false) );
   }

   boost::filesystem::remove_all("/Users/john.debord/chainbase2/build/test/data");
}

BOOST_AUTO_TEST_CASE(test_three) {
   chainrocks::database database{"/Users/john.debord/chainbase2/build/test/data"};
   
   for (size_t i{0}; i < 10; ++i) {
      database.rocksdb_put(keys1[i], values1[i]);
   }

   {
   std::string value;
   for (size_t i{}; i < 10; ++i) {
      database.rocksdb_get(keys1[i], value);
      BOOST_TEST_REQUIRE( (value) == (values1[i]) );
   }
   }

   for (size_t i{0}; i < 10; ++i) {
      database.rocksdb_remove(keys1[i]);
   }

   {
   std::string value;
   for (size_t i{}; i < 10; ++i) {
      database.rocksdb_get(keys1[i], value);
      BOOST_TEST_REQUIRE( (value) == ("") );
   }
   }

   boost::filesystem::remove_all("/Users/john.debord/chainbase2/build/test/data");
BOOST_AUTO_TEST_SUITE_END()
