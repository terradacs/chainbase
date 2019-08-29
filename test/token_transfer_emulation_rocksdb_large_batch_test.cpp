#include <ctime> // time

#include <fstream>  // std::ofstream
#include <iostream> // std::cout
#include <limits>   // std::numeric_limits
#include <random>   // std::default_random_engine, std::uniform_int_distribution
#include <set>      // std::set

#include <mach/mach.h>
#include <sys/mount.h>
#include <sys/sysctl.h>

#include <chainbase/chainrocks_rocksdb.hpp> // chainrocks::database

class logger;
class system_metrics;
class generated_data;
class timer;
class database_test;

static const size_t byte{1};
static const size_t kilobyte{byte*1024};
static const size_t megabyte{kilobyte*1024};
static const size_t gigabyte{megabyte*1024};

static time_t initial_time{time(NULL)};
static size_t prev_total_ticks{};
static size_t prev_idle_ticks{};

using byte_array = std::vector<uint8_t>;

class logger {
public:
   logger(const std::string& data_file = "/Users/john.debord/chai/measurements/data.csv")
      : _data_file{data_file}
   {
      _tps.reserve(1000);
      _ram_usage.reserve(1000);
      _cpu_load.reserve(1000);
   }

   void flush_all() {
      for (size_t i{}; i < _tps.size(); ++i) {
         _data_file << _tps[i].first << '\t' << _tps[i].second;
         _data_file                  << '\t' << _cpu_load[i].second;
         _data_file                  << '\t' << _ram_usage[i].second << '\n';
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

struct arbitrary_datum {
   arbitrary_datum(size_t n)
      : _blob_size{n}
      , _struct_size{_blob_size+16}
      , _bytes{new uint8_t[_blob_size]{}}
   {
   }

   arbitrary_datum(const arbitrary_datum& ad) {
      _blob_size   = ad._blob_size;
      _struct_size = ad._struct_size;
      _bytes       = new uint8_t[ad._blob_size];
   }

   ~arbitrary_datum() {
      delete[] _bytes;
   }

   operator std::string() const {
      char* tmp{new char[_struct_size+1]};
      memcpy(tmp, (void*)this, 16);
      memcpy(tmp+16, (void*)_bytes, _blob_size);
      tmp[_struct_size] = '\0';
      std::string ret{tmp};
      delete[] tmp;
      return ret;
   }
   
   size_t size() const {
      return _struct_size;
   }
   
   size_t   _blob_size;
   size_t   _struct_size;
   uint8_t* _bytes;
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

   // inline void print_accounts() { _print_something("_accounts", _accounts); }
   // inline void print_values()   { _print_something("_values",   _values);   }
   // inline void print_swaps0()   { _print_something("_swaps0",   _swaps0);   }
   // inline void print_swaps1()   { _print_something("_swaps1",   _swaps1);   }

   inline const size_t num_of_accounts_and_values() const { return _num_of_accounts_and_values; }
   inline const size_t num_of_swaps()               const { return _num_of_swaps;               }

   inline const std::vector<chainrocks::rocksdb_datum<uint64_t, uint64_t>>& accounts() const { return _accounts; }
   inline const std::vector<arbitrary_datum>&                               values()   const { return _values;   }
   inline const std::vector<chainrocks::rocksdb_datum<uint64_t, uint64_t>>& swaps0()   const { return _swaps0;   }
   inline const std::vector<chainrocks::rocksdb_datum<uint64_t, uint64_t>>& swaps1()   const { return _swaps1;   }

private:
   std::default_random_engine _dre;
   std::uniform_int_distribution<size_t> _uid;

   size_t _num_of_accounts_and_values;
   size_t _num_of_swaps;

   std::vector<chainrocks::rocksdb_datum<uint64_t, uint64_t>> _accounts;
   std::vector<arbitrary_datum> _values;
   std::vector<chainrocks::rocksdb_datum<uint64_t, uint64_t>> _swaps0;
   std::vector<chainrocks::rocksdb_datum<uint64_t, uint64_t>> _swaps1;

   inline void _generate_values() {
      std::cout << "Generating values... " << std::flush;

      std::cout << '\n';
      for (size_t i{}; i < _num_of_accounts_and_values; ++i) {
         _accounts.push_back(_uid(_dre));
         _values.push_back(arbitrary_datum{(_uid(_dre) % 300000)});
      }

      for (size_t i{}; i < _num_of_swaps; ++i) {
         _swaps0.push_back(_generate_value()%_num_of_accounts_and_values);
         _swaps1.push_back(_generate_value()%_num_of_accounts_and_values);
      }

      std::cout << "done.\n" << std::flush;
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

   inline void _print_something(const std::string& vec_name, const std::vector<arbitrary_datum>& vec) {
      std::cout << vec_name << ": \n";
      size_t count{};
      for (auto e : vec) {
         std::cout << e.size() << '\t';
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
      : _gen_data{num_of_accounts_and_values, num_of_swaps, lower_bound_inclusive, upper_bound_inclusive}
   {
   }

   ~database_test()
   {
   }

   inline void print_everything() {
      // _gen_data.print_accounts();
      // _gen_data.print_values();
      // _gen_data.print_swaps0();
      // _gen_data.print_swaps1();
   }

   inline void start_test() {
      _initial_database_state();
      _execution_loop();
      _log.flush_all();
   }

private:
   // chainrocks::database<rocksdb::Slice, std::optional<std::vector<uint8_t>>> _database;
   chainrocks::database<uint64_t, uint64_t> _database;
   generated_data _gen_data;
   logger _log;
   system_metrics _system_metrics;

   inline void _initial_database_state() {
      size_t transactions_per_second{};

      time_t old_time{initial_time};
      time_t new_time{initial_time};
      
      std::cout << "Filling initial database state... " << std::flush;
      for (size_t i{}; i < _gen_data.num_of_accounts_and_values()/10; ++i) {
         new_time = time(NULL);
         if (new_time != old_time) {
            _log.log_tps      ({(new_time - initial_time), transactions_per_second/(new_time - initial_time)});
            _log.log_ram_usage({(new_time - initial_time), _system_metrics.total_ram_currently_used()});
            _log.log_cpu_load ({(new_time - initial_time), _system_metrics.calculate_cpu_load()});
            old_time = new_time;
         }
         
         auto session{_database.start_undo_session(true)};
         // Create 10 new accounts per undo session.
         // AKA; create 10 new accounts per block.
         for (size_t j{}; j < 10; ++j) {
            _database.put_batch(_gen_data.accounts()[i*10+j], _gen_data.values()[i*10+j]);
         }
         session.push();
      }
      _database.start_undo_session(true).push();
      _database.write_batch();
      std::cout << "done.\n" << std::flush;
   }

   inline void _execution_loop() {
      size_t transactions_per_second{};
      std::string value0;
      std::string value1;

      time_t old_time{initial_time};
      time_t new_time{initial_time};

      std::cout << "Benchmarking... " << std::flush;
      for (size_t i{}; i < _gen_data.num_of_swaps(); ++i) {
         new_time = time(NULL);
         if (new_time != old_time) {
            _log.log_tps      ({(new_time - initial_time), transactions_per_second/(new_time - initial_time)});
            _log.log_ram_usage({(new_time - initial_time), _system_metrics.total_ram_currently_used()});
            _log.log_cpu_load ({(new_time - initial_time), _system_metrics.calculate_cpu_load()});
            old_time = new_time;
         }

         auto rand_account0{_gen_data.accounts()[_gen_data.swaps0()[i]]};
         auto rand_account1{_gen_data.accounts()[_gen_data.swaps1()[i]]};

         auto session{_database.start_undo_session(true)};
         _database._state.get(rand_account0, value0);
         _database._state.get(rand_account1, value1);
         _database.put_batch(rand_account0, value1);
         _database.put_batch(rand_account1, value0);
         session.squash();
         transactions_per_second += 2;
      }
      _database.write_batch();
      std::cout << "done.\n" << std::flush;
   }
};

int main(int argc, char** argv) {
   static const size_t num_of_accounts_and_values{100};
   static const size_t num_of_swaps{100};
   static const size_t lower_bound_inclusive{0};
   static const size_t upper_bound_inclusive{std::numeric_limits<size_t>::max()};

   database_test dt{num_of_accounts_and_values, num_of_swaps, lower_bound_inclusive, upper_bound_inclusive};
   dt.start_test();
   return 0;
}
