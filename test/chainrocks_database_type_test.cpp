/// [ ] TODO: add more descriptive test names.
/// [ ] TODO: fix up comments.

#define BOOST_TEST_MODULE chainrocks_database_type_test

#include <iostream> // std::cout

#include <boost/test/unit_test.hpp>
#include <chainbase/chainrocks.hpp>

/// Test data.
static const uint64_t keys1[10]{ 0ULL, 1ULL, 2ULL, 3ULL, 4ULL, 5ULL, 6ULL, 7ULL, 8ULL, 9ULL};
static const uint64_t keys2[10]{10ULL,11ULL,12ULL,13ULL,14ULL,15ULL,16ULL,17ULL,18ULL,19ULL};
static const std::string values1[10]{"a","b","c","d","e","f","g","h","i","j"};
static const std::string values2[10]{"k","l","m","n","o","p","q","r","s","t"};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/// Creating, Modifying, and Removing Tests.

// Test 1:
// Start undo session
// Fill up `_state` with new values
// Undo state.
//
// _state:
// _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
// _state:
BOOST_AUTO_TEST_CASE(test_one) {
   chainrocks::database database{"/Users/john.debord/chainbase2/build/test/data"};
   // _state:
   database.print_state(); 
   BOOST_TEST_REQUIRE( (database.state()) == (std::map<uint64_t, std::string>{}) );

   auto session{database.start_undo_session(true)};
   for (size_t i{0}; i < 10; ++i) {
      database.put(keys1[i], values1[i]);
   }

   // _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
   database.print_state(); 
   BOOST_TEST_REQUIRE( (database.state()) == (std::map<uint64_t, std::string>{{0ULL,"a"},{1ULL,"b"},{2ULL,"c"},{3ULL,"d"},{4ULL,"e"},
                                                                              {5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );

   session.undo();

   // _state:
   database.print_state(); 
   BOOST_TEST_REQUIRE( (database.state()) == (std::map<uint64_t, std::string>{}) );

   boost::filesystem::remove_all("/Users/john.debord/chainbase2/build/test/data");
}

// Test 2:
// Make pre-filled state
// Start undo session
// Fill up `_state` with new values
// Undo state.
// _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
// _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j 10k 11l 12m 13n 14o 15p 16q 17r 18s 19t
// _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
BOOST_AUTO_TEST_CASE(test_two) {
   chainrocks::database database{"/Users/john.debord/chainbase2/build/test/data"};
   // _state:
   database.print_state(); 
   BOOST_TEST_REQUIRE( (database.state()) == (std::map<uint64_t, std::string>{}) );

   for (size_t i{}; i < 10; ++i) {
      database.put(keys1[i], values1[i]);
   }

   // _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
   database.print_state(); 
   BOOST_TEST_REQUIRE( (database.state()) == (std::map<uint64_t, std::string>{{0ULL,"a"},{1ULL,"b"},{2ULL,"c"},{3ULL,"d"},{4ULL,"e"},
                                                                              {5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );

   auto session{database.start_undo_session(true)};
   for (size_t i{0}; i < 10; ++i) {
      database.put(keys2[i], values2[i]);
   }

   // _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j 10k 11l 12m 13n 14o 15p 16q 17r 18s 19t
   database.print_state(); 
   BOOST_TEST_REQUIRE( (database.state()) == (std::map<uint64_t, std::string>{{ 0ULL,"a"},{ 1ULL,"b"},{ 2ULL,"c"},{ 3ULL,"d"},{ 4ULL,"e"},
                                                                              { 5ULL,"f"},{ 6ULL,"g"},{ 7ULL,"h"},{ 8ULL,"i"},{ 9ULL,"j"},
                                                                              {10ULL,"k"},{11ULL,"l"},{12ULL,"m"},{13ULL,"n"},{14ULL,"o"},
                                                                              {15ULL,"p"},{16ULL,"q"},{17ULL,"r"},{18ULL,"s"},{19ULL,"t"}}) );

   session.undo();

   // _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
   database.print_state(); 
   BOOST_TEST_REQUIRE( (database.state()) == (std::map<uint64_t, std::string>{{0ULL,"a"},{1ULL,"b"},{2ULL,"c"},{3ULL,"d"},{4ULL,"e"},
                                                                              {5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );

   boost::filesystem::remove_all("/Users/john.debord/chainbase2/build/test/data");
}

// Test 3:
// Make pre-filled state
// Start undo session
// Modify `_state`
// Undo state.
// _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
// _state: 0k 1l 2m 3n 4o 5p 6q 7r 8s 9t
// _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
BOOST_AUTO_TEST_CASE(test_three) {
   chainrocks::database database{"/Users/john.debord/chainbase2/build/test/data"};
   // _state:
   database.print_state(); 
   BOOST_TEST_REQUIRE( (database.state()) == (std::map<uint64_t, std::string>{}) );

   for (size_t i{}; i < 10; ++i) {
      database.put(keys1[i], values1[i]);
   }

   // _state: 0k 1l 2m 3n 4o 5p 6q 7r 8s 9t
   database.print_state(); 
   BOOST_TEST_REQUIRE( (database.state()) == (std::map<uint64_t, std::string>{{0ULL,"a"},{1ULL,"b"},{2ULL,"c"},{3ULL,"d"},{4ULL,"e"},
                                                                              {5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );

   auto session{database.start_undo_session(true)};
   for (size_t i{0}; i < 10; ++i) {
      database.put(keys1[i], values2[i]);
   }

   // _state: 0k 1l 2m 3n 4o 5p 6q 7r 8s 9t
   database.print_state(); 
   BOOST_TEST_REQUIRE( (database.state()) == (std::map<uint64_t, std::string>{{0ULL,"k"},{1ULL,"l"},{2ULL,"m"},{3ULL,"n"},{4ULL,"o"},
                                                                              {5ULL,"p"},{6ULL,"q"},{7ULL,"r"},{8ULL,"s"},{9ULL,"t"}}) );

   session.undo();

   // _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
   database.print_state(); 
   BOOST_TEST_REQUIRE( (database.state()) == (std::map<uint64_t, std::string>{{0ULL,"a"},{1ULL,"b"},{2ULL,"c"},{3ULL,"d"},{4ULL,"e"},
                                                                              {5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );

   boost::filesystem::remove_all("/Users/john.debord/chainbase2/build/test/data");
}

// // Test 4:
// // Make pre-filled state
// Start undo session
// Remove some `_state`
// Undo state.
// _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
// _state:
// _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
BOOST_AUTO_TEST_CASE(test_four) {
   chainrocks::database database{"/Users/john.debord/chainbase2/build/test/data"};
   // _state:
   database.print_state(); 
   BOOST_TEST_REQUIRE( (database.state()) == (std::map<uint64_t, std::string>{}) );

   for (size_t i{}; i < 10; ++i) {
      database.put(keys1[i], values1[i]);
   }

   // _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
   database.print_state(); 
   BOOST_TEST_REQUIRE( (database.state()) == (std::map<uint64_t, std::string>{{0ULL,"a"},{1ULL,"b"},{2ULL,"c"},{3ULL,"d"},{4ULL,"e"},
                                                                              {5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );

   auto session{database.start_undo_session(true)};
   for (size_t i{0}; i < 10; ++i) {
      database.remove(keys1[i]);
   }

   // _state:
   database.print_state(); 
   BOOST_TEST_REQUIRE( (database.state()) == (std::map<uint64_t, std::string>{}) );

   session.undo();

   // _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
   database.print_state(); 
   BOOST_TEST_REQUIRE( (database.state()) == (std::map<uint64_t, std::string>{{0ULL,"a"},{1ULL,"b"},{2ULL,"c"},{3ULL,"d"},{4ULL,"e"},
                                                                              {5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );

   boost::filesystem::remove_all("/Users/john.debord/chainbase2/build/test/data");
}

// Test 5:
// Start undo session
// Fill up `_state` with new values
// Start another undo session
// Fill up more `_state` with new values
// Undo all.
// _state:
// _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
// _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j 10k 11l 12m 13n 14o 15p 16q 17r 18s 19t
// _state:
BOOST_AUTO_TEST_CASE(test_five) {
   chainrocks::database database{"/Users/john.debord/chainbase2/build/test/data"};
   // _state:
   database.print_state(); 
   BOOST_TEST_REQUIRE( (database.state()) == (std::map<uint64_t, std::string>{}) );

   auto session{database.start_undo_session(true)};
   for (size_t i{0}; i < 10; ++i) {
      database.put(keys1[i], values1[i]);
   }

   // _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
   database.print_state(); 
   BOOST_TEST_REQUIRE( (database.state()) == (std::map<uint64_t, std::string>{{0ULL,"a"},{1ULL,"b"},{2ULL,"c"},{3ULL,"d"},{4ULL,"e"},
                                                                              {5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );

   session = database.start_undo_session(true);
   for (size_t i{0}; i < 10; ++i) {
      database.put(keys2[i], values2[i]);
   }

   // _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j 10k 11l 12m 13n 14o 15p 16q 17r 18s 19t
   database.print_state(); 
   BOOST_TEST_REQUIRE( (database.state()) == (std::map<uint64_t, std::string>{{ 0ULL,"a"},{ 1ULL,"b"},{ 2ULL,"c"},{ 3ULL,"d"},{ 4ULL,"e"},
                                                                              { 5ULL,"f"},{ 6ULL,"g"},{ 7ULL,"h"},{ 8ULL,"i"},{ 9ULL,"j"},
                                                                              {10ULL,"k"},{11ULL,"l"},{12ULL,"m"},{13ULL,"n"},{14ULL,"o"},
                                                                              {15ULL,"p"},{16ULL,"q"},{17ULL,"r"},{18ULL,"s"},{19ULL,"t"}}) );

   database.undo_all(); // TODO: This is very flimsy. `session` type or some other type should be handling this, not database.

   // _state:
   database.print_state(); 
   BOOST_TEST_REQUIRE( (database.state()) == (std::map<uint64_t, std::string>{}) );

   boost::filesystem::remove_all("/Users/john.debord/chainbase2/build/test/data");
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/// Squash Tests.

// Test 6:
// Start undo session
// Add some keys
// Start undo session
// Add some keys
// Squash
// _state:
// _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
// _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j 10k 11l 12m 13n 14o 15p 16q 17r 18s 19t
// _new_keys<0>: 0 1 2 3 4 5 6 7 8 9
// _new_keys<1>: 10 11 12 13 14 15 16 17 18 19
// _new_keys<0>: 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19
BOOST_AUTO_TEST_CASE(test_six) {
   chainrocks::database database{"/Users/john.debord/chainbase2/build/test/data"};
   // _state:
   database.print_state(); 
   BOOST_TEST_REQUIRE( (database.state()) == (std::map<uint64_t, std::string>{}) );

   auto session1{database.start_undo_session(true)}; // TODO: a temp `session` object gets destroyed `_stack` as well.
   for (size_t i{0}; i < 10; ++i) {
      database.put(keys1[i], values1[i]);
   }

   // _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
   database.print_state();
   BOOST_TEST_REQUIRE( (database.state()) == (std::map<uint64_t, std::string>{{0ULL,"a"},{1ULL,"b"},{2ULL,"c"},{3ULL,"d"},{4ULL,"e"},
                                                                              {5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );

   auto session2{database.start_undo_session(true)};
   for (size_t i{0}; i < 10; ++i) {
      database.put(keys2[i], values2[i]);
   }

   // _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j 10k 11l 12m 13n 14o 15p 16q 17r 18s 19t
   database.print_state();
   BOOST_TEST_REQUIRE( (database.state()) == (std::map<uint64_t, std::string>{{ 0ULL,"a"},{ 1ULL,"b"},{ 2ULL,"c"},{ 3ULL,"d"},{ 4ULL,"e"},
                                                                              { 5ULL,"f"},{ 6ULL,"g"},{ 7ULL,"h"},{ 8ULL,"i"},{ 9ULL,"j"},
                                                                              {10ULL,"k"},{11ULL,"l"},{12ULL,"m"},{13ULL,"n"},{14ULL,"o"},
                                                                              {15ULL,"p"},{16ULL,"q"},{17ULL,"r"},{18ULL,"s"},{19ULL,"t"}}) );

   // new_keys: 0 1 2 3 4 5 6 7 8 9
   // new_keys: 10 11 12 13 14 15 16 17 18 19
   database.print_keys();
   BOOST_TEST_REQUIRE( (database.stack()[database.stack().size()-2]._new_keys) == (std::set<uint64_t>{0ULL,1ULL,2ULL,3ULL,4ULL,5ULL,6ULL,7ULL,8ULL,9ULL}) );

   session2.squash();

   // new_keys: 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19
   database.print_keys();
   BOOST_TEST_REQUIRE( (database.stack().back()._new_keys) == (std::set<uint64_t>{ 0ULL, 1ULL, 2ULL, 3ULL, 4ULL, 5ULL, 6ULL, 7ULL, 8ULL, 9ULL,
                                                                                  10ULL,11ULL,12ULL,13ULL,14ULL,15ULL,16ULL,17ULL,18ULL,19ULL}) );

   boost::filesystem::remove_all("/Users/john.debord/chainbase2/build/test/data");
}

// Test 7:
// Start undo session
// Add some keys
// Start undo session
// Modify some keys
// Squash
// _state:
// _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
// _state: 0k 1l 2m 3n 4o 5f 6g 7h 8i 9j
// _state: 0k 1l 2m 3n 4o 5f 6g 7h 8i 9j
BOOST_AUTO_TEST_CASE(test_seven) {
   chainrocks::database database{"/Users/john.debord/chainbase2/build/test/data"};
   // _state:
   database.print_state(); 
   BOOST_TEST_REQUIRE( (database.state()) == (std::map<uint64_t, std::string>{}) );

   auto session1{database.start_undo_session(true)};
   for (size_t i{0}; i < 10; ++i) {
      database.put(keys1[i], values1[i]);
   }

   // _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
   database.print_state();
   BOOST_TEST_REQUIRE( (database.state()) == (std::map<uint64_t, std::string>{{0ULL,"a"},{1ULL,"b"},{2ULL,"c"},{3ULL,"d"},{4ULL,"e"},
                                                                              {5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );

   auto session2{database.start_undo_session(true)};
   for (size_t i{0}; i < 5; ++i) {
      database.put(keys1[i], values2[i]);
   }

   // _state: 0k 1l 2m 3n 4o 5f 6g 7h 8i 9j
   database.print_state();
   BOOST_TEST_REQUIRE( (database.state()) == (std::map<uint64_t, std::string>{{0ULL,"k"},{1ULL,"l"},{2ULL,"m"},{3ULL,"n"},{4ULL,"o"},
                                                                              {5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );

   session2.squash();

   // _state: 0k 1l 2m 3n 4o 5f 6g 7h 8i 9j
   database.print_state();
   BOOST_TEST_REQUIRE( (database.state()) == (std::map<uint64_t, std::string>{{0ULL,"k"},{1ULL,"l"},{2ULL,"m"},{3ULL,"n"},{4ULL,"o"},
                                                                              {5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );

   boost::filesystem::remove_all("/Users/john.debord/chainbase2/build/test/data");
}

// Test 8:
// Make pre-filled state
// Start undo session
// Remove some `_state`
// Start undo session
// Remove some `_state`
// Squash
// _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
// _state: 5f 6g 7h 8i 9j
// _state: 9j
// _state: 9j
BOOST_AUTO_TEST_CASE(test_eight) {
   chainrocks::database database{"/Users/john.debord/chainbase2/build/test/data"};
   // _state:
   database.print_state(); 
   BOOST_TEST_REQUIRE( (database.state()) == (std::map<uint64_t, std::string>{}) );
      
   for (size_t i{}; i < 10; ++i) {
      database.put(keys1[i], values1[i]);
   }

   // _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
   database.print_state();
   BOOST_TEST_REQUIRE( (database.state()) == (std::map<uint64_t, std::string>{{0ULL,"a"},{1ULL,"b"},{2ULL,"c"},{3ULL,"d"},{4ULL,"e"},
                                                                              {5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );

   auto session1{database.start_undo_session(true)};
   for (size_t i{0}; i < 5; ++i) {
      database.remove(keys1[i]);
   }

   // _state: 5f 6g 7h 8i 9j
   database.print_state();
   BOOST_TEST_REQUIRE( (database.state()) == (std::map<uint64_t, std::string>{{5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );

   auto session2{database.start_undo_session(true)};
   for (size_t i{5}; i < 9; ++i) {
      database.remove(keys1[i]);
   }

   // _state: 9j
   database.print_state();
   BOOST_TEST_REQUIRE( (database.state()) == (std::map<uint64_t, std::string>{{9ULL,"j"}}) );

   session2.squash();

   // _state: 9j
   database.print_state();
   BOOST_TEST_REQUIRE( (database.state()) == (std::map<uint64_t, std::string>{{9ULL,"j"}}) );

   boost::filesystem::remove_all("/Users/john.debord/chainbase2/build/test/data");
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/// RAII and `push` function tests

// Test 9:
// Initiate RAII `{`
// Start undo session
// Fill up `_state` with new values
// End RAII `}`
//
// _state:
// _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
// _state:
BOOST_AUTO_TEST_CASE(test_nine) {
   chainrocks::database database{"/Users/john.debord/chainbase2/build/test/data"};
   // _state:
   database.print_state(); 
   BOOST_TEST_REQUIRE( (database.state()) == (std::map<uint64_t, std::string>{}) );

   {
   auto session{database.start_undo_session(true)};
   for (size_t i{0}; i < 10; ++i) {
      database.put(keys1[i], values1[i]);
   }

   // _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
   database.print_state(); 
   BOOST_TEST_REQUIRE( (database.state()) == (std::map<uint64_t, std::string>{{0ULL,"a"},{1ULL,"b"},{2ULL,"c"},{3ULL,"d"},{4ULL,"e"},
                                                                              {5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );
   }

   // _state:
   database.print_state(); 
   BOOST_TEST_REQUIRE( (database.state()) == (std::map<uint64_t, std::string>{}) );

   boost::filesystem::remove_all("/Users/john.debord/chainbase2/build/test/data");
}

// Test 10:
// Start undo session
// Initiate RAII `{`
// Fill up `_state` with new values
// End RAII `}`
//
// _state:
// _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
// _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
BOOST_AUTO_TEST_CASE(test_ten) {
   chainrocks::database database{"/Users/john.debord/chainbase2/build/test/data"};
   // _state:
   database.print_state(); 
   BOOST_TEST_REQUIRE( (database.state()) == (std::map<uint64_t, std::string>{}) );

   auto session{database.start_undo_session(true)};
   {
   for (size_t i{0}; i < 10; ++i) {
      database.put(keys1[i], values1[i]);
   }

   // _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
   database.print_state(); 
   BOOST_TEST_REQUIRE( (database.state()) == (std::map<uint64_t, std::string>{{0ULL,"a"},{1ULL,"b"},{2ULL,"c"},{3ULL,"d"},{4ULL,"e"},
                                                                              {5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );
   }

   // _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
   database.print_state(); 
   BOOST_TEST_REQUIRE( (database.state()) == (std::map<uint64_t, std::string>{{0ULL,"a"},{1ULL,"b"},{2ULL,"c"},{3ULL,"d"},{4ULL,"e"},
                                                                              {5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );

   boost::filesystem::remove_all("/Users/john.debord/chainbase2/build/test/data");
}

// Test 11:
// Initiate RAII `{`
// Start undo session
// Fill up `_state` with new values
// Push undo session
// End RAII `}`
//
// _state:
// _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
// _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
BOOST_AUTO_TEST_CASE(test_eleven) {
   chainrocks::database database{"/Users/john.debord/chainbase2/build/test/data"};
   // _state:
   database.print_state(); 
   BOOST_TEST_REQUIRE( (database.state()) == (std::map<uint64_t, std::string>{}) );

   {
   auto session{database.start_undo_session(true)};
   for (size_t i{0}; i < 10; ++i) {
      database.put(keys1[i], values1[i]);
   }

   // _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
   database.print_state(); 
   BOOST_TEST_REQUIRE( (database.state()) == (std::map<uint64_t, std::string>{{0ULL,"a"},{1ULL,"b"},{2ULL,"c"},{3ULL,"d"},{4ULL,"e"},
                                                                              {5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );
   session.push();
   }

   // _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j
   database.print_state(); 
   BOOST_TEST_REQUIRE( (database.state()) == (std::map<uint64_t, std::string>{{0ULL,"a"},{1ULL,"b"},{2ULL,"c"},{3ULL,"d"},{4ULL,"e"},
                                                                              {5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );

   boost::filesystem::remove_all("/Users/john.debord/chainbase2/build/test/data");
BOOST_AUTO_TEST_SUITE_END()
