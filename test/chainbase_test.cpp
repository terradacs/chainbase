#include <ctime> // time

#include <chrono>   // std::chrono::time_point, std::chrono::system_clock::now
#include <fstream>  // std::ofstream
#include <iomanip>  // std::setw
#include <iostream> // std::cout, std::flush
#include <limits>   // std::numeric_limits
#include <random>   // std::default_random_engine, std::uniform_int_distribution
#include <set>      // std::set

#include <mach/mach.h>
#include <sys/mount.h>
#include <sys/sysctl.h>

#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/test/unit_test.hpp>
#include <chainbase/chainbase.hpp>

using namespace boost::multi_index;

class logger;
class clocker;
class system_metrics;
class generated_data;
class timer;
class database_test;

__attribute__((unused)) static const size_t byte{1};
__attribute__((unused)) static const size_t kilobyte{byte*1024};
__attribute__((unused)) static const size_t megabyte{kilobyte*1024};
__attribute__((unused)) static const size_t gigabyte{megabyte*1024};

static std::unique_ptr<logger> loggerman;
static std::unique_ptr<clocker> clockerman;
static time_t initial_time{time(NULL)};
static size_t prev_total_ticks{};
static size_t prev_idle_ticks{};

using arbitrary_datum = std::vector<uint8_t>;

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
#ifdef TO_CONSOLE
      for (size_t i{}; i < _tps.size(); ++i) {
         std::cout << std::setw(10) << _tps[i].first        << '\t';
         std::cout << std::setw(10) << _tps[i].second       << '\t';
         std::cout << std::setw(10) << _cpu_load[i].second  << '\t';
         std::cout << std::setw(10) << _ram_usage[i].second << '\n';
         std::cout << std::flush;
      }
#endif // MACRO
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

class clocker {
public:
   clocker(size_t interval_in_seconds)
      : _interval_in_seconds{interval_in_seconds*1000}
   {
   }

   void reset() {
      _original_time = _get_time();
      _old_time      = _original_time;
      _new_time      = _old_time;

// #ifdef TO_CONSOLE
      std::cout << "reset:: _old_time: "      << _old_time/_interval_in_seconds   << '\t';
      std::cout << "reset:: _new_time: "      << _new_time/_interval_in_seconds   << '\t';
      std::cout << "should_log:: _get_time: " << _get_time()/_interval_in_seconds << '\t';
      std::cout << "::::::::::::::::::::::: " << ((_new_time - _old_time)%_interval_in_seconds) << '\n';

// #endif // TO_CONSOLE
   }

   inline bool should_log() {
      _new_time = _get_time();

// #ifdef TO_CONSOLE
      std::cout << "should_log:: _old_time: " << _old_time/_interval_in_seconds   << '\t';
      std::cout << "should_log:: _new_time: " << _new_time/_interval_in_seconds   << '\t';
      std::cout << "should_log:: _get_time: " << _get_time()/_interval_in_seconds << '\n';
      std::cout << "::::::::::::::::::::::: " << ((_new_time - _old_time)%_interval_in_seconds) << '\n';
// #endif // TO_CONSOLE
            
      if (((_new_time - _old_time)%_interval_in_seconds) > _old_time) {
         _old_time = _new_time;
         _new_time = _get_time();
         return true;
      }
      else {
         _old_time = _new_time;
         _new_time = _get_time();
         return false;
      }
   }

   inline size_t get_time() {
      return ((_new_time - _old_time)%_interval_in_seconds);
   }

private:
   size_t _interval_in_seconds;
   size_t _original_time;
   size_t _old_time;
   size_t _new_time;

   inline size_t _get_time() {
      return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch()).count();
   }
};

class system_metrics {
public:
   system_metrics()
   {
   }

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
      size_t total_ticks_since_last_time{total_ticks - prev_total_ticks};
      size_t idle_ticks_since_last_time {idle_ticks  - prev_idle_ticks};

      double ret{1.0F - ((total_ticks_since_last_time > 0) ? (static_cast<double>(idle_ticks_since_last_time) / total_ticks_since_last_time) : 0)};

      prev_total_ticks = total_ticks;
      prev_idle_ticks  = idle_ticks;
      return ret;
   }
};

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
   inline const size_t num_of_swaps()               const { return _num_of_swaps; }

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
      clockerman->reset();

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

struct account : public chainbase::object<0,account> {
   template<typename Constructor, typename Allocator>
   account(Constructor&& c, Allocator&& a) { c(*this); }
   id_type id;
   arbitrary_datum _account_key;
   arbitrary_datum _account_value;
};

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

class database_test {
public:
   database_test(unsigned int seed,
                 size_t lower_bound_inclusive,
                 size_t upper_bound_inclusive,
                 size_t num_of_acc_and_vals,
                 size_t num_of_swaps,
                 size_t max_key_length,
                 size_t max_key_value,
                 size_t max_value_length,
                 size_t max_value_value)
      : _gen_data{seed,
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
   chainbase::database _database{_database_dir, chainbase::database::read_write, static_cast<uint64_t>((uint64_t)4096*(uint64_t)100000000)};
   generated_data _gen_data;
   system_metrics _system_metrics;

   inline void _initial_database_state() {
      size_t transactions_per_second{};
      clockerman->reset();

      std::cout << "Filling initial database state...\n" << std::flush;
      loggerman->print_progress(1,0);
      
      for (size_t i{}; i < _gen_data.num_of_acc_and_vals()/10; ++i) {
         if (clockerman->should_log()) {
            loggerman->log_tps      ({clockerman->get_time(),  transactions_per_second/clockerman->get_time()});
            loggerman->log_ram_usage({clockerman->get_time(),  _system_metrics.total_ram_currently_used()});
            loggerman->log_cpu_load ({clockerman->get_time(), _system_metrics.calculate_cpu_load()});
            loggerman->print_progress(i, _gen_data.num_of_acc_and_vals()/10);
         }

         _database.start_undo_session(true);

         auto session{_database.start_undo_session(true)};
         _database.start_undo_session(true);
         // Create 10 new accounts per undo session.
         // AKA; create 10 new accounts per block.
         for (size_t j{}; j < 10; ++j){
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

      time_t old_time{initial_time};
      time_t new_time{initial_time};

      std::cout << "Benchmarking...\n" << std::flush;
      loggerman->print_progress(1,0);
      for (size_t i{}; i < _gen_data.num_of_swaps(); ++i) {
         new_time = time(NULL);
         if (new_time != old_time) {
            loggerman->log_tps      ({(new_time - initial_time), transactions_per_second/(new_time - initial_time)});
            loggerman->log_ram_usage({(new_time - initial_time), _system_metrics.total_ram_currently_used()});
            loggerman->log_cpu_load ({(new_time - initial_time), _system_metrics.calculate_cpu_load()});
            loggerman->print_progress(i, _gen_data.num_of_swaps());
            old_time = new_time;
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
};

void print_help() {
   std::cout << "RocksDB/Chainbase Benchmarking\n";
   std::cout << "Usage:" << '\n';
   std::cout << "./token_transfer_emulation_rocksdb_large_batch_test \\\n"
             << "    <number-of-accounts/values> \\\n"
             << "    <number-of-swaps> \\\n"
             << "    <max-key-length> \\\n"
             << "    <max-key-size> \\\n"
             << "    <max-value-length> \\\n"
             << "    <max-value-size> \n";
}

// ./chainbase_test 42 1000000 1000000 1023 255 1023 255
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
                       max_value_value};
      dt.start_test();
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
