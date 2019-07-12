// cd /Users/john.debord/chainbase/test/; g++ -Wall -Wextra -std=c++17 -o prog -lboost_system -lboost_test_exec_monitor token_transfer_emulation.cpp; ./prog

#define BOOST_TEST_MODULE chainrocks test

// #include <cstdlib>
#include <ctime> // time

#include <iostream> // std::cout
#include <random>   // std::default_random_engine, std::uniform_int_distribution
#include <set>      // std::set
// #include <boost/test/unit_test.hpp>
// #include <chainbase/chainrocks.hpp>

template<typename T>
class generated_datum {
public:
   generated_datum(size_t size, size_t upper_bound) {
      _generate_values(size, upper_bound);
   }

   void regenerate_values(size_t size, size_t upper_bound) {
      _values.clear();
      _generate_values(size, upper_bound);
   }

   const std::set<T>& data() const {
      return _values;
   }

   const size_t& size() const {
      return _size;
   }

   const size_t& upper_bound() const {
      return _upper_bound;
   }

private:
   std::set<T> _values;
   size_t      _size;
   size_t      _upper_bound;

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
      _values.insert(value);
   }
};

int main() {
   generated_datum<uint64_t> keys(100000, 10);

   return 0;
}
// BOOST_AUTO_TEST_CASE(test_one) {

// } catch (...) {
//    throw;
// }
