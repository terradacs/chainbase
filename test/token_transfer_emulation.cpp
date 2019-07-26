#define BOOST_TEST_MODULE token_transfer_emulation_test

#include <mach/mach_host.h>
#include <mach/mach_init.h>
#include <mach/mach_types.h>
#include <mach/vm_statistics.h>

#include <ctime> // time

#include <chrono>   // std::chrono::high_resolution_clock
#include <fstream>  // std::ofstream
#include <iostream> // std::cout
#include <random>   // std::default_random_engine, std::uniform_int_distribution
#include <limits>   // std::numeric_limits
#include <set>      // std::set

#include <boost/test/unit_test.hpp>
#include <chainbase/chainrocks.hpp>

#define MEASURE_START                                                                                 \
   {                                                                                                  \
   std::cout << "-------STARTING MEASUREMENT-------" << '\n';                                         \
   std::chrono::high_resolution_clock::time_point t_begin{std::chrono::high_resolution_clock::now()}; \
   std::chrono::high_resolution_clock::time_point t_intermediate{std::chrono::high_resolution_clock::now()};

#define INTERMEDIATE_MEASUREMENT                                                                                      \
   std::chrono::high_resolution_clock::time_point tmp_intermediate{std::chrono::high_resolution_clock::now()};        \
   auto tmp_duration{std::chrono::duration_cast<std::chrono::microseconds>(tmp_intermediate-t_intermediate).count()}; \
   t_intermediate = std::chrono::high_resolution_clock::now();                                                        \
   _milliseconds << tmp_duration << ',';

#define MEASURE_STOP                                                                                                  \
   std::chrono::high_resolution_clock::time_point tmp_intermediate{std::chrono::high_resolution_clock::now()};        \
   auto tmp_duration{std::chrono::duration_cast<std::chrono::microseconds>(tmp_intermediate-t_intermediate).count()}; \
   _milliseconds << tmp_duration;                                                                                     \
   std::chrono::high_resolution_clock::time_point t_end{std::chrono::high_resolution_clock::now()};                   \
   auto duration{std::chrono::duration_cast<std::chrono::microseconds>(t_end-t_begin).count()};                       \
   _total_time << duration;                                                                                           \
   std::cout << "-------STOPPING MEASUREMENT-------" << '\n';                                                         \
   }

class ram_metrics {
public:
   ram_metrics()
      : _mach_port{mach_host_self()}
      , _count{sizeof(_vm_stats) / sizeof(natural_t)}
   {
   }

   void operator()() {
      if (KERN_SUCCESS == host_page_size(_mach_port, &_vm_page_size) &&
          KERN_SUCCESS == host_statistics64(_mach_port, HOST_VM_INFO, (host_info64_t)&_vm_stats, &_count))
      {
         long long free_memory = (int64_t)_vm_stats.free_count * (int64_t)_vm_page_size;

         long long used_memory = ((int64_t)_vm_stats.active_count   +
                                  (int64_t)_vm_stats.inactive_count +
                                  (int64_t)_vm_stats.wire_count)    * (int64_t)_vm_page_size;
         printf("free memory: %lld\nused memory: %lld\n", free_memory, used_memory);
      }
   }
   
private:
   vm_size_t              _vm_page_size;
   vm_statistics64_data_t _vm_stats;
   mach_port_t            _mach_port;
   mach_msg_type_number_t _count;
};

class generated_data {
public:
   generated_data(size_t num_of_accounts_and_values,
                  size_t num_of_swaps,
                  size_t lower_bound_inclusive,
                  size_t upper_bound_inclusive)
      : _dre{static_cast<unsigned int>(time(0))}
      , _uid{lower_bound_inclusive, upper_bound_inclusive}
      , _num_of_accounts_and_values{num_of_accounts_and_values}
      , _num_of_swaps{num_of_swaps}

   {
      _generate_values();
   }

   inline void print_accounts() {
      _print_something("_accounts", _accounts);
   }

   inline void print_values() {
      _print_something("_values", _values);
   }

   inline void print_swaps0() {
      _print_something("_swaps0", _swaps0);
   }

   inline void print_swaps1() {
      _print_something("_swaps1", _swaps1);
   }

   inline const size_t num_of_accounts_and_values() const {
      return _num_of_accounts_and_values;
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

   size_t _num_of_accounts_and_values;
   size_t _num_of_swaps;

   std::vector<size_t> _accounts;
   std::vector<size_t> _values;
   std::vector<size_t> _swaps0;
   std::vector<size_t> _swaps1;

   inline void _generate_values() {
      std::cout << "Generating values... ";

      size_t count{0};
      while (count < _num_of_accounts_and_values) {
         _accounts.push_back(_generate_value());
         _values.push_back(_generate_value());
         ++count;
      }

      count = 0;
      while (count < _num_of_swaps) {
         _swaps0.push_back(_generate_value()%_num_of_accounts_and_values);
         _swaps1.push_back(_generate_value()%_num_of_accounts_and_values);
         ++count;
      }

      std::cout << "done.\n";
   }

   inline size_t _generate_value() {
      return _uid(_dre);
   }

   inline void _print_something(const std::string& vec_name, const std::vector<size_t>& vec) {
      std::cout << vec_name << ": \n";
      size_t count{};
      for (auto e : vec) {
         std::cout << e << '\t';
         ++count;
         if ((count % 10) == 0) {
            std::cout << '\n';
         }
      }
   }
};

class database_test {
public:
   database_test(size_t num_of_accounts_and_values,
                 size_t num_of_swaps,
                 size_t lower_bound_inclusive,
                 size_t upper_bound_inclusive)
      : _milliseconds{"/Users/john.debord/chainbase/build/test/milliseconds"}
      , _ram_usage   {"/Users/john.debord/chainbase/build/test/ram_usage"}
      , _total_time  {"/Users/john.debord/chainbase/build/test/total_time"}
      , _database    {"/Users/john.debord/chainbase/build/test/data"}
      , _gen_data    {num_of_accounts_and_values, num_of_swaps, lower_bound_inclusive, upper_bound_inclusive}
   {
      _log_data.reserve((_gen_data.num_of_swaps()*0.01)*2);
   }

   ~database_test() {
      boost::filesystem::remove_all("/Users/john.debord/chainbase/build/test/data");
   }

   inline void print_everything() {
      _gen_data.print_accounts();
      _gen_data.print_values();
      _gen_data.print_swaps0();
      _gen_data.print_swaps1();
   }

   inline void start_test() {
      _initial_database_state();
      _execution_loop();
   }

private:
   std::ofstream                          _milliseconds;
   std::ofstream                          _ram_usage;
   std::ofstream                          _total_time;
   chainrocks::database                   _database;
   generated_data                         _gen_data;
   std::vector<std::chrono::microseconds> _log_data{};

   inline void _initial_database_state() {
      std::cout << "Filling initial database state... ";
      for (size_t i{}; i < _gen_data.num_of_accounts_and_values(); ++i) {
         auto session{_database.start_undo_session(true)};
         _database.put(_gen_data.accounts()[i], std::to_string(_gen_data.values()[i]));
         session.push();
      }
      _database.start_undo_session(true).push();
      std::cout << "done.\n";
   }

   inline void _execution_loop() {
      MEASURE_START;
      std::cout << "Benchmarking... ";
      for (size_t i{}; i < _gen_data.num_of_swaps(); ++i) {
         if (__builtin_expect(((size_t)i % (size_t)(_gen_data.num_of_swaps() * 0.01)) == 0, false)) {
            INTERMEDIATE_MEASUREMENT;
         }

         auto rand_account0{_gen_data.accounts()[_gen_data.swaps0()[i]]};
         auto rand_account1{_gen_data.accounts()[_gen_data.swaps1()[i]]};

         auto session{_database.start_undo_session(true)};
         auto tmp{_database.get(rand_account0).second};
         _database.put(rand_account0, _database.get(rand_account1).second);
         _database.put(rand_account1, tmp);
         session.squash();
      }
         std::cout << "done.\n";
         MEASURE_STOP;
   }
};

BOOST_AUTO_TEST_CASE(test_one) {
   const static size_t num_of_accounts_and_values{1000000};
   const static size_t num_of_swaps{1000000};
   const static size_t lower_bound_inclusive{0};
   const static size_t upper_bound_inclusive{std::numeric_limits<size_t>::max()};

   database_test dt{num_of_accounts_and_values, num_of_swaps, lower_bound_inclusive, upper_bound_inclusive};
   dt.start_test();
BOOST_AUTO_TEST_SUITE_END()
