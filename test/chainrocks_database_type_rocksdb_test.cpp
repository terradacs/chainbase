#define BOOST_TEST_MODULE chainrocks_database_type_rocksdb_test

#include <boost/test/unit_test.hpp>
#include <chainbase/chainrocks.hpp>

struct database_fixture {
   database_fixture() : database{"/Users/john.debord/chainbase/build/test/data"}
   {
   }

   ~database_fixture() {
      boost::filesystem::remove_all("/Users/john.debord/chainbase/build/test/data");
   }
   
   chainrocks::database database;
};

/// Test data.
static const uint64_t keys0[10]{ 0ULL, 1ULL, 2ULL, 3ULL, 4ULL, 5ULL, 6ULL, 7ULL, 8ULL, 9ULL};
static const std::string values0[10]{"a","b","c","d","e","f","g","h","i","j"};

///////////////////////////////////
/// Testing `rocksdb` functionality

BOOST_FIXTURE_TEST_CASE(test_one, database_fixture) {
   for (size_t i{0}; i < 10; ++i) {
      database.rocksdb_put(keys0[i], values0[i]);
   }

   for (size_t i{}; i < 10; ++i) {
      BOOST_TEST_REQUIRE( (database.rocksdb_does_key_exist(keys0[i])) == (true) );
   }
}

BOOST_FIXTURE_TEST_CASE(test_two, database_fixture) {
   for (size_t i{0}; i < 10; ++i) {
      database.rocksdb_put(keys0[i], values0[i]);
   }

   for (size_t i{0}; i < 10; ++i) {
      database.rocksdb_remove(keys0[i]);
   }

   for (size_t i{}; i < 10; ++i) {
      BOOST_TEST_REQUIRE( (database.rocksdb_does_key_exist(keys0[i])) == (false) );
   }
}

BOOST_FIXTURE_TEST_CASE(test_three, database_fixture) {
   for (size_t i{0}; i < 10; ++i) {
      database.rocksdb_put(keys0[i], values0[i]);
   }

   {
   std::string value;
   for (size_t i{}; i < 10; ++i) {
      database.rocksdb_get(keys0[i], value);
      BOOST_TEST_REQUIRE( (value) == (values0[i]) );
   }
   }

   for (size_t i{0}; i < 10; ++i) {
      database.rocksdb_remove(keys0[i]);
   }

   {
   std::string value;
   for (size_t i{}; i < 10; ++i) {
      database.rocksdb_get(keys0[i], value);
      BOOST_TEST_REQUIRE( (value) == ("") );
   }
   }
BOOST_AUTO_TEST_SUITE_END()
