/// [ ] TODO: add more descriptive test names.

#define BOOST_TEST_MODULE chainrocks test

#include <iostream>
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
   try {
      print_state();
      BOOST_TEST_REQUIRE( (_state) == (std::map<uint64_t, std::string>{}) );
    
      start_undo_session();
      for (size_t i{0}; i < 10; ++i) {
         put(keys1[i], values1[i]);
      }
      print_state(); // This might not be right.
      BOOST_TEST_REQUIRE( (_state) == (std::map<uint64_t, std::string>{{0ULL,"a"},{1ULL,"b"},{2ULL,"c"},{3ULL,"d"},{4ULL,"e"},
                                                                       {5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );
    
      undo();
      print_state();
      BOOST_TEST_REQUIRE( (_state) == (std::map<uint64_t, std::string>{}) );
      // BOOST_TEST_REQUIRE( (_state) == (std::map<uint64_t, std::string>{{0ULL,""},{1ULL,""},{2ULL,""},{3ULL,""},{4ULL,""},
      //                                                                  {5ULL,""},{6ULL,""},{7ULL,""},{8ULL,""},{9ULL,""}}) );
      clear_everything();
   } catch (...) {
      throw;
   }
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
   try {
      for (size_t i{}; i < 10; ++i) {
         _state[keys1[i]] = values1[i];
      }
      print_state();
      BOOST_TEST_REQUIRE( (_state) == (std::map<uint64_t, std::string>{{0ULL,"a"},{1ULL,"b"},{2ULL,"c"},{3ULL,"d"},{4ULL,"e"},
                                                                       {5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );
    
      start_undo_session();
      for (size_t i{0}; i < 10; ++i) {
         put(keys2[i], values2[i]);
      }
      print_state();
      BOOST_TEST_REQUIRE( (_state) == (std::map<uint64_t, std::string>{{ 0ULL,"a"},{ 1ULL,"b"},{ 2ULL,"c"},{ 3ULL,"d"},{ 4ULL,"e"},
                                                                       { 5ULL,"f"},{ 6ULL,"g"},{ 7ULL,"h"},{ 8ULL,"i"},{ 9ULL,"j"},
                                                                       {10ULL,"k"},{11ULL,"l"},{12ULL,"m"},{13ULL,"n"},{14ULL,"o"},
                                                                       {15ULL,"p"},{16ULL,"q"},{17ULL,"r"},{18ULL,"s"},{19ULL,"t"}}) );
    
      undo();
      print_state();
      BOOST_TEST_REQUIRE( (_state) == (std::map<uint64_t, std::string>{{0ULL,"a"},{1ULL,"b"},{2ULL,"c"},{3ULL,"d"},{4ULL,"e"},
                                                                       {5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );
      // BOOST_TEST_REQUIRE( (_state) == (std::map<uint64_t, std::string>{{ 0ULL,"a"},{ 1ULL,"b"},{ 2ULL,"c"},{ 3ULL,"d"},{ 4ULL,"e"},
      //                                                                  { 5ULL,"f"},{ 6ULL,"g"},{ 7ULL,"h"},{ 8ULL,"i"},{ 9ULL,"j"},
      //                                                                  {10ULL, ""},{11ULL, ""},{12ULL, ""},{13ULL, ""},{14ULL, ""},
      //                                                                  {15ULL, ""},{16ULL, ""},{17ULL, ""},{18ULL, ""},{19ULL, ""}}) );
      clear_everything();
   } catch (...) {
      throw;
   }
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
   try {
      for (size_t i{}; i < 10; ++i) {
         _state[keys1[i]] = values1[i];
      }
      print_state();
      BOOST_TEST_REQUIRE( (_state) == (std::map<uint64_t, std::string>{{0ULL,"a"},{1ULL,"b"},{2ULL,"c"},{3ULL,"d"},{4ULL,"e"},
                                                                       {5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );
    
      start_undo_session();
      for (size_t i{0}; i < 10; ++i) {
         put(keys1[i], values2[i]);
      }
      print_state();
      BOOST_TEST_REQUIRE( (_state) == (std::map<uint64_t, std::string>{{0ULL,"k"},{1ULL,"l"},{2ULL,"m"},{3ULL,"n"},{4ULL,"o"},
                                                                       {5ULL,"p"},{6ULL,"q"},{7ULL,"r"},{8ULL,"s"},{9ULL,"t"}}) );
    
      undo();
      print_state();
      BOOST_TEST_REQUIRE( (_state) == (std::map<uint64_t, std::string>{{0ULL,"a"},{1ULL,"b"},{2ULL,"c"},{3ULL,"d"},{4ULL,"e"},
                                                                       {5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );
      clear_everything();
   } catch (...) {
      throw;
   }
}

// Test 4:
// Make pre-filled state
// Start undo session
// Remove some `_state`
// Undo state.
// _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j 
// _state:
// _state: 0a 1b 2c 3d 4e 5f 6g 7h 8i 9j 
BOOST_AUTO_TEST_CASE(test_four) {
   try {
      for (size_t i{}; i < 10; ++i) {
         _state[keys1[i]] = values1[i];
      }
      print_state();
      BOOST_TEST_REQUIRE( (_state) == (std::map<uint64_t, std::string>{{0ULL,"a"},{1ULL,"b"},{2ULL,"c"},{3ULL,"d"},{4ULL,"e"},
                                                                       {5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );
    
      start_undo_session();
      for (size_t i{0}; i < 10; ++i) {
         remove(keys1[i]);
      }
      print_state();
      BOOST_TEST_REQUIRE( (_state) == (std::map<uint64_t, std::string>{}) );
    
      undo();
      print_state();
      BOOST_TEST_REQUIRE( (_state) == (std::map<uint64_t, std::string>{{0ULL,"a"},{1ULL,"b"},{2ULL,"c"},{3ULL,"d"},{4ULL,"e"},
                                                                       {5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );
      clear_everything();
   } catch (...) {
      throw;
   }
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
   try {
      print_state();
      BOOST_TEST_REQUIRE( (_state) == (std::map<uint64_t, std::string>{}) );
    
      start_undo_session();
      for (size_t i{0}; i < 10; ++i) {
         put(keys1[i], values1[i]);
      }
      print_state();
      BOOST_TEST_REQUIRE( (_state) == (std::map<uint64_t, std::string>{{0ULL,"a"},{1ULL,"b"},{2ULL,"c"},{3ULL,"d"},{4ULL,"e"},
                                                                       {5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );

      start_undo_session();
      for (size_t i{0}; i < 10; ++i) {
         put(keys2[i], values2[i]);
      }
      print_state();
      BOOST_TEST_REQUIRE( (_state) == (std::map<uint64_t, std::string>{{ 0ULL,"a"},{ 1ULL,"b"},{ 2ULL,"c"},{ 3ULL,"d"},{ 4ULL,"e"},
                                                                       { 5ULL,"f"},{ 6ULL,"g"},{ 7ULL,"h"},{ 8ULL,"i"},{ 9ULL,"j"},
                                                                       {10ULL,"k"},{11ULL,"l"},{12ULL,"m"},{13ULL,"n"},{14ULL,"o"},
                                                                       {15ULL,"p"},{16ULL,"q"},{17ULL,"r"},{18ULL,"s"},{19ULL,"t"}}) );
    
      undo_all();
      print_state();
      BOOST_TEST_REQUIRE( (_state) == (std::map<uint64_t, std::string>{}) );
      // BOOST_TEST_REQUIRE( (_state) == (std::map<uint64_t, std::string>{{ 0ULL,""},{ 1ULL,""},{ 2ULL,""},{ 3ULL,""},{ 4ULL,""},
      //                                                                  { 5ULL,""},{ 6ULL,""},{ 7ULL,""},{ 8ULL,""},{ 9ULL,""},
      //                                                                  {10ULL,""},{11ULL,""},{12ULL,""},{13ULL,""},{14ULL,""},
      //                                                                  {15ULL,""},{16ULL,""},{17ULL,""},{18ULL,""},{19ULL,""}}) );
      clear_everything();
   } catch (...) {
      throw;
   }
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
   try {
      print_state();
      BOOST_TEST_REQUIRE( (_state) == (std::map<uint64_t, std::string>{}) );
    
      start_undo_session();
      for (size_t i{0}; i < 10; ++i) {
         put(keys1[i], values1[i]);
      }
      print_state();
      BOOST_TEST_REQUIRE( (_state) == (std::map<uint64_t, std::string>{{0ULL,"a"},{1ULL,"b"},{2ULL,"c"},{3ULL,"d"},{4ULL,"e"},
                                                                       {5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );

      start_undo_session();
      for (size_t i{0}; i < 10; ++i) {
         put(keys2[i], values2[i]);
      }
      print_state();
      BOOST_TEST_REQUIRE( (_state) == (std::map<uint64_t, std::string>{{ 0ULL,"a"},{ 1ULL,"b"},{ 2ULL,"c"},{ 3ULL,"d"},{ 4ULL,"e"},
                                                                       { 5ULL,"f"},{ 6ULL,"g"},{ 7ULL,"h"},{ 8ULL,"i"},{ 9ULL,"j"},
                                                                       {10ULL,"k"},{11ULL,"l"},{12ULL,"m"},{13ULL,"n"},{14ULL,"o"},
                                                                       {15ULL,"p"},{16ULL,"q"},{17ULL,"r"},{18ULL,"s"},{19ULL,"t"}}) );

      print_keys();

      BOOST_TEST_REQUIRE( (_stack[_stack.size()-2].new_keys) == (std::set<uint64_t>{0ULL,1ULL,2ULL,3ULL,4ULL,5ULL,6ULL,7ULL,8ULL,9ULL}) );
    
      squash();
      print_keys();
      BOOST_TEST_REQUIRE( (_stack.back().new_keys) == (std::set<uint64_t>{ 0ULL, 1ULL, 2ULL, 3ULL, 4ULL, 5ULL, 6ULL, 7ULL, 8ULL, 9ULL,
                                                                          10ULL,11ULL,12ULL,13ULL,14ULL,15ULL,16ULL,17ULL,18ULL,19ULL}) );
      
      clear_everything();
   } catch (...) {
      throw;
   }
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
   try {
      print_state();
      BOOST_TEST_REQUIRE( (_state) == (std::map<uint64_t, std::string>{}) );
    
      start_undo_session();
      for (size_t i{0}; i < 10; ++i) {
         put(keys1[i], values1[i]);
      }
      print_state();
      BOOST_TEST_REQUIRE( (_state) == (std::map<uint64_t, std::string>{{0ULL,"a"},{1ULL,"b"},{2ULL,"c"},{3ULL,"d"},{4ULL,"e"},
                                                                       {5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );

      start_undo_session();
      for (size_t i{0}; i < 5; ++i) {
         put(keys1[i], values2[i]);
      }
      print_state();
      BOOST_TEST_REQUIRE( (_state) == (std::map<uint64_t, std::string>{{0ULL,"k"},{1ULL,"l"},{2ULL,"m"},{3ULL,"n"},{4ULL,"o"},
                                                                       {5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );
    
      squash();
      print_state();
      BOOST_TEST_REQUIRE( (_state) == (std::map<uint64_t, std::string>{{0ULL,"k"},{1ULL,"l"},{2ULL,"m"},{3ULL,"n"},{4ULL,"o"},
                                                                       {5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );
      clear_everything();
   } catch (...) {
      throw;
   }
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
   try {
      for (size_t i{}; i < 10; ++i) {
         _state[keys1[i]] = values1[i];
      }
      print_state();
      BOOST_TEST_REQUIRE( (_state) == (std::map<uint64_t, std::string>{{0ULL,"a"},{1ULL,"b"},{2ULL,"c"},{3ULL,"d"},{4ULL,"e"},
                                                                       {5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );
    
      start_undo_session();
      for (size_t i{0}; i < 5; ++i) {
         remove(keys1[i]);
      }
      print_state();
      BOOST_TEST_REQUIRE( (_state) == (std::map<uint64_t, std::string>{{5ULL,"f"},{6ULL,"g"},{7ULL,"h"},{8ULL,"i"},{9ULL,"j"}}) );

      start_undo_session();
      for (size_t i{5}; i < 9; ++i) {
         remove(keys1[i]);
      }
      print_state();
      BOOST_TEST_REQUIRE( (_state) == (std::map<uint64_t, std::string>{{9ULL,"j"}}) );
    
      squash();
      print_state();
      BOOST_TEST_REQUIRE( (_state) == (std::map<uint64_t, std::string>{{9ULL,"j"}}) );
      clear_everything();
   } catch (...) {
      throw;
   }
}
