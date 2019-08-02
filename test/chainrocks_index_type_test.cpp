#define BOOST_TEST_MODULE chainrocks_index_type_test

#include <boost/test/unit_test.hpp>
#include <chainbase/chainrocks.hpp>

struct index_fixture {
   index_fixture() : _index{}
   {
   }

   ~index_fixture()
   {
   }

   chainrocks::index _index;
};

/// Test data.
static const uint64_t keys0[10]{ 0ULL, 1ULL, 2ULL, 3ULL, 4ULL, 5ULL, 6ULL, 7ULL, 8ULL, 9ULL};
static const uint64_t keys1[10]{10ULL,11ULL,12ULL,13ULL,14ULL,15ULL,16ULL,17ULL,18ULL,19ULL};
static const std::string values0[10]{"a","b","c","d","e","f","g","h","i","j"};
static const std::string values1[10]{"k","l","m","n","o","p","q","r","s","t"};

/////////////////////////////////////
/// Testing `database` functionality:
/// Creating/modifying/removing tests.

// Test #1:
//
// `start undo session`
// Fill `_state` with new values
// `undo`
//
// _state:
// _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
// _state:
BOOST_FIXTURE_TEST_CASE(test_one, index_fixture) {
   // _state:
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{}) );

   auto session{_index.start_undo_session(true)};
   for (size_t i{0}; i < 10; ++i) {
      _index.put(keys0[i], values0[i]);
   }

   // _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{{0ULL,"a"},{1ULL,"b"},{2ULL,"c"},{3ULL,"d"},{4ULL,"e"},
                                                                            {5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );

   _index.undo();

   // _state:
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{}) );
}

// Test #2:
//
// Pre-fill `_state`
// `start_undo_session`
// Fill `_state` with new values
// `undo`
//
// _state:
// _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
// _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j 10k 11l 12m 13n 14o 15p 16q 17r 18s 19t
// _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
BOOST_FIXTURE_TEST_CASE(test_two, index_fixture) {
   // _state:
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{}) );

   for (size_t i{}; i < 10; ++i) {
      _index.put(keys0[i], values0[i]);
   }

   // _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{{0ULL,"a"},{1ULL,"b"},{2ULL,"c"},{3ULL,"d"},{4ULL,"e"},
                                                                            {5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );

   auto session{_index.start_undo_session(true)};
   for (size_t i{0}; i < 10; ++i) {
      _index.put(keys1[i], values1[i]);
   }

   // _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j 10k 11l 12m 13n 14o 15p 16q 17r 18s 19t
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{{ 0ULL,"a"},{ 1ULL,"b"},{ 2ULL,"c"},{ 3ULL,"d"},{ 4ULL,"e"},
                                                                            { 5ULL,"f"},{ 6ULL,"g"},{ 7ULL,"h"},{ 8ULL,"i"},{ 9ULL,"j"},
                                                                            {10ULL,"k"},{11ULL,"l"},{12ULL,"m"},{13ULL,"n"},{14ULL,"o"},
                                                                            {15ULL,"p"},{16ULL,"q"},{17ULL,"r"},{18ULL,"s"},{19ULL,"t"}}) );

   _index.undo();

   // _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{{0ULL,"a"},{1ULL,"b"},{2ULL,"c"},{3ULL,"d"},{4ULL,"e"},
                                                                            {5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );
}

// Test #3:
//
// Pre-fill `_state`
// `start_undo_session`
// Modify `_state`
// `undo`
//
// _state:
// _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
// _state: 0k 1l 2m 3n 4o 5p 6q 7r 8s 9t
// _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
BOOST_FIXTURE_TEST_CASE(test_three, index_fixture) {
   // _state:
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{}) );

   for (size_t i{}; i < 10; ++i) {
      _index.put(keys0[i], values0[i]);
   }

   // _state: 0k 1l 2m 3n 4o 5p 6q 7r 8s 9t
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{{0ULL,"a"},{1ULL,"b"},{2ULL,"c"},{3ULL,"d"},{4ULL,"e"},
                                                                            {5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );

   auto session{_index.start_undo_session(true)};
   for (size_t i{0}; i < 10; ++i) {
      _index.put(keys0[i], values1[i]);
   }

   // _state: 0k 1l 2m 3n 4o 5p 6q 7r 8s 9t
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{{0ULL,"k"},{1ULL,"l"},{2ULL,"m"},{3ULL,"n"},{4ULL,"o"},
                                                                            {5ULL,"p"},{6ULL,"q"},{7ULL,"r"},{8ULL,"s"},{9ULL,"t"}}) );

   _index.undo();

   // _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{{0ULL,"a"},{1ULL,"b"},{2ULL,"c"},{3ULL,"d"},{4ULL,"e"},
                                                                            {5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );
}

// Test #4:
//
// Pre-fill `_state`
// `start_undo_session`
// Remove some `_state`
// `undo`
//
// _state:
// _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
// _state:
// _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
BOOST_FIXTURE_TEST_CASE(test_four, index_fixture) {
   // _state:
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{}) );

   for (size_t i{}; i < 10; ++i) {
      _index.put(keys0[i], values0[i]);
   }

   // _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{{0ULL,"a"},{1ULL,"b"},{2ULL,"c"},{3ULL,"d"},{4ULL,"e"},
                                                                            {5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );

   auto session{_index.start_undo_session(true)};
   for (size_t i{0}; i < 10; ++i) {
      _index.remove(keys0[i]);
   }

   // _state:
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{}) );

   _index.undo();

   // _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{{0ULL,"a"},{1ULL,"b"},{2ULL,"c"},{3ULL,"d"},{4ULL,"e"},
                                                                            {5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );
}

// Test #5:
//
// `start_undo_session`
// Fill `_state` with new values
// `start_undo_session`
// Fill `_state` with more new values
// `undo_all`
//
// _state:
// _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
// _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j 10k 11l 12m 13n 14o 15p 16q 17r 18s 19t
// _state:
BOOST_FIXTURE_TEST_CASE(test_five, index_fixture) {
   // _state:
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{}) );

   auto session0{_index.start_undo_session(true)};
   for (size_t i{0}; i < 10; ++i) {
      _index.put(keys0[i], values0[i]);
   }

   // _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{{0ULL,"a"},{1ULL,"b"},{2ULL,"c"},{3ULL,"d"},{4ULL,"e"},
                                                                            {5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );

   auto session1{_index.start_undo_session(true)};
   for (size_t i{0}; i < 10; ++i) {
      _index.put(keys1[i], values1[i]);
   }

   // _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j 10k 11l 12m 13n 14o 15p 16q 17r 18s 19t
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{{ 0ULL,"a"},{ 1ULL,"b"},{ 2ULL,"c"},{ 3ULL,"d"},{ 4ULL,"e"},
                                                                            { 5ULL,"f"},{ 6ULL,"g"},{ 7ULL,"h"},{ 8ULL,"i"},{ 9ULL,"j"},
                                                                            {10ULL,"k"},{11ULL,"l"},{12ULL,"m"},{13ULL,"n"},{14ULL,"o"},
                                                                            {15ULL,"p"},{16ULL,"q"},{17ULL,"r"},{18ULL,"s"},{19ULL,"t"}}) );

   _index.undo_all();

   // _state:
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{}) );
}

/////////////////////////////////////
/// Testing `database` functionality:
/// Squash tests.

// Test #6:
//
// `start_undo_session`
// Add some keys
// `start_undo_session`
// Add some keys
// `squash`
//
// _state:
// _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
// _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j 10k 11l 12m 13n 14o 15p 16q 17r 18s 19t
// _new_keys<0>: 0 1 2 3 4 5 6 7 8 9
// _new_keys<1>: 10 11 12 13 14 15 16 17 18 19
// _new_keys<0>: 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19
BOOST_FIXTURE_TEST_CASE(test_six, index_fixture) {
   // _state:
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{}) );

   auto session0{_index.start_undo_session(true)};
   for (size_t i{0}; i < 10; ++i) {
      _index.put(keys0[i], values0[i]);
   }

   // _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{{0ULL,"a"},{1ULL,"b"},{2ULL,"c"},{3ULL,"d"},{4ULL,"e"},
                                                                            {5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );

   auto session1{_index.start_undo_session(true)};
   for (size_t i{0}; i < 10; ++i) {
      _index.put(keys1[i], values1[i]);
   }

   // _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j 10k 11l 12m 13n 14o 15p 16q 17r 18s 19t
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{{ 0ULL,"a"},{ 1ULL,"b"},{ 2ULL,"c"},{ 3ULL,"d"},{ 4ULL,"e"},
                                                                            { 5ULL,"f"},{ 6ULL,"g"},{ 7ULL,"h"},{ 8ULL,"i"},{ 9ULL,"j"},
                                                                            {10ULL,"k"},{11ULL,"l"},{12ULL,"m"},{13ULL,"n"},{14ULL,"o"},
                                                                            {15ULL,"p"},{16ULL,"q"},{17ULL,"r"},{18ULL,"s"},{19ULL,"t"}}) );

   // new_keys: 0 1 2 3 4 5 6 7 8 9
   // new_keys: 10 11 12 13 14 15 16 17 18 19
   _index.print_keys();
   BOOST_TEST_REQUIRE( (_index.stack()[_index.stack().size()-2]._new_keys) == (std::set<uint64_t>{0ULL,1ULL,2ULL,3ULL,4ULL,5ULL,6ULL,7ULL,8ULL,9ULL}) );

   _index.squash();

   // new_keys: 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19
   _index.print_keys();
   BOOST_TEST_REQUIRE( (_index.stack().back()._new_keys) == (std::set<uint64_t>{ 0ULL, 1ULL, 2ULL, 3ULL, 4ULL, 5ULL, 6ULL, 7ULL, 8ULL, 9ULL,
                                                                                10ULL,11ULL,12ULL,13ULL,14ULL,15ULL,16ULL,17ULL,18ULL,19ULL}) );
}

// Test #7:
//
// `start_undo_session`
// Add some keys
// `start_undo_session`
// Modify some keys
// `squash`
//
// _state:
// _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
// _state: 0k 1l 2m 3n 4o 5f 6g 7h 8i 9j
// _state: 0k 1l 2m 3n 4o 5f 6g 7h 8i 9j
BOOST_FIXTURE_TEST_CASE(test_seven, index_fixture) {
   // _state:
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{}) );

   auto session0{_index.start_undo_session(true)};
   for (size_t i{0}; i < 10; ++i) {
      _index.put(keys0[i], values0[i]);
   }

   // _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{{0ULL,"a"},{1ULL,"b"},{2ULL,"c"},{3ULL,"d"},{4ULL,"e"},
                                                                            {5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );

   auto session1{_index.start_undo_session(true)};
   for (size_t i{0}; i < 5; ++i) {
      _index.put(keys0[i], values1[i]);
   }

   // _state: 0k 1l 2m 3n 4o 5f 6g 7h 8i 9j
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{{0ULL,"k"},{1ULL,"l"},{2ULL,"m"},{3ULL,"n"},{4ULL,"o"},
                                                                            {5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );

   _index.squash();

   // _state: 0k 1l 2m 3n 4o 5f 6g 7h 8i 9j
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{{0ULL,"k"},{1ULL,"l"},{2ULL,"m"},{3ULL,"n"},{4ULL,"o"},
                                                                            {5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );
}

// Test #8:
//
// Pre-fill state
// `start_undo_session`
// Remove some `_state`
// `start_undo_session`
// Remove some `_state`
// `squash`
//
// _state:
// _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
// _state: 5f 6g 7h 8i 9j
// _state: 9j
// _state: 9j
BOOST_FIXTURE_TEST_CASE(test_eight, index_fixture) {
   // _state:
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{}) );

   for (size_t i{}; i < 10; ++i) {
      _index.put(keys0[i], values0[i]);
   }

   // _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{{0ULL,"a"},{1ULL,"b"},{2ULL,"c"},{3ULL,"d"},{4ULL,"e"},
                                                                            {5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );

   auto session0{_index.start_undo_session(true)};
   for (size_t i{0}; i < 5; ++i) {
      _index.remove(keys0[i]);
   }

   // _state: 5f 6g 7h 8i 9j
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{{5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );

   auto session1{_index.start_undo_session(true)};
   for (size_t i{5}; i < 9; ++i) {
      _index.remove(keys0[i]);
   }

   // _state: 9j
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{{9ULL,"j"}}) );

   _index.squash();

   // _state: 9j
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{{9ULL,"j"}}) );
}

/////////////////////////////////////
/// Testing `database` functionality:
/// RAII/push tests.

// Test #9:
//
// Initiate RAII `{`
// `start_undo_session`
// Fill `_state` with new values
// End RAII `}`
//
// _state:
// _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
// _state:
BOOST_FIXTURE_TEST_CASE(test_nine, index_fixture) {
   // _state:
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{}) );

   {
   auto session{_index.start_undo_session(true)};
   for (size_t i{0}; i < 10; ++i) {
      _index.put(keys0[i], values0[i]);
   }

   // _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{{0ULL,"a"},{1ULL,"b"},{2ULL,"c"},{3ULL,"d"},{4ULL,"e"},
                                                                            {5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );
   }

   // _state:
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{}) );
}

// Test #10:
//
// `start_undo_session`
// Initiate RAII `{`
// Fill `_state` with new values
// End RAII `}`
//
// _state:
// _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
// _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
BOOST_FIXTURE_TEST_CASE(test_ten, index_fixture) {
   // _state:
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{}) );

   auto session{_index.start_undo_session(true)};
   {
   for (size_t i{0}; i < 10; ++i) {
      _index.put(keys0[i], values0[i]);
   }

   // _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{{0ULL,"a"},{1ULL,"b"},{2ULL,"c"},{3ULL,"d"},{4ULL,"e"},
                                                                            {5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );
   }

   // _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{{0ULL,"a"},{1ULL,"b"},{2ULL,"c"},{3ULL,"d"},{4ULL,"e"},
                                                                            {5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );
}

////////////////
/// Other tests.

// Test #11:
//
// Pre-fill state
// `start_undo_session`
// Remove `_state` 0a
// `start_undo_session`
// Put `_state` 0a
// `squash`
// Remove some `_state`
BOOST_FIXTURE_TEST_CASE(test_eleven, index_fixture) {
   // _state:
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{}) );

   for (size_t i{}; i < 10; ++i) {
      _index.put(keys0[i], values0[i]);
   }

   // _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{{0ULL,"a"},{1ULL,"b"},{2ULL,"c"},{3ULL,"d"},{4ULL,"e"},
                                                                            {5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );

   auto session0{_index.start_undo_session(true)};
   _index.remove(keys0[0]);

   // _state: 1b 2c 3d 4e 5f 6g 7h 8i 9j
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{{1ULL,"b"},{2ULL,"c"},{3ULL,"d"},{4ULL,"e"}, {5ULL,"f"},
                                                                            {6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );

   auto session1{_index.start_undo_session(true)};
   _index.put(keys0[0], values0[0]);

   // _state: 9j
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{{0ULL,"a"},{1ULL,"b"},{2ULL,"c"},{3ULL,"d"},{4ULL,"e"},
                                                                            {5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );

   _index.squash();

   // Under my previous implementation this would throw, which is incorrect behavior.
   // Now, under the fixed implementation, this should now throw.
   _index.remove(keys0[0]);

   // _state: 1b 2c 3d 4e 5f 6g 7h 8i 9j
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{{1ULL,"b"},{2ULL,"c"},{3ULL,"d"},{4ULL,"e"}, {5ULL,"f"},
                                                                            {6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );
}

// Test #12:
// Sanity test.
// I produced this test to check that the `_on_remove` logic remained
// correct and consistent.
BOOST_FIXTURE_TEST_CASE(test_twelve, index_fixture) {
   // _state:
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{}) );

   auto session0{_index.start_undo_session(true)};

   _index.put(keys0[0], values0[0]);

   // _state: 0a
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{{0ULL,"a"}}) );
   
   _index.remove(keys0[0]);

   // _state:
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{}) );
}

// Test #13:
// Sanity test.
// I produced this test to check that the `commit` logic remained
// correct and consistent.
BOOST_FIXTURE_TEST_CASE(test_thirteen, index_fixture) {
   // _state:
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{}) );

   _index.start_undo_session(true).push();
   _index.put(keys0[0], values0[0]);
   // _state: 0a
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{{0ULL,"a"}}) );

   _index.start_undo_session(true).push();
   _index.put(keys0[1], values0[1]);
   // _state: 0a 1b
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{{0ULL,"a"},{1ULL,"b"}}) );

   _index.start_undo_session(true).push();
   _index.put(keys0[2], values0[2]);
   // _state: 0a 1b 2c
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{{0ULL,"a"},{1ULL,"b"},{2ULL,"c"}}) );
   
   BOOST_CHECK_THROW( _index.commit(4), std::runtime_error );

   _index.commit(3);
   // _state: 0a 1b 2c
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{{0ULL,"a"},{1ULL,"b"},{2ULL,"c"}}) );

   _index.undo_all();
   // _state: 0a 1b 2c
   _index.print_state();
   BOOST_TEST_REQUIRE( (_index.state()) == (std::map<uint64_t, std::string>{{0ULL,"a"},{1ULL,"b"},{2ULL,"c"}}) );
   
BOOST_AUTO_TEST_SUITE_END()
