/**
 *  @file logger.hpp
 *  @copyright defined in eosio/LICENSE.txt
 */

#pragma once

#include <iomanip>  // std::setw
#include <fstream>  // std::ofstream
#include <iostream> // std::cout|flush
#include <set>      // std::set
#include <utility>  // std::pair
#include <vector>   // std::vector

#include <boost/filesystem.hpp> // boost::filesystem::unique_path|remove_all

/**
 * Implementation of this test's logging facility.
 *
 * All logging, but the logging of the test's current progression
 * status, is deferred until the end of the test. The data is flushed
 * to its respective file as a comma-separated list. The supported
 * metrics are as follows: CPU Usage, RAM Usage,
 * Transactions-Per-Second (with differing variations of measuring it
 * over time).
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
