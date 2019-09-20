/**
 *  @file generated_data.cpp
 *  @copyright defined in eosio/LICENSE.txt
 */

#include <iostream> // std::cout|flush

#include "generated_data.hpp"

generated_data::generated_data() = default;
   
generated_data::generated_data(unsigned int seed,
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

void generated_data::init(unsigned int seed,
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

const size_t generated_data::num_of_accounts() const { return _num_of_accounts; }
const size_t generated_data::num_of_swaps() const { return _num_of_swaps; }
const std::vector<arbitrary_datum>& generated_data::accounts() const { return _accounts; }
const std::vector<arbitrary_datum>& generated_data::values() const { return _values; }
const std::vector<size_t>& generated_data::swaps0() const { return _swaps0; }
const std::vector<size_t>& generated_data::swaps1() const { return _swaps1; }

void generated_data::_generate_values() {
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

size_t generated_data::_generate_value() {
   return _uid(_dre);
}
