/**
 *  @file logger.hpp
 *  @copyright defined in eosio/LICENSE.txt
 */

#pragma once

#include <fstream> // std::ofstream
#include <utility> // std::pair
#include <vector>  // std::vector

#include <boost/filesystem.hpp> // boost::filesystem::unique_path|remove_all

/**
 * Implementation of the benchmark's logging facility.
 *
 * All logging, but the logging of the benchmark's current progression
 * status, is deferred until the end of the benchmark. The data is flushed
 * to its respective file as a comma-separated list. The supported
 * metrics are as follows: CPU Usage, RAM Usage,
 * Transactions-Per-Second (with differing variations of measuring it
 * over time).
 */
class logger {
public:
   /**
    * Constructor; normal operation.
    */
   logger();

   /**
    * Log the progress of a construct to the console. With `n' being
    * the numerator and `m' being the denominator multiplied by a
    * factor of 100.
    */
   void print_progress(size_t n, size_t m);

   /**
    * Flush all saved metrics to the specified file.
    */
   void flush_all();

   /**
    * Save a TPS measurement to the TPS vector.
    */
   void log_tps(const std::pair<size_t,size_t>& p);

   /**
    * Save a RAM usage measurement to the RAM usage vector.
    */
   void log_ram_usage(const std::pair<size_t,double>& p);

   /**
    * Save a CPU load measurement to the CPU load vector.
    */
   void log_cpu_load(const std::pair<size_t,double>& p);

private:
   /**
    * Holds the TPS measurements.
    */
   std::vector<std::pair<size_t,size_t>> _tps;

   /**
    * Holds the RAM usage measurements.
    */
   std::vector<std::pair<size_t,double>> _ram_usage;

   /**
    * Holds the CPU load measurements.
    */
   std::vector<std::pair<size_t,double>> _cpu_load;

   /**
    * File in which to log all metrics.
    */
   std::ofstream _data_file;
};
