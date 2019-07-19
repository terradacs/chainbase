#define BOOST_TEST_MODULE token_transfer_emulation_test

#include <ctime> // time

#include <chrono>   // std::chrono::high_resolution_clock
#include <fstream>  // std::ofstream
#include <iostream> // std::cout
#include <random>   // std::default_random_engine, std::uniform_int_distribution
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

std::ofstream outfile{"/Users/john.debord/chainbase2/build/test/outfile"};

template<typename T>
class generated_datum {
public:
   generated_datum(size_t size, size_t upper_bound) {
      _generate_values(size, upper_bound);
   }

   const T& operator[](size_t n) const {
      return _values[n];
   }

   void regenerate_values(size_t size, size_t upper_bound) {
      _values.clear();
      _generate_values(size, upper_bound);
   }

   const size_t& size() const {
      return _size;
   }

   const size_t& upper_bound() const {
      return _upper_bound;
   }

private:
   std::vector<T> _values;
   size_t _size;
   size_t _upper_bound;

   /// Note: To generate random byte arrays, construct a char table.
   /// Then index randomly into the char array an append it to the given array.
   inline void _generate_values(size_t size, size_t upper_bound) {
      /// Note: use `exponential_distribution` in the future to simulate more
      /// accounts with values at the lower bound as opposed to the upper bound.
      std::uniform_int_distribution<T> uid{0, upper_bound};
      std::default_random_engine dre(time(0));
      
      std::cout << "Generating values...\n";
      for (size_t i{}; i < size; ++i) {
         _generate_value(uid, dre);
      }
      std::cout << "Done.\n";
   }

   inline void _generate_value(std::uniform_int_distribution<T>& uid, std::default_random_engine& dre) {
      T value{uid(dre)};
      _values.push_back(value);
   }
};

BOOST_AUTO_TEST_CASE(test_one) {
   size_t                    size  {10000};
   generated_datum<uint64_t> keys  {size, 100};
   generated_datum<uint64_t> values{size, 50};
   generated_datum<uint64_t> ticks {size, 1000};

   chainrocks::index idx;
   // _state:
   idx.print_state(); 
   BOOST_TEST_REQUIRE( (idx.state()) == (std::map<uint64_t, std::string>{}) );
   
   std::vector<decltype(idx.start_undo_session(true))> sessions{};
   
   MEASURE_START("token_transfer_emulation")
   for (size_t i{}; i < size; ++i) {
      if ((ticks[i]%2) == true) {
         idx.put(keys[i], std::to_string(values[i]));
#ifdef LOG
         outfile << "`put(" << keys[i] << ',' << std::to_string(values[i]) << ")`\n";
#endif // LOG
      }
      else {
         idx.remove(keys[i]);
         #ifdef LOG
         outfile << "`remove(" << values[i] << ")`\n";
         #endif
      }

      if ((ticks[i]%10)  == 0) {
         sessions.push_back(idx.start_undo_session(true));
#ifdef LOG
         outfile << "`session`\tstarted\n";
#endif
      }
      
      if ((ticks[i]%100) == 0) {
         if (!sessions.empty()) {
            sessions.back().squash();
#ifdef LOG
            outfile << "`squash`\texecuted\n";
#endif
         }
      }
      if ((ticks[i]%200) == 0) {
         if (!sessions.empty()) {
            sessions.back().undo();
#ifdef LOG
            outfile << "`undo`\t\texecuted\n";
#endif
         }
      }
      if ((ticks[i]%250) == 0 && ticks[i] > 500) {
         if (!sessions.empty()) {
            idx.undo_all();
#ifdef LOG
            outfile << "`undo_all`\texecuted\n";
#endif
         }
      }
      if ((ticks[i]%250) == 0 && ticks[i] < 500) {
         if (!sessions.empty()) {
            idx.commit();
#ifdef LOG
            outfile << "`commit`\texecuted\n";
#endif
         }
      }

      if ((i%10000) == 0) {
         INTERMEDIATE_MEASUREMENT
      }
   }
   MEASURE_STOP
BOOST_AUTO_TEST_SUITE_END()
