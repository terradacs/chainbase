#include <iostream>
#include <boost/test/unit_test.hpp>
#include <chainrocks/chainrocks.hpp>

static const chainrocks::key keys[6]{0ULL,1ULL,2ULL,3ULL,4ULL,5ULL};
static const chainrocks::value values[6]{ {'a','b','c'},{'a','c','b'},{'b','a','c'},{'b','c','a'}, {'c','a','b'},{'c','b','a'} };

/**
 * This test Simply tests the basic functionality of chainrocks.
 */
BOOST_AUTO_TEST_CASE(chainrocks_basics) {
   boost::filesystem::path temp{boost::filesystem::unique_path()};
   std::cerr << temp.native() << " \n";
   try {
      //////////////////////////////////////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////////////////////////

      chainrocks::rocksdb_database db{temp};

      static std::string test_value;
      
      //////////////////////////////////////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////////////////////////

      {
      db.create(keys[0], values[0]);
      db.get(keys[0], test_value);
      BOOST_REQUIRE_EQUAL( (test_value), (chainrocks::value{'a','b','c'}) );
      }

      //////////////////////////////////////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////////////////////////

      {
      db.remove(keys[0]);
      db.get(keys[0], test_value);
      BOOST_REQUIRE_EQUAL( (test_value), (chainrocks::value{}) );
      }
      
      //////////////////////////////////////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////////////////////////

      {
      db.create(keys[0], values[0]);
      db.modify(keys[0], values[1]);
      db.get(keys[0], test_value);
      BOOST_REQUIRE_EQUAL( (test_value), (chainrocks::value{'a','b','c'}) );
      }

      //////////////////////////////////////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////////////////////////
      
      {
         chainrocks::session session{db.start_undo_session(true)};
         db.modify(keys[0], values[1]);

         db.get(keys[0], test_value);
         BOOST_REQUIRE_EQUAL( (test_value), (chainrocks::value{'a','c','b'}) );
      }
      db.get(keys[0], test_value);
      BOOST_REQUIRE_EQUAL( (test_value), (chainrocks::value{'a','b','c'}) );

      //////////////////////////////////////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////////////////////////

      {
         chainrocks::session session{db.start_undo_session(true)};

         db.get(keys[0], test_value);
         BOOST_REQUIRE_EQUAL( (test_value), (chainrocks::value{'a','b','c'}) );
         db.get(keys[1], test_value);
         BOOST_REQUIRE_EQUAL( (test_value), (chainrocks::value{}) );
         db.get(keys[2], test_value);
         BOOST_REQUIRE_EQUAL( (test_value), (chainrocks::value{}) );
         db.get(keys[3], test_value);
         BOOST_REQUIRE_EQUAL( (test_value), (chainrocks::value{}) );
         db.get(keys[4], test_value);
         BOOST_REQUIRE_EQUAL( (test_value), (chainrocks::value{}) );
         db.get(keys[5], test_value);
         BOOST_REQUIRE_EQUAL( (test_value), (chainrocks::value{}) );

         db.create(keys[0], values[0]);
         db.create(keys[1], values[1]);
         db.create(keys[2], values[2]);
         db.create(keys[3], values[3]);
         db.create(keys[4], values[4]);
         db.create(keys[5], values[5]);

         db.get(keys[0], test_value);
         BOOST_REQUIRE_EQUAL( (test_value), (chainrocks::value{'a','b','c'}) );
         db.get(keys[1], test_value);
         BOOST_REQUIRE_EQUAL( (test_value), (chainrocks::value{'a','c','b'}) );
         db.get(keys[2], test_value);
         BOOST_REQUIRE_EQUAL( (test_value), (chainrocks::value{'b','a','c'}) );
         db.get(keys[3], test_value);
         BOOST_REQUIRE_EQUAL( (test_value), (chainrocks::value{'b','c','a'}) );
         db.get(keys[4], test_value);
         BOOST_REQUIRE_EQUAL( (test_value), (chainrocks::value{'c','a','b'}) );
         db.get(keys[5], test_value);
         BOOST_REQUIRE_EQUAL( (test_value), (chainrocks::value{'c','b','a'}) );
         
         session.push();
      }

      db.get(keys[0], test_value);
      BOOST_REQUIRE_EQUAL( (test_value), (chainrocks::value{'a','b','c'}) );
      db.get(keys[1], test_value);
      BOOST_REQUIRE_EQUAL( (test_value), (chainrocks::value{'a','c','b'}) );
      db.get(keys[2], test_value);
      BOOST_REQUIRE_EQUAL( (test_value), (chainrocks::value{'b','a','c'}) );
      db.get(keys[3], test_value);
      BOOST_REQUIRE_EQUAL( (test_value), (chainrocks::value{'b','c','a'}) );
      db.get(keys[4], test_value);
      BOOST_REQUIRE_EQUAL( (test_value), (chainrocks::value{'c','a','b'}) );
      db.get(keys[5], test_value);
      BOOST_REQUIRE_EQUAL( (test_value), (chainrocks::value{'c','b','a'}) );
      
      db.undo();
      
      db.get(keys[0], test_value);
      BOOST_REQUIRE_EQUAL( (test_value), (chainrocks::value{'a','b','c'}) );
      db.get(keys[1], test_value);
      BOOST_REQUIRE_EQUAL( (test_value), (chainrocks::value{}) );
      db.get(keys[2], test_value);
      BOOST_REQUIRE_EQUAL( (test_value), (chainrocks::value{}) );
      db.get(keys[3], test_value);
      BOOST_REQUIRE_EQUAL( (test_value), (chainrocks::value{}) );
      db.get(keys[4], test_value);
      BOOST_REQUIRE_EQUAL( (test_value), (chainrocks::value{}) );
      db.get(keys[5], test_value);
      BOOST_REQUIRE_EQUAL( (test_value), (chainrocks::value{}) );

      //////////////////////////////////////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////////////////////////
   }
   catch (...) {
      bfs::remove_all(temp);
      throw;
   }
}

/**
 * This test is for testing how chainrocks will react during extreme circumstances of undo sessions.
 */
BOOST_AUTO_TEST_CASE(chainrocks_extreme) {
   boost::filesystem::path temp{boost::filesystem::unique_path()};
   std::cerr << temp.native() << " \n";
   try {
      
   }
   catch (...) {
      bfs::remove_all(temp);
      throw;
   }
}
