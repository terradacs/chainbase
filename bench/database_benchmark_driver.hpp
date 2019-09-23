/**
 *  @file database_benchmark_driver.hpp
 *  @copyright defined in eosio/LICENSE.txt
 */

#include "database_benchmark_interface.hpp"

template<typename Database>
database_benchmark<Database>::database_benchmark(window window)
   : _window{window}
{
}

template<typename Database>
database_benchmark<Database>::~database_benchmark() = default;

template<typename Database>
void database_benchmark<Database>::set_program_options(boost::program_options::options_description& cli) {
   cli.add_options()
      ("seed,s",
       boost::program_options::value<unsigned int>(&_seed)->default_value(42),
       "Seed value for the random number generator.")

      ("lower-bound,l",
       boost::program_options::value<size_t>(&_lower_bound_inclusive)->default_value(std::numeric_limits<size_t>::min()),
       "Seed value for the random number generator.")

      ("upper-bound,u",
       boost::program_options::value<size_t>(&_upper_bound_inclusive)->default_value(std::numeric_limits<size_t>::max()),
       "Seed value for the random number generator.")
         
      ("num-of-accounts,n",
       boost::program_options::value<size_t>(&_num_of_accounts)->default_value(100000),
       "Number of unique individual accounts with a corresponding value; key/value pair.")
         
      ("num-of-swaps,w",
       boost::program_options::value<size_t>(&_num_of_swaps)->default_value(100000),
       "Number of swaps to perform during the benchmark.")
         
      ("max-key-length,k",
       boost::program_options::value<size_t>(&_max_key_length)->default_value(1023),
       "Maximum key byte vector size.")
         
      ("max-key-value,y",
       boost::program_options::value<size_t>(&_max_key_value)->default_value(255),
       "Maximum value to fill the key byte vector with.")
         
      ("max-value-length,v",
       boost::program_options::value<size_t>(&_max_value_length)->default_value(1023),
       "Maximum value byte vector size.")

      ("max-value-value,e",
       boost::program_options::value<size_t>(&_max_value_value)->default_value(255),
       "Maximum value to fill the value byte vector with.")
         
      ("help,h", "Print this help message and exit.");
}

template<typename Database>
void database_benchmark<Database>::execute_benchmark() {
   _gen_data.init(_seed,
                  _lower_bound_inclusive,
                  _upper_bound_inclusive,
                  _num_of_accounts,
                  _num_of_swaps,
                  _max_key_length,
                  _max_key_value,
                  _max_value_length,
                  _max_value_value);
      
   _initial_database_state();
   _execution_loop();
   loggerman->flush_all();
}

template<typename Database>
void database_benchmark<Database>::_initial_database_state() {
   clockerman->reset_clocker();

   std::cout << "Filling initial database state...\n" << std::flush;
   loggerman->print_progress(1,0);
      
   for (size_t i{}; i < _gen_data.num_of_accounts()/10; ++i) {
      if (UNLIKELY(clockerman->should_log())) {
         loggerman->log_tps      ({clockerman->seconds_since_start_of_benchmark(), 0});
         loggerman->log_ram_usage({clockerman->seconds_since_start_of_benchmark(), _system_metrics.total_ram_currently_used()});
         loggerman->log_cpu_load ({clockerman->seconds_since_start_of_benchmark(), _system_metrics.calculate_cpu_load()});
         loggerman->print_progress(i, _gen_data.num_of_accounts()/10);
         clockerman->update_clocker();
      }

      _database.start_undo_session(true);

      auto session{_database.start_undo_session(true)};
      _database.start_undo_session(true);
      // Create 10 new accounts per undo session.
      // AKA; create 10 new accounts per block.
      for (size_t j{}; j < 10; ++j) {
         _database.put(_gen_data.accounts()[i*10+j], _gen_data.values()[i*10+j]);
      }
      session.push();
   }

   loggerman->print_progress(1,1);
   _database.start_undo_session(true).push();
   _database.write();
   std::cout << "done.\n" << std::flush;
}

template<typename Database>
void database_benchmark<Database>::_execution_loop() {
   size_t transactions_per_second{};

   std::cout << "Benchmarking...\n" << std::flush;
   loggerman->print_progress(1,0);
      
   for (size_t i{}; i < _gen_data.num_of_swaps(); ++i) {
      if (UNLIKELY(clockerman->should_log())) {
         switch (_window) {
             case window::expanding_window:
                loggerman->log_tps({clockerman->seconds_since_start_of_benchmark(), _expanding_window_metric(transactions_per_second)});
                clockerman->update_clocker();
                break;
             case window::narrow_window:
                loggerman->log_tps({clockerman->seconds_since_start_of_benchmark(), _narrow_window_metric(transactions_per_second)});
                clockerman->update_clocker();
                transactions_per_second = 0;
                break;
             case window::rolling_window:
                loggerman->log_tps({clockerman->seconds_since_start_of_benchmark(), _rolling_window_metric(transactions_per_second)});
                clockerman->update_clocker();
                transactions_per_second = 0;
                break;
             default:
                throw std::runtime_error{"database_benchmark::should_log()"};
                break;
         }
            
         loggerman->log_ram_usage({clockerman->seconds_since_start_of_benchmark(), _system_metrics.total_ram_currently_used()});
         loggerman->log_cpu_load ({clockerman->seconds_since_start_of_benchmark(), _system_metrics.calculate_cpu_load()});
         loggerman->print_progress(i, _gen_data.num_of_swaps());
      }

      auto session{_database.start_undo_session(true)};
      _database.swap(_gen_data, i);
      session.squash();
      transactions_per_second += 2;
   }

   loggerman->print_progress(1,1);
   _database.write();
   std::cout << "done.\n" << std::flush;
}

template<typename Database>
size_t database_benchmark<Database>::_expanding_window_metric(size_t tps) {
   return tps/clockerman->expanding_window();
}

template<typename Database>
size_t database_benchmark<Database>::_narrow_window_metric(size_t tps) {
   return tps/clockerman->narrow_window();
}

template<typename Database>
size_t database_benchmark<Database>::_rolling_window_metric(size_t tps) {
   return clockerman->rolling_window(tps/clockerman->narrow_window());
}
