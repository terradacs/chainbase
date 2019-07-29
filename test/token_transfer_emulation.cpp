#define BOOST_TEST_MODULE token_transfer_emulation_test

#include <ctime> // time

#include <chrono>   // std::chrono::high_resolution_clock
#include <fstream>  // std::ofstream
#include <iostream> // std::cout
#include <limits>   // std::numeric_limits
#include <random>   // std::default_random_engine, std::uniform_int_distribution
#include <set>      // std::set

#include <mach/mach.h>
#include <sys/mount.h>
#include <sys/sysctl.h>

#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE
#include <chainbase/chainrocks.hpp> // chainrocks::database

// The metrics that I want are the following:
// (I think that it's important that I emulate the actually LIB mechanism)
// 1) The performance of continually: start_undo_session -> op -> op -> squash -> repeat
// 2) The performance of continually: start_undo_session -> op -> op -> push   -> pop off for LIB
// 3) Transaction Per Second
// 4) Measuring RAM usage (see what happens when I exceed the RAM limitations)
// 5) Swap in RocksDB and see what happens

// Setup ---> Test (log metrics to a vector) ---> Finish Test ---> Output Metrics To Files

#define MEASURE_START                                                                                 \
   {                                                                                                  \
   std::cout << "-------STARTING MEASUREMENT-------" << '\n';                                         \
   auto initial_time{time(0)};                                                                        \
   std::chrono::high_resolution_clock::time_point t_begin{std::chrono::high_resolution_clock::now()}; \
   std::chrono::high_resolution_clock::time_point t_intermediate{std::chrono::high_resolution_clock::now()};

#define INTERMEDIATE_MEASUREMENT                                                                                      \
   std::chrono::high_resolution_clock::time_point tmp_intermediate{std::chrono::high_resolution_clock::now()};        \
   auto tmp_duration{std::chrono::duration_cast<std::chrono::microseconds>(tmp_intermediate-t_intermediate).count()}; \
   t_intermediate = std::chrono::high_resolution_clock::now();                                                        \
   _milliseconds << time(0)-initial_time << ',' << tmp_duration    << '\n'; \
   _system_metrics  << time(0)-initial_time << ',' << "hi"/*system_metrics{}()*/ << '\n';

#define MEASURE_STOP                                                                                                  \
   std::chrono::high_resolution_clock::time_point tmp_intermediate{std::chrono::high_resolution_clock::now()};        \
   auto tmp_duration{std::chrono::duration_cast<std::chrono::microseconds>(tmp_intermediate-t_intermediate).count()}; \
   _milliseconds << time(0)-initial_time << ',' << tmp_duration;                                                      \
   _system_metrics  << time(0)-initial_time << ',' << "hi"/*system_metrics{}()*/; \
   std::chrono::high_resolution_clock::time_point t_end{std::chrono::high_resolution_clock::now()};                   \
   auto duration{std::chrono::duration_cast<std::chrono::microseconds>(t_end-t_begin).count()};                       \
   _total_time << duration;                                                                                           \
   std::cout << "-------STOPPING MEASUREMENT-------" << '\n';                                                         \
   }

// class logging_vectors {
// public:
// private:
// };

class system_metrics {
public:
   system_metrics()
      : _mach_port{mach_host_self()}
      , _count{sizeof(_vm_stats) / sizeof(natural_t)}
   {
   }

   void total_vm() {
      struct statfs my_stats;
      if (statfs("/", &my_stats) == KERN_SUCCESS) {
         std::cout << "my_stats.f_bsize: " << my_stats.f_bsize << '\n';
         std::cout << "my_stats.f_bfree: " << my_stats.f_bfree << '\n';
         std::cout << "my_stats.f_bsize*stats.f_bfree: " << (my_stats.f_bsize * my_stats.f_bfree) << '\n';
      }
   }

   void total_vm_currently_used() {
      struct xsw_usage my_vmusage;
      size_t size{sizeof(my_vmusage)};
      if (sysctlbyname("vm.swapusage", &my_vmusage, &size, NULL, 0) == KERN_SUCCESS) {
         std::cout << "my_vmusage.xsu_total: " << my_vmusage.xsu_total << '\n';
         std::cout << "my_vmusage.xsu_avail: " << my_vmusage.xsu_avail << '\n';
         std::cout << "my_vmusage.xsu_used: "  << my_vmusage.xsu_used  << '\n';
      }
   }
   
   void total_vm_used_by_proc() {
      struct task_basic_info my_task_info;
      mach_msg_type_number_t my_task_info_count = TASK_BASIC_INFO_COUNT;
      if (task_info(mach_task_self(),
                    TASK_BASIC_INFO,
                    reinterpret_cast<task_info_t>(&my_task_info),
                    &my_task_info_count) == KERN_SUCCESS)
      {
         std::cout << "my_task_info.virtual_size: "  << my_task_info.virtual_size  << '\n';
         std::cout << "my_task_info.resident_size: " << my_task_info.resident_size << '\n';
      }
   }

   void total_ram() {   
      int management_information_base[2]{CTL_HW, HW_MEMSIZE};
      size_t ram;
      size_t size = sizeof(size_t);
      if (sysctl(management_information_base, 2, &ram, &size, NULL, 0) == KERN_SUCCESS) {
         std::cout << "Total RAM on This System: " << ram << '\n';
      }
   }

   void total_ram_currently_used() {
      vm_size_t page_size;
      vm_statistics64_data_t vm_stats;
      mach_port_t mach_port{mach_host_self()};
      mach_msg_type_number_t count{sizeof(vm_stats) / sizeof(natural_t)};
   
      if (host_page_size(mach_port, &page_size) == KERN_SUCCESS &&
          host_statistics64(mach_port, HOST_VM_INFO, reinterpret_cast<host_info64_t>(&vm_stats), &count) == KERN_SUCCESS)
      {
         size_t free_memory{vm_stats.free_count * page_size};

         size_t used_memory{(vm_stats.active_count   +
                             vm_stats.inactive_count +
                             vm_stats.wire_count)    * page_size};

         std::cout << "free_memory: " << free_memory << '\n';
         std::cout << "used_memory: " << used_memory << '\n';
      }
   }

   void calculate_cpu_load() {
      get_cpu_load();
   }

   double get_cpu_load() {
      host_cpu_load_info_data_t cpuinfo;
      mach_msg_type_number_t count{HOST_CPU_LOAD_INFO_COUNT};
   
      if (host_statistics(mach_host_self(), HOST_CPU_LOAD_INFO, reinterpret_cast<host_info_t>(&cpuinfo), &count) == KERN_SUCCESS) {
         size_t total_ticks{};
         for (int i{}; i < CPU_STATE_MAX; i++) {
            total_ticks += cpuinfo.cpu_ticks[i];
         } 
         return calculate_cpu_load(cpuinfo.cpu_ticks[CPU_STATE_IDLE], total_ticks);
      }
      else return -1.0F;
   }

   double calculate_cpu_load(size_t idle_ticks, size_t total_ticks) {
      size_t total_ticks_since_last_time{total_ticks - prev_total_ticks};
      size_t idle_ticks_since_last_time {idle_ticks  - prev_idle_ticks};
   
      double ret{1.0F - ((total_ticks_since_last_time > 0) ? (static_cast<double>(idle_ticks_since_last_time) / total_ticks_since_last_time) : 0)};
   
      prev_total_ticks = total_ticks;
      prev_idle_ticks  = idle_ticks;
      return ret;
   }
   
private:
   static size_t prev_total_ticks;
   static size_t prev_idle_ticks;
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

   inline void print_accounts() { _print_something("_accounts", _accounts); }
   inline void print_values()   { _print_something("_values",   _values);   }
   inline void print_swaps0()   { _print_something("_swaps0",   _swaps0);   }
   inline void print_swaps1()   { _print_something("_swaps1",   _swaps1);   }

   inline const size_t num_of_accounts_and_values() const { return _num_of_accounts_and_values; }
   inline const size_t num_of_swaps()               const { return _num_of_swaps;               }
   
   inline const std::vector<size_t>& accounts() const { return _accounts; }
   inline const std::vector<size_t>& values()   const { return _values;   }
   inline const std::vector<size_t>& swaps0()   const { return _swaps0;   }
   inline const std::vector<size_t>& swaps1()   const { return _swaps1;   }

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

      for (size_t i{}; i < _num_of_accounts_and_values; ++i) {
         _accounts.push_back(_generate_value());
         _values.push_back(_generate_value());
      }

      
      for (size_t i{}; i < _num_of_swaps; ++i) {
         _swaps0.push_back(_generate_value()%_num_of_accounts_and_values);
         _swaps1.push_back(_generate_value()%_num_of_accounts_and_values);
      }

      std::cout << "done.\n";
   }

   inline size_t _generate_value() { return _uid(_dre); }

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
      : _milliseconds  {"/Users/john.debord/chainbase/build/test/milliseconds.csv"}
      , _system_metrics{"/Users/john.debord/chainbase/build/test/system_metrics.csv"}
      , _total_time    {"/Users/john.debord/chainbase/build/test/total_time.csv"}
      , _database      {"/Users/john.debord/chainbase/build/test/data"}
      , _gen_data      {num_of_accounts_and_values, num_of_swaps, lower_bound_inclusive, upper_bound_inclusive}
   {
      _log_data.reserve((_gen_data.num_of_swaps()*0.01)*2);
   }

   ~database_test()
   {
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
   std::ofstream _milliseconds;
   std::ofstream _system_metrics;
   std::ofstream _total_time;
   chainrocks::database _database;
   generated_data _gen_data;
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
      std::cout << "Benchmarking... ";
      MEASURE_START;
      for (size_t i{}; i < _gen_data.num_of_swaps(); ++i) {
         if (__builtin_expect((i % static_cast<size_t>(_gen_data.num_of_swaps() * 0.01)) == 0, false)) {
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
      MEASURE_STOP;
      std::cout << "done.\n";
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
