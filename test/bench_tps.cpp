/**
 *  @file bench_tps.cpp
 *  @copyright defined in eosio/LICENSE.txt
 */

#include <exception> // std::exception|runtime_error
#include <iostream>  // std::cout|flush
#include <memory>    // std::unique_ptr

#include <boost/program_options.hpp> // boost::program_options::options_description|variables_map

#include "chainbase_interface.hpp"  // chainbase_interface
#include "chainrocks_interface.hpp" // chainrocks_interface
#include "common.hpp"               // window
#include "clocker.hpp"              // clocker
#include "database_test.hpp"        // database_test
#include "generated_data.hpp"       // generated_data
#include "logger.hpp"               // logger
#include "system_metrics.hpp"       // clocker

auto clockerman = std::make_unique<clocker>(1);
auto loggerman  = std::make_unique<logger>();

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

      // database_test<chainbase_interface>  dt_chainbase {window::expanding_window};
      database_test<chainrocks_interface> dt_chainrocks{window::expanding_window};
      // dt_chainbase.set_program_options(cli);
      dt_chainrocks.set_program_options(cli);
      
      boost::program_options::variables_map vmap;
      boost::program_options::store(boost::program_options::parse_command_line(argc, argv, cli), vmap);
      boost::program_options::notify(vmap);
      
      if (vmap.count("help") > 0) {
         cli.print(std::cerr);
         return 0;
      }

      // dt_chainbase.execute_test();
      dt_chainrocks.execute_test();
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
