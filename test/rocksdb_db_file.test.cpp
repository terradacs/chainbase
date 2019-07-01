#define BOOST_TEST_MODULE rocksdb_db_file test

#include <boost/test/unit_test.hpp>
#include <chainbase/chainbase.hpp>

#include <iostream>
#include <string>

using namespace chainbase;

static const std::string keys[6]  {"0ULL", "1ULL", "2ULL", "3ULL", "4ULL", "5ULL"};
static const std::string values[6]{"abc" , "acb" , "bac" , "bca" , "cab" , "cba" };

BOOST_AUTO_TEST_CASE(basic_functionality) {
   boost::filesystem::path temp{boost::filesystem::unique_path()};
   std::cerr << temp.native() << " \n";
   try {
      //////////////////////////////////////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////////////////////////

      chainbase::database db(temp, database::read_write, 1024*1024*8);
      // static std::string test_value; // Get will not return data if nothing found (use optional?)

      //////////////////////////////////////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////////////////////////

      {
      std::string test_value;
      db.put(keys[0], values[0]);
      db.get(keys[0], test_value);
      BOOST_REQUIRE_EQUAL( test_value, std::string{"abc"} );
      }

      //////////////////////////////////////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////////////////////////

      {
      std::string test_value;
      db.erase(keys[0]);
      db.get(keys[0], test_value);
      BOOST_REQUIRE_EQUAL( test_value, std::string{} );
      }

      //////////////////////////////////////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////////////////////////
   }
   catch (...) {
      bfs::remove_all(temp);
      throw;
   }
}
