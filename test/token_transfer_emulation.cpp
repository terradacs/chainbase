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

/// Note: use `exponential_distribution` in the future to simulate more
/// accounts with values at the lower bound as opposed to the upper bound.
static const uint64_t lower_bound_inclusive{0};
static const uint64_t upper_bound_inclusive{9};
static std::uniform_int_distribution<uint64_t> uid{lower_bound_inclusive, upper_bound_inclusive};
static std::default_random_engine dre(time(0));

class generated_datum {
public:
   generated_datum(size_t size)
      : _size{size}
   {
      _generate_values(size);
   }

   const uint64_t& operator[](size_t n) const {
      return _values[n];
   }

   void regenerate_values(size_t size) {
      _values.clear();
      _generate_values(size);
   }
   
   const std::vector<uint64_t>& data() const {
      return _values;
   }

   const size_t size() const {
      return _size;
   }

private:
   std::vector<uint64_t> _values;
   size_t _size;

   /// Note: To generate random byte arrays, construct a char table.
   /// Then index randomly into the char array an append it to the given array.
   inline void _generate_values(size_t size) {
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
generated_datum random_keys{10};
generated_datum random_values{10};
generated_datum random_indices{10};

void initial_database_state() {
   // Simulating 300 blocks of account/value initializations.
   // Therefore with 1,000,000 unique accounts and values, there shall be
   // 3,333 new accounts/values per block (AKA `start_undo_session(true)`).
   // After which, the undo session shall be pushed onto the undo stack.
   for (size_t i{}; i < random_keys.size(); ++i) {
      auto session{database.start_undo_session(true)};
      database.put(random_keys[i], std::to_string(random_values[i]));
      keys.insert(random_keys[i]);
      values.insert(random_values[i]);
      session.push();
   }
}

void execution_loop() {
   database.print_state();
   
   for (auto e : random_indices.data()) {
      std::cout << "e:                           " << e                             << "\n";
      std::cout << "e+1%10:                      " << ((e+1)%10)                    << "\n";
      auto tmp{database.get(e).second};
      std::cout << "tmp:                         " << tmp                           << "\n";
      std::cout << "database.get(e+1%10).second: " << database.get((e+1)%10).second << "\n\n";
      
      database.put(e, database.get((e+1)%10).second);
      database.put((e+1)%10, tmp);
   }
   database.print_state();
}

BOOST_AUTO_TEST_CASE(test_one) {
   initial_database_state();
   execution_loop();
   boost::filesystem::remove_all("/Users/john.debord/chainbase/build/test/data");
BOOST_AUTO_TEST_SUITE_END()
