/**
 *  @file generated_data.hpp
 *  @copyright defined in eosio/LICENSE.txt
 */

#pragma once

#include <iostream> // std::cout|flush
#include <random>   // std::default_random_engine|uniform_int_distribution
#include <vector>   // std::vector

#include "common.hpp" // arbitrary_datum

/**
 * Implementation of this test's random data generation facility.
 *
 * Random numbers are generated with a uniform distribution by using
 * the standard's random number generation facilities (this is subject
 * to be cusomizable in the future). It's important to note here, that
 * the user is able to choose how large or small he/she wishes the
 * keys/values to be. But not only can the size be specified, but the
 * filler value can also be specified. The filler value is important
 * to note and must have an appropriate degree of randomness to it. If
 * this weren't so, some database implementations may take advantage
 * of the lack of entropy.
 */
class generated_data {
public:
   generated_data() = default;
   
   generated_data(unsigned int seed,
                  size_t lower_bound_inclusive,
                  size_t upper_bound_inclusive,
                  size_t num_of_accounts,
                  size_t num_of_swaps,
                  size_t max_key_length,
                  size_t max_key_value,
                  size_t max_value_length,
                  size_t max_value_value)
      : _dre{seed}
      , _uid{lower_bound_inclusive, upper_bound_inclusive}
      , _num_of_accounts{num_of_accounts}
      , _num_of_swaps{num_of_swaps}
      , _max_key_length{max_key_length}
      , _max_key_value{max_key_value}
      , _max_value_length{max_value_length}
      , _max_value_value{max_value_value}
   {
      _generate_values();
   }

   void init(unsigned int seed,
             size_t lower_bound_inclusive,
             size_t upper_bound_inclusive,
             size_t num_of_accounts,
             size_t num_of_swaps,
             size_t max_key_length,
             size_t max_key_value,
             size_t max_value_length,
             size_t max_value_value)
   {
      _dre.seed(seed);
      _uid = std::uniform_int_distribution<size_t>{lower_bound_inclusive, upper_bound_inclusive};
      _num_of_accounts = num_of_accounts;
      _num_of_swaps = num_of_swaps;
      _max_key_length = max_key_length;
      _max_key_value = max_key_value;
      _max_value_length = max_value_length;
      _max_value_value = max_value_value;
      _generate_values();
   }

   inline const size_t num_of_accounts() const { return _num_of_accounts; }
   inline const size_t num_of_swaps()    const { return _num_of_swaps;    }

   inline const std::vector<arbitrary_datum>& accounts() const { return _accounts; }
   inline const std::vector<arbitrary_datum>& values()   const { return _values;   }
   inline const std::vector<size_t>&          swaps0()   const { return _swaps0;   }
   inline const std::vector<size_t>&          swaps1()   const { return _swaps1;   }

private:
   std::default_random_engine _dre;
   std::uniform_int_distribution<size_t> _uid;

   size_t _num_of_accounts;
   size_t _num_of_swaps;
   size_t _max_key_length;
   size_t _max_key_value;
   size_t _max_value_length;
   size_t _max_value_value;

   std::vector<arbitrary_datum> _accounts;
   std::vector<arbitrary_datum> _values;
   std::vector<size_t> _swaps0;
   std::vector<size_t> _swaps1;

   inline void _generate_values() {
      clockerman->reset_clocker();

      std::cout << "Generating values...\n" << std::flush;
      loggerman->print_progress(1,0);      

      for (size_t i{}; i < _num_of_accounts; ++i) {
         _accounts.push_back(arbitrary_datum(_uid(_dre)%(_max_key_length  +1), _uid(_dre)%(_max_key_value  +1)));
         _values.push_back  (arbitrary_datum(_uid(_dre)%(_max_value_length+1), _uid(_dre)%(_max_value_value+1)));

         if (UNLIKELY(clockerman->should_log())) {
            loggerman->print_progress(i, _num_of_accounts);
            clockerman->update_clocker();
         }
      }

      for (size_t i{}; i < _num_of_swaps; ++i) {
         _swaps0.push_back(_generate_value()%_num_of_accounts);
         _swaps1.push_back(_generate_value()%_num_of_accounts);

         if (UNLIKELY(clockerman->should_log())) {
            loggerman->print_progress(i, _num_of_accounts);
            clockerman->update_clocker();
         }
      }

      loggerman->print_progress(1,1);
      std::cout << "done.\n" << std::flush;
   }

   inline size_t _generate_value() {
      return _uid(_dre);
   }
};
