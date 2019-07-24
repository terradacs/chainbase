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

#define LOG

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

class generated_datum {
public:
   generated_datum(size_t size, size_t upper_bound_inclusive)
      : _size{size}
      , _upper_bound_inclusive{upper_bound_inclusive}
   {
      _generate_values(size, upper_bound_inclusive);
   }

   const uint64_t& operator[](size_t n) const {
      return _values[n];
   }

   void regenerate_values(size_t size, size_t upper_bound_inclusive) {
      _values.clear();
      _generate_values(size, upper_bound_inclusive);
   }
   
   const std::vector<uint64_t>& data() const {
      return _values;
   }

   const size_t size() const {
      return _size;
   }

   const size_t upper_bound() const {
      return _upper_bound_inclusive;
   }

private:
   std::vector<uint64_t> _values;
   size_t _size;
   size_t _upper_bound_inclusive;

   /// Note: To generate random byte arrays, construct a char table.
   /// Then index randomly into the char array an append it to the given array.
   inline void _generate_values(size_t size, size_t upper_bound_inclusive) {
      /// Note: use `exponential_distribution` in the future to simulate more
      /// accounts with values at the lower bound as opposed to the upper bound.
      std::uniform_int_distribution<uint64_t> uid{0, upper_bound_inclusive};
      std::default_random_engine dre(time(0));
      
      std::cout << "Generating values...\n";
      size_t count{0};
      while (count < size) {
         uint64_t value{uid(dre)};
         auto iter{std::find(_values.cbegin(), _values.cend(), value)};
         if (iter == _values.cend()) {
            _values.push_back(value);
            ++count;
         }
         else {
            continue;
         }
      }
      std::cout << "Done.\n";
   }
};

std::ofstream outfile{"/Users/john.debord/chainbase/build/test/outfile"};
chainrocks::database database{"/Users/john.debord/chainbase/build/test/data"};
std::set<uint64_t> keys{};
std::set<uint64_t> values{};
generated_datum random_keys{10, std::numeric_limits<size_t>::max()};
generated_datum random_values{10, std::numeric_limits<size_t>::max()};
generated_datum random_indexes{10, 9};

void initial_database_state() {
   // Simulating 300 blocks of account/value initializations.
   // Therefore with 1,000,000 unique accounts and values, there shall be
   // 3,333 new accounts/values per block (AKA `start_undo_session(true)`).
   // After which, the undo session shall be pushed onto the undo stack.
   for (size_t i{}; i < keys.size(); ++i) {
      auto session{database.start_undo_session(true)};
      database.put(random_keys[i], std::to_string(random_values[i]));
      keys.insert(random_keys[i]);
      values.insert(random_values[i]);
      session.push();
   }
}

/// Stuff.
void execution_loop() {
   for (auto e : random_indexes.data()) {
      database.put(*keys.cbegin()+e, std::to_string(*values.cbegin()+e+1%10));
      database.put(*keys.cbegin()+(e+1)%10, std::to_string(*values.cbegin()+e));
   }
}

/// Stuff.
BOOST_AUTO_TEST_CASE(test_one) {
   std::cout << "ok\n";
   initial_database_state();
   execution_loop();
   // size_t                    size  {1000};
//    generated_datum<uint64_t> keys  {size, 100};
//    generated_datum<uint64_t> values{size, 50};
//    generated_datum<uint64_t> ticks {size, 1000};

//    chainrocks::database database{"/Users/john.debord/chainbase/build/test/data"};
//    // _state:
//    database.print_state(); 
//    BOOST_TEST_REQUIRE( (database.state()) == (std::map<uint64_t, std::string>{}) );
   
//    std::vector<decltype(database.start_undo_session(true))> sessions{};
   
//    MEASURE_START("token_transfer_emulation")
//    for (size_t i{}; i < size; ++i) {
//       if ((ticks[i]%2) == true) {
//          database.put(keys[i], std::to_string(values[i]));
// #ifdef LOG
//          outfile << "`put(" << keys[i] << ',' << std::to_string(values[i]) << ")`\n";
// #endif // LOG
//       }
//       else {
//          database.remove(keys[i]);
//          #ifdef LOG
//          outfile << "`remove(" << values[i] << ")`\n";
//          #endif
//       }

//       if ((ticks[i]%10)  == 0) {
//          sessions.push_back(database.start_undo_session(true));
// #ifdef LOG
//          outfile << "`session`\tstarted\n";
// #endif // LOG
//       }
      
//       if ((ticks[i]%100) == 0) {
//          if (!sessions.empty()) {
//             sessions.back().squash();
// #ifdef LOG
//             outfile << "`squash`\texecuted\n";
// #endif // LOG
//          }
//       }
//       if ((ticks[i]%200) == 0) {
//          if (!sessions.empty()) {
//             sessions.back().undo();
// #ifdef LOG
//             outfile << "`undo`\t\texecuted\n";
// #endif // LOG
//          }
//       }
//       if ((ticks[i]%250) == 0 && ticks[i] > 500) {
//          if (!sessions.empty()) {
//             database.undo_all();
// #ifdef LOG
//             outfile << "`undo_all`\texecuted\n";
// #endif // LOG
//          }
//       }
//       if ((ticks[i]%250) == 0 && ticks[i] < 500) {
//          if (!sessions.empty()) {
//             database.commit();
// #ifdef LOG
//             outfile << "`commit`\texecuted\n";
// #endif // LOG
//          }
//       }

//       if ((i%10000) == 0) {
//          INTERMEDIATE_MEASUREMENT
//       }
//    }
//    MEASURE_STOP
//    boost::filesystem::remove_all("/Users/john.debord/chainbase/build/test/data");
   boost::filesystem::remove_all("/Users/john.debord/chainbase/build/test/data");
BOOST_AUTO_TEST_SUITE_END()
