#define BOOST_TEST_MODULE token_transfer_emulation_test

#include <ctime> // time

#include <chrono>   // std::chrono::high_resolution_clock
#include <fstream>  // std::ofstream
#include <iostream> // std::cout
#include <random>   // std::default_random_engine, std::uniform_int_distribution
#include <limits>   // std::numeric_limits
#include <set>      // std::set

#include <boost/test/unit_test.hpp>
#include <chainbase/chainrocks.hpp>

#define MEASURE_START(current_test)                                                              \
   {                                                                                             \
   outfile << "-------STARTING MEASUREMENT-------" << '\n';                                      \
   outfile << "Currently testing: " current_test   << '\n';                                      \
   std::chrono::high_resolution_clock::time_point t1{std::chrono::high_resolution_clock::now()}; \

#define INTERMEDIATE_MEASUREMENT                                                                             \
   outfile << "*******STARTING INTERMEDIATE MEASUREMENT*******" << '\n';                                     \
   std::chrono::high_resolution_clock::time_point tmp_time_point{std::chrono::high_resolution_clock::now()}; \
   auto tmp_duration{std::chrono::duration_cast<std::chrono::microseconds>(tmp_time_point-t1).count()};      \
   outfile << "Microseconds: " << tmp_duration << '\n';                                                      \
   outfile << "*******STOPPING INTERMEDIATE MEASUREMENT*******" << "\n\n";                                   \

#define MEASURE_STOP                                                                             \
   std::chrono::high_resolution_clock::time_point t2{std::chrono::high_resolution_clock::now()}; \
   auto duration{std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count()};          \
   outfile << "Microseconds: " << duration     << '\n';                                          \
   outfile << "-------STOPPING MEASUREMENT-------" << "\n\n";                                    \
   }                                                                                             \

class generated_data {
public:
   generated_data(size_t num_of_keys_and_values,
                  size_t num_of_swaps,
                  size_t lower_bound_inclusive,
                  size_t upper_bound_inclusive)
      : _dre{static_cast<unsigned int>(time(0))}
      , _uid{lower_bound_inclusive, upper_bound_inclusive}
      , _num_of_keys_and_values{num_of_keys_and_values}
      , _num_of_swaps{num_of_swaps}
      
   {
      _generate_values();
   }

   void print_accounts() {
      std::cout << "_accounts:\n";
      for (auto e : _accounts) {
         std::cout << e << '\n';
      }
   }

   void print_values() {
      std::cout << "_values:\n";
      for (auto e : _values) {
         std::cout << e << '\n';
      }
   }

   void print_swaps0() {
      std::cout << "_swaps0:\n";
      for (auto e : _swaps0) {
         std::cout << e << '\n';
      }
   }

   void print_swaps1() {
      std::cout << "_swaps1:\n";
      for (auto e : _swaps1) {
         std::cout << e << '\n';
      }
   }

   inline const size_t num_of_keys_and_values() const {
      return _num_of_keys_and_values;
   }

   inline const size_t num_of_swaps() const {
      return _num_of_swaps;
   }
   
   inline const std::vector<size_t>& accounts() const {
      return _accounts;
   }

   inline const std::vector<size_t>& values() const {
      return _values;
   }

   inline const std::vector<size_t>& swaps0() const {
      return _swaps0;
   }

   inline const std::vector<size_t>& swaps1() const {
      return _swaps1;
   }

private:
   std::default_random_engine _dre;
   std::uniform_int_distribution<size_t> _uid;
   
   size_t _num_of_keys_and_values;
   size_t _num_of_swaps;
   
   std::vector<size_t> _accounts;
   std::vector<size_t> _values;
   std::vector<size_t> _swaps0;
   std::vector<size_t> _swaps1;

   inline void _generate_values() {
      std::cout << "Generating values...\n";
      
      size_t count{0};
      while (count < _num_of_keys_and_values) {
         _accounts.push_back(_generate_value());
         _values.push_back(_generate_value());
         ++count;
      }

      count = 0;
      while (count < _num_of_swaps) {
         _swaps0.push_back(_generate_value()%_num_of_keys_and_values);
         _swaps1.push_back(_generate_value()%_num_of_keys_and_values);
         ++count;
      }
      
      std::cout << "Done.\n";
   }

   inline size_t _generate_value() {
      return _uid(_dre);
   }
};

class database_test {
public:
   database_test(size_t num_of_keys_and_values,
                 size_t num_of_swaps,
                 size_t lower_bound_inclusive,
                 size_t upper_bound_inclusive)
      : _output  {"/Users/john.debord/chainbase/build/test/outfile"}
      , _database{"/Users/john.debord/chainbase/build/test/data"}
      , _gen_data{num_of_keys_and_values, num_of_swaps, lower_bound_inclusive, upper_bound_inclusive}
      
   {
   }

   ~database_test() {
      boost::filesystem::remove_all("/Users/john.debord/chainbase/build/test/data");
   }

   inline void start_test() {
      _initial_database_state();
      _execution_loop();
      // Log statistics.
   }
   
private:
   std::ofstream        _output;
   chainrocks::database _database;
   generated_data       _gen_data;

   // Simulating 300 blocks of account/value initializations.
   // Therefore with 1,000,000 unique accounts and values, there shall
   // be 3,333 new accounts/values per block (AKA
   // `start_undo_session(true)`).  After which, the undo session
   // shall be pushed onto the undo stack.
   inline void _initial_database_state() {
      for (size_t i{}; i < _gen_data.num_of_keys_and_values(); ++i) {
         auto session{_database.start_undo_session(true)};
         _database.put(_gen_data.accounts()[i], std::to_string(_gen_data.values()[i]));
         session.push();
      }
   }

   inline void _execution_loop() {
      _database.print_state();
      _gen_data.print_swaps0();
      _gen_data.print_swaps1();
      for (size_t i{}; i < _gen_data.num_of_swaps(); ++i) {
         auto rand_account0{_gen_data.accounts()[_gen_data.swaps0()[i]]};
         auto rand_account1{_gen_data.accounts()[_gen_data.swaps1()[i]]};
         
         auto tmp{_database.get(rand_account0).second};
         _database.put(rand_account0, _database.get(rand_account1).second);
         _database.put(rand_account1, tmp);
      }
      _database.print_state();
   }
};

BOOST_AUTO_TEST_CASE(test_one) {
   database_test dt{10,10,0,10};
   dt.start_test();
   // generated_data gd{1000000, 0, std::numeric_limits<size_t>::max()};
   // gd.print_accounts();
   // gd.print_values();
BOOST_AUTO_TEST_SUITE_END()
