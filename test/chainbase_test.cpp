#include <ctime> // time

#include <algorithm> // std::accumulate
#include <chrono>    // std::chrono::time_point, std::chrono::system_clock::now
#include <fstream>   // std::ofstream
#include <iomanip>   // std::setw
#include <iostream>  // std::cout, std::flush
#include <limits>    // std::numeric_limits
#include <random>    // std::default_random_engine, std::uniform_int_distribution
#include <set>       // std::set

#ifdef __APPLE__
#include <mach/mach.h>
#include <sys/mount.h>
#include <sys/sysctl.h>
#endif // __APPLE__

#ifdef __linux__
// ...
#endif // __linux__

#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/test/unit_test.hpp>
#include <chainbase/chainbase.hpp>

using namespace boost::multi_index;

struct account;
class logger;
class clocker;
class system_metrics;
class generated_data;
class timer;
class database_test;

/**
 * Unused variables; may need for future use when a more
 * comprehensible user-friendly API gets developed.
 */
__attribute__((unused)) static const size_t byte{1};
__attribute__((unused)) static const size_t kilobyte{byte*1024};
__attribute__((unused)) static const size_t megabyte{kilobyte*1024};
__attribute__((unused)) static const size_t gigabyte{megabyte*1024};

/**
 * Alias for which to describe a vector of random bytes. Since this
 * test doesn't care what the data is, we use an arbitrary amount of
 * bytes with arbitrary values to be used as fillers for benchmarking
 * the database.
 */
using arbitrary_datum = std::vector<uint8_t>;

/**
 * Logging facility to handle all logging operations. Ranging from
 * printing output to the console to logging the metrics to their
 * respective files.
 */
static std::unique_ptr<logger> loggerman;

/**
 * Clocking facility to handle the logic of when to log the tests
 * specific metrics.
 */
static std::unique_ptr<clocker> clockerman;

/**
 * Data structure to be used for testing `chainbase'. In the future
 * this should be separated out from the test itself and be used as a
 * template parameter to class `database_test' itself.
 */
struct account : public chainbase::object<0,account> {
   template<typename Constructor, typename Allocator>
   account(Constructor&& c, Allocator&& a) { c(*this); }
   id_type id;
   arbitrary_datum _account_key;
   arbitrary_datum _account_value;
};

/**
 * Boiler plate type-alias used for testing `chainbase'.
 */
using account_index = multi_index_container<
   account,
   indexed_by<
      ordered_unique<member<account,account::id_type,&account::id>>,
      ordered_non_unique<BOOST_MULTI_INDEX_MEMBER(account,arbitrary_datum,_account_key)>,
      ordered_non_unique<BOOST_MULTI_INDEX_MEMBER(account,arbitrary_datum,_account_value)>
   >,
   chainbase::allocator<account>
>;
CHAINBASE_SET_INDEX_TYPE(account, account_index)

/**
 * Implementation of this test's logging facility.
 *
 * All logging, but logging the test's current progression status, is
 * deferred until the end of the test. Where the data is flushed to
 * its respective file as a comma-separated list. The supported
 * metrics are as follows: Transactions-Per-Second (with differing
 * variations of measuring it over time), CPU Usage, and RAM Usage.
 */
class logger {
public:
   logger()
      : _data_file{(boost::filesystem::current_path() /= std::string{"/data.csv"}).string()}
   {
      _tps.reserve(1000);
      _ram_usage.reserve(1000);
      _cpu_load.reserve(1000);
   }

   void print_progress(size_t n, size_t m) {
      if (m == 0) {
         std::cout << '[' << std::setw(3) << 0 << "%]\n";
         return;
      }
      std::cout << '[' << std::setw(3) << (static_cast<size_t>(static_cast<double>(n)/m*100.0)) << "%]\n";
   }

   void flush_all() {
      for (size_t i{}; i < _tps.size(); ++i) {
         _data_file << std::setw(10) << _tps[i].first        << '\t';
         _data_file << std::setw(10) << _tps[i].second       << '\t';
         _data_file << std::setw(10) << _cpu_load[i].second  << '\t';
         _data_file << std::setw(10) << _ram_usage[i].second << '\n';
         _data_file << std::flush;
      }
   }

   inline void log_tps(const std::pair<size_t,size_t>& p)       { _tps.push_back(p);       }
   inline void log_ram_usage(const std::pair<size_t,double>& p) { _ram_usage.push_back(p); }
   inline void log_cpu_load(const std::pair<size_t,double>& p)  { _cpu_load.push_back(p);  }

private:
   std::vector<std::pair<size_t,size_t>> _tps;
   std::vector<std::pair<size_t,double>> _ram_usage;
   std::vector<std::pair<size_t,double>> _cpu_load;
   std::ofstream _data_file;
};

/**
 * Implementation of this test's clocking facility.
 *
 * The clocking facility is responsible for all things related to time
 * and the differing ways of returning/interpreting the time for the
 * need of alternative ways to measuring data.
 */
class clocker {
public:
   clocker(size_t interval_in_seconds)
      : _interval_in_seconds{interval_in_seconds*1000}
   {
   }

   void reset_clocker() {
      _original_time = _retrieve_time();
      _old_time      = _original_time;
      _new_time      = _old_time;
   }

   inline bool should_log() {
      _new_time = _retrieve_time();

      if ((_new_time - _old_time) > _interval_in_seconds) {
         return true;
      }
      else {
         return false;
      }
   }

   inline void update_clocker() {
      _old_time = _new_time;
   }

   inline size_t seconds_since_start_of_test() {
      return ((_new_time - _original_time)/1000);
   }

   inline size_t expanding_window() {
      return  seconds_since_start_of_test();
   }

   inline size_t narrow_window() {
      return ((_new_time - _old_time)/1000);
   }

   // TODO
   // // Currently only a 5 second rolling window is provided.
   // inline size_t rolling_window() {
   //    return  seconds_since_start_of_test();
   // }
   
private:
   size_t _interval_in_seconds;
   size_t _original_time;
   size_t _old_time;
   size_t _new_time;

   // TODO
   // struct rolling_average {
   //    std::vector<size_t> _last_five_terms{0,0,0,0,0}; // Will be `_last_n_terms'.

   //    void push_term() {
   //    }

   //    size_t get_rolling_average() {
   //       return (std::accumulate(_last_five_terms.crbegin(),_last_five_terms.cbegin()+5,0) / 5);
   //    }
   // };

   inline long long _retrieve_time() {
      std::chrono::time_point tp{std::chrono::high_resolution_clock::now()};
      std::chrono::milliseconds d{std::chrono::time_point_cast<std::chrono::milliseconds>(tp).time_since_epoch()};
      long long ms{std::chrono::duration_cast<std::chrono::milliseconds>(d).count()};
      return ms;
   }
};

/**
 * Implementation of this test's system-metric measuring facilities.
 *
 * Only determining specific metrics for __APPLE__ computers are used
 * as of late. The current set of the most useful metrics are as
 * follows: `total_vm_currently_used' (determines how much Virtual
 * Memory is currently in use by the system), `total_vm_used_by_proc'
 * (determines how much Virtual Memory is currently in use by the
 * current process), `total_ram_currently_used' (determines how much
 * RAM is currently in use by the system), `get_cpu_load (determines
 * the current load the CPU is experiencing)'.
 */
class system_metrics {
public:
   system_metrics()
      : _prev_total_ticks{}
      , _prev_idle_ticks{}
   {
   }

#ifdef __APPLE__
   // Deprioritize.
   void total_vm() {
      struct statfs my_stats;
      if (statfs("/", &my_stats) == KERN_SUCCESS) {
         std::cout << "my_stats.f_bsize: " << my_stats.f_bsize << '\n';
         std::cout << "my_stats.f_bfree: " << my_stats.f_bfree << '\n';
         std::cout << "my_stats.f_bsize*stats.f_bfree: " << (my_stats.f_bsize * my_stats.f_bfree) << '\n';
      }
   }

   // Deprioritize.
   void total_vm_currently_used() {
      struct xsw_usage my_vmusage;
      size_t size{sizeof(my_vmusage)};
      if (sysctlbyname("vm.swapusage", &my_vmusage, &size, NULL, 0) == KERN_SUCCESS) {
         std::cout << "my_vmusage.xsu_total: " << my_vmusage.xsu_total << '\n';
         std::cout << "my_vmusage.xsu_avail: " << my_vmusage.xsu_avail << '\n';
         std::cout << "my_vmusage.xsu_used: "  << my_vmusage.xsu_used  << '\n';
      }
   }

   // Deprioritize.
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

   // Deprioritize.
   size_t total_ram() {
      int management_information_base[2]{CTL_HW, HW_MEMSIZE};
      size_t ram;
      size_t size = sizeof(size_t);
      if (sysctl(management_information_base, 2, &ram, &size, NULL, 0) == KERN_SUCCESS) {
         return ram;
      }
      else {
         return 0;
      }
   }

   // Prioritize.
   double total_ram_currently_used() {
      vm_size_t page_size;
      vm_statistics64_data_t vm_stats;
      mach_port_t mach_port{mach_host_self()};
      mach_msg_type_number_t count{sizeof(vm_stats) / sizeof(natural_t)};

      if (host_page_size(mach_port, &page_size) == KERN_SUCCESS &&
          host_statistics64(mach_port, HOST_VM_INFO, reinterpret_cast<host_info64_t>(&vm_stats), &count) == KERN_SUCCESS)
      {
         size_t used_memory{(vm_stats.active_count   +
                             vm_stats.inactive_count +
                             vm_stats.wire_count)    * page_size};
         return (static_cast<double>(used_memory) / total_ram());
      }
      else {
         return 0;
      }
   }

   // Deprioritize.
   double calculate_cpu_load() {
      return get_cpu_load();
   }

   // Deprioritize.
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

   // Deprioritize.
   double calculate_cpu_load(size_t idle_ticks, size_t total_ticks) {
      size_t total_ticks_since_last_time{total_ticks - _prev_total_ticks};
      size_t idle_ticks_since_last_time {idle_ticks  - _prev_idle_ticks};

      double ret{1.0F - ((total_ticks_since_last_time > 0) ? (static_cast<double>(idle_ticks_since_last_time) / total_ticks_since_last_time) : 0)};

      _prev_total_ticks = total_ticks;
      _prev_idle_ticks  = idle_ticks;
      return ret;
   }
#endif // __APPLE__

#ifdef __linux__
// ...
#endif // __linux__

private:
   size_t _prev_total_ticks;
   size_t _prev_idle_ticks;
};

/**
 * Implementation of this test's random data generation facility.
 *
 * Random numbers are generated with a uniform distribution (this is
 * subject to be cusomizable in the future) by using the standards
 * random number generation facilities. It's important to note here,
 * that the user is able to choose how large or small he/she wishes
 * the keys/values to be. But not only can the size be specified, but
 * the filler value can also be specified. The filler value is
 * important to note and must have an appropriate degree of randomness
 * to it. If this weren't so, some database implementations may take
 * advantage of the lack of entropy.
 */
class generated_data {
public:
   generated_data(unsigned int seed,
                  size_t lower_bound_inclusive,
                  size_t upper_bound_inclusive,
                  size_t num_of_acc_and_vals,
                  size_t num_of_swaps,
                  size_t max_key_length,
                  size_t max_key_value,
                  size_t max_value_length,
                  size_t max_value_value)
      : _dre{seed}
      , _uid{lower_bound_inclusive, upper_bound_inclusive}
      , _num_of_acc_and_vals{num_of_acc_and_vals}
      , _num_of_swaps{num_of_swaps}
      , _max_key_length{max_key_length}
      , _max_key_value{max_key_value}
      , _max_value_length{max_value_length}
      , _max_value_value{max_value_value}
   {
      _generate_values();
   }

   inline const size_t num_of_acc_and_vals() const { return _num_of_acc_and_vals; }
   inline const size_t num_of_swaps()        const { return _num_of_swaps;        }

   inline const std::vector<arbitrary_datum>& accounts() const { return _accounts; }
   inline const std::vector<arbitrary_datum>& values()   const { return _values;   }
   inline const std::vector<size_t>&          swaps0()   const { return _swaps0;   }
   inline const std::vector<size_t>&          swaps1()   const { return _swaps1;   }

private:
   std::default_random_engine _dre;
   std::uniform_int_distribution<size_t> _uid;

   size_t _num_of_acc_and_vals;
   size_t _num_of_swaps;
   size_t _max_key_length;
   size_t _max_key_value;
   size_t _max_value_length;
   size_t _max_value_value;

   std::vector<arbitrary_datum> _accounts;
   std::vector<arbitrary_datum> _values;
   std::vector<size_t>          _swaps0;
   std::vector<size_t>          _swaps1;

   inline void _generate_values() {
      std::cout << "Generating values...\n" << std::flush;
      loggerman->print_progress(1,0);
      clockerman->reset_clocker();

      for (size_t i{}; i < _num_of_acc_and_vals; ++i) {
         _accounts.push_back(arbitrary_datum(_uid(_dre)%(_max_key_length  +1), _uid(_dre)%(_max_key_value  +1)));
         _values.push_back  (arbitrary_datum(_uid(_dre)%(_max_value_length+1), _uid(_dre)%(_max_value_value+1)));

         if (clockerman->should_log()) {
            loggerman->print_progress(i, _num_of_acc_and_vals);
         }
      }

      for (size_t i{}; i < _num_of_swaps; ++i) {
         _swaps0.push_back(_generate_value()%_num_of_acc_and_vals);
         _swaps1.push_back(_generate_value()%_num_of_acc_and_vals);
      }

      loggerman->print_progress(1,1);
      std::cout << "done.\n" << std::flush;
   }

   inline size_t _generate_value() {
      return _uid(_dre);
   }
};

/**
 * Implementationn of the test.
 *
 * The test involves a series of three steps:
 * 1) Generate the specified amount of random data to be used as data
 * and as what shall be done to the data (note that the random number
 * generator may be seeded for a deterministic re-run of the test).
 * 2) Fill the given database up with the generated data to simulate
 * an environment that already has working data in place to be used.
 * 3) Perform the specified amount of transfers/swaps on the given
 * data. The transfer/swap operation was chosen because of how
 * fundamental the operation is.
 */
class database_test {
public:
   enum class window : size_t {
      expanding_window = 0,
      narrow_window    = 1,
      rolling_window   = 2
   };
   
   database_test(unsigned int seed,
                 size_t lower_bound_inclusive,
                 size_t upper_bound_inclusive,
                 size_t num_of_acc_and_vals,
                 size_t num_of_swaps,
                 size_t max_key_length,
                 size_t max_key_value,
                 size_t max_value_length,
                 size_t max_value_value,
                 window window)
      : _window{window}
      , _gen_data{seed,
              lower_bound_inclusive,
              upper_bound_inclusive,
              num_of_acc_and_vals,
              num_of_swaps,
              max_key_length,
              max_key_value,
              max_value_length,
              max_value_value}
   {
      _database.add_index<account_index>();
   }

   ~database_test()
   {
      boost::filesystem::remove_all(_database_dir);
   }

   inline void start_test() {
      _initial_database_state();
      _execution_loop();
      loggerman->flush_all();
   }

private:
   const boost::filesystem::path _database_dir{boost::filesystem::current_path() /= std::string{"/chainbase"}};
   chainbase::database _database{_database_dir, chainbase::database::read_write, (4096ULL * 100000000ULL)};
   window _window;
   generated_data _gen_data;
   system_metrics _system_metrics;

   inline void _initial_database_state() {
      clockerman->reset_clocker();

      std::cout << "Filling initial database state...\n" << std::flush;
      loggerman->print_progress(1,0);
      
      for (size_t i{}; i < _gen_data.num_of_acc_and_vals()/10; ++i) {
         if (clockerman->should_log()) {
            loggerman->log_tps      ({clockerman->seconds_since_start_of_test(), 0});
            loggerman->log_ram_usage({clockerman->seconds_since_start_of_test(), _system_metrics.total_ram_currently_used()});
            loggerman->log_cpu_load ({clockerman->seconds_since_start_of_test(), _system_metrics.calculate_cpu_load()});
            loggerman->print_progress(i, _gen_data.num_of_acc_and_vals()/10);
            clockerman->update_clocker();
         }

         _database.start_undo_session(true);

         auto session{_database.start_undo_session(true)};
         _database.start_undo_session(true);
         // Create 10 new accounts per undo session.
         // AKA; create 10 new accounts per block.
         for (size_t j{}; j < 10; ++j) {
            _database.create<account>([&](account& acc) {
               acc._account_key   = _gen_data.accounts()[i*10+j];
               acc._account_value = _gen_data.values  ()[i*10+j];
            });
         }
         session.push();
      }

      loggerman->print_progress(1,1);
      _database.start_undo_session(true).push();
      std::cout << "done.\n" << std::flush;
   }

   inline void _execution_loop() {
      size_t transactions_per_second{};

      std::cout << "Benchmarking...\n" << std::flush;
      loggerman->print_progress(1,0);
      
      for (size_t i{}; i < _gen_data.num_of_swaps(); ++i) {
         if (clockerman->should_log()) {
            switch (_window) {
                case window::expanding_window:
                   loggerman->log_tps({clockerman->seconds_since_start_of_test(), transactions_per_second/clockerman->expanding_window()});
                   clockerman->update_clocker();
                   break;
                case window::narrow_window:
                   loggerman->log_tps({clockerman->seconds_since_start_of_test(), transactions_per_second/clockerman->narrow_window()});
                   clockerman->update_clocker();
                   transactions_per_second = 0;
                   break;
                // TODO
                // case window::rolling_window:
                //    loggerman->log_tps({clockerman->seconds_since_start_of_test(), transactions_per_second/clockerman->seconds_since_start_of_test()});
                //    break;
                default:
                   throw std::runtime_error{"database_test::should_log()"};
                   break;
            }
            loggerman->log_ram_usage({clockerman->seconds_since_start_of_test(), _system_metrics.total_ram_currently_used()});
            loggerman->log_cpu_load ({clockerman->seconds_since_start_of_test(), _system_metrics.calculate_cpu_load()});
            loggerman->print_progress(i, _gen_data.num_of_swaps());
         }

         const auto& rand_account0{_database.get(account::id_type(_gen_data.swaps0()[i]))};
         const auto& rand_account1{_database.get(account::id_type(_gen_data.swaps1()[i]))};

         auto session{_database.start_undo_session(true)};
         arbitrary_datum tmp{rand_account0._account_value};

         _database.modify(rand_account0, [&](account& acc) {
            acc._account_value = rand_account1._account_value;
         });
         _database.modify(rand_account1, [&](account& acc) {
            acc._account_value = tmp;
         });

         session.squash();
         transactions_per_second += 2;
      }

      loggerman->print_progress(1,1);
      std::cout << "done.\n" << std::flush;
   }

   inline size_t _expanding_window_metric() {
      return transactions_per_second/clockerman->expanding_window();
   }

   inline size_t _narrow_window_metric() {
      return transactions_per_second/clockerman->narrow_window();
   }

   // TODO
   // inline size_t _rolling_window_metric() {
   //    return clockerman->();
   // }
};

void print_help() {
   std::cout << "Base Layer Transactional Database Benchmarking Tool\n";
   std::cout << "Example Usage:\n";
   std::cout << "./bench-tps -s|--seed 42 \\\n"
             << "            -n|--number-of-accounts-and-values 1000000 \\\n"
             << "            -w|--number-of-swaps 1000000 \\\n"
             << "            -k|--maximum-key-size 1023 \\\n"
             << "            -y|--maximum-key-individual-byte-value 255 \\\n"
             << "            -v|--maximum-value-size 1023 \\\n"
             << "            -e|--maximum-value-individual-byte-value 255\n";
}

// Usage:
// ./bench-tps 42 1000000 1000000 1023 255 1023 255
//
// Future Usage:
// ./bench-tps -s|--seed=42 \
//             -n|--number-of-accounts-and-values=1000000 \
//             -w|--number-of-swaps=1000000 \
//             -k|--maximum-key-size=1023 \
//             -y|--maximum-key-individual-byte-value=255 \
//             -v|--maximum-value-size=1023 \
//             -e|--maximum-value-individual-byte-value=255
int main(int argc, char** argv) {
   loggerman  = std::make_unique<logger>();
   clockerman = std::make_unique<clocker>(1);

   try {
      if (argc == 2 && (argv[1] == std::string{"-h"} || argv[1] == std::string{"--help"})) {
         print_help();
         return 0;
      }

      if (argc != 8) {
         std::cout << "Please enter the correct amount of arguments.\n\n";
         print_help();
         return 0;
      }

      static const size_t lower_bound_inclusive{0};
      static const size_t upper_bound_inclusive{std::numeric_limits<size_t>::max()};

      static const unsigned int seed               {static_cast<unsigned int>(std::stoi(argv[1],nullptr,10))};
      static const size_t       num_of_acc_and_vals{static_cast<size_t>      (std::stoi(argv[2],nullptr,10))};
      static const size_t       num_of_swaps       {static_cast<size_t>      (std::stoi(argv[3],nullptr,10))};
      static const size_t       max_key_length     {static_cast<size_t>      (std::stoi(argv[4],nullptr,10))};
      static const size_t       max_key_value      {static_cast<size_t>      (std::stoi(argv[5],nullptr,10))};
      static const size_t       max_value_length   {static_cast<size_t>      (std::stoi(argv[6],nullptr,10))};
      static const size_t       max_value_value    {static_cast<size_t>      (std::stoi(argv[7],nullptr,10))};

      database_test dt{seed,
                       lower_bound_inclusive,
                       upper_bound_inclusive,
                       num_of_acc_and_vals,
                       num_of_swaps,
                       max_key_length,
                       max_key_value,
                       max_value_length,
                       max_value_value,
                       database_test::window::narrow_window};
      dt.start_test();
   }
   catch(const std::runtime_error& e) {
      std::cout << "`std::runtime_error'\n";
      std::cout << e.what() << '\n';
   }
   catch(const std::exception& e) {
      std::cout << "`std::exception'\n";
      std::cout << e.what() << '\n';
   }
   catch(...) {
      std::cout << "Other\n";
   }

   return 0;
}
