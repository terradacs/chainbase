/**
 *  @file bench_tps.cpp
 *  @copyright defined in eosio/LICENSE.txt
 */

#include <exception> // std::exception|runtime_error
#include <iostream>  // std::cout|flush
#include <memory>    // std::unique_ptr

#include <boost/program_options.hpp> // boost::program_options::options_description|variables_map

#include "chainbase_interface.hpp"       // chainbase_interface
#include "chainrocks_interface.hpp"      // chainrocks_interface
#include "clocker.hpp"                   // clocker
#include "database_benchmark_driver.hpp" // database_benchmark
#include "logger.hpp"                    // logger

std::unique_ptr<clocker> clockerman = std::make_unique<clocker>(1);
std::unique_ptr<logger>  loggerman  = std::make_unique<logger>();

/**
 * Implementation of the benchmark; bringing all the pieces together.
 *
 * The important part is this line:
 * `database_benchmark<chainrocks_interface> dt{window::expanding_window};'
 * Here the user is allowed to specify the underlying database backend
 * he/she has implemented. In this case we use the `chainrocks'
 * interface, which connects to the `chainrocks' key-value store
 * backend. After which, for the benchmark's argument, the user shall
 * specify how the benchmark should measured. These options include:
 *
 * 1) `window::expanding_window': Think average over all time spent.
 * 2) `window::narrow_window': Think intervals in between one second.
 * 3) `window::rolling_window': Think moving average.
 *
 * The output of which shall be a `data.csv' file, which can then be
 * fed into the `gnuplot' script provided in the `build/bench/'
 * directory; and there lie the results of the benchmark.
 */
int main(int argc, char** argv) {
   try {
      boost::program_options::options_description cli{
         "bench-tps; A Base Layer Transactional Database Benchmarking Tool\n" \
         "Usage:\n" \
         "bench-tps -s|--seed 42 \\\n" \
         "          -l|--lower-bound 0 \\\n" \
         "          -u|--upper-bound 18446744073709551615 \\\n" \
         "          -n|--num-of-accounts 100000 \\\n" \
         "          -w|--num-of-swaps 100000 \\\n" \
         "          -k|--max-key-length 1023 \\\n" \
         "          -y|--max-key-value 255 \\\n" \
         "          -v|--max-value-length 1023 \\\n" \
         "          -e|--max-value-value 255\n"};

      
      database_benchmark<chainrocks_interface> dt{window::expanding_window};
      dt.set_program_options(cli);
         
      boost::program_options::variables_map vmap;
      boost::program_options::store(boost::program_options::parse_command_line(argc, argv, cli), vmap);
      boost::program_options::notify(vmap);
      
      if (vmap.count("help") > 0) {
         cli.print(std::cerr);
         return 0;
      }

      dt.execute_benchmark();
   }
   catch(const std::runtime_error& e) {
      std::cout << "`std::runtime_error'\n" << std::flush;
      std::cout << e.what() << '\n' << std::flush;
   }
   catch(const std::exception& e) {
      std::cout << "`std::exception'\n" << std::flush;
      std::cout << e.what() << '\n' << std::flush;
   }
   catch(...) {
      std::cout << "Other\n" << std::flush;
   }

   return 0;
}
