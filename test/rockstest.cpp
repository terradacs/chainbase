#include <iostream>
#include <boost/test/unit_test.hpp>
#include <chainbase/chainrocks.hpp>

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
      
      //////////////////////////////////////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////////////////////////

      {
      db.create(keys[0], values[0]);

      const value& value0{db.get(keys[0])};

      BOOST_REQUIRE_EQUAL(value0, {'a','b','c'});
      }

      //////////////////////////////////////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////////////////////////

      {
      db.remove(keys[0]);

      const value& value0{db.get(keys[0])};

      BOOST_REQUIRE_EQUAL(value0, {});
      }
      
      //////////////////////////////////////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////////////////////////

      {
      db.create(keys[0], values[0]);
      db.modify(keys[0], values[1]);

      const value& value0{db.get(keys[0])};
      
      BOOST_REQUIRE_EQUAL(value0, {'a','b','c'});
      }

      //////////////////////////////////////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////////////////////////
      
      {
         chainrocks::session session{db.start_undo_session(true)};
         db.modify(keys[0], values[1]);

         const value& value0{db.get(keys[0])};

         BOOST_REQUIRE_EQUAL(value0, {'a','c','b'});
      }
      BOOST_REQUIRE_EQUAL(value0, {'a','c','b'});

      //////////////////////////////////////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////////////////////////

      {
         chainrocks::session session{db.start_undo_session(true)};

         BOOST_REQUIRE_EQUAL(db.get(keys[0]), {'a','b','c'});
         BOOST_REQUIRE_EQUAL(db.get(keys[1]), {});
         BOOST_REQUIRE_EQUAL(db.get(keys[2]), {});
         BOOST_REQUIRE_EQUAL(db.get(keys[3]), {});
         BOOST_REQUIRE_EQUAL(db.get(keys[4]), {});
         BOOST_REQUIRE_EQUAL(db.get(keys[5]), {});

         db.create(keys[0], values[0]);
         db.create(keys[1], values[1]);
         db.create(keys[2], values[2]);
         db.create(keys[3], values[3]);
         db.create(keys[4], values[4]);
         db.create(keys[5], values[5]);

         BOOST_REQUIRE_EQUAL(db.get(keys[0]), {'a','b','c'});
         BOOST_REQUIRE_EQUAL(db.get(keys[1]), {'a','c','b'});
         BOOST_REQUIRE_EQUAL(db.get(keys[2]), {'b','a','c'});
         BOOST_REQUIRE_EQUAL(db.get(keys[3]), {'b','c','a'});
         BOOST_REQUIRE_EQUAL(db.get(keys[4]), {'c','a','b'});
         BOOST_REQUIRE_EQUAL(db.get(keys[5]), {'c','b','a'});
         
         session.push();
      }

      BOOST_REQUIRE_EQUAL(db.get(keys[0]), {'a','b','c'});
      BOOST_REQUIRE_EQUAL(db.get(keys[1]), {'a','c','b'});
      BOOST_REQUIRE_EQUAL(db.get(keys[2]), {'b','a','c'});
      BOOST_REQUIRE_EQUAL(db.get(keys[3]), {'b','c','a'});
      BOOST_REQUIRE_EQUAL(db.get(keys[4]), {'c','a','b'});
      BOOST_REQUIRE_EQUAL(db.get(keys[5]), {'c','b','a'});
      
      db.undo();
      
      BOOST_REQUIRE_EQUAL(db.get(keys[0]), {'a','c','b'});
      BOOST_REQUIRE_EQUAL(db.get(keys[1]), {});
      BOOST_REQUIRE_EQUAL(db.get(keys[2]), {});
      BOOST_REQUIRE_EQUAL(db.get(keys[3]), {});
      BOOST_REQUIRE_EQUAL(db.get(keys[4]), {});
      BOOST_REQUIRE_EQUAL(db.get(keys[5]), {});

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
