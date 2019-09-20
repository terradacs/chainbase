/**
 *  @file system_metrics.hpp
 *  @copyright defined in eosio/LICENSE.txt
 */

#pragma once

/**
 * Implementation of benchmark's system-metric measuring facilities.
 *
 * As of late, only specific metrics for __APPLE__ computers are used.
 *  The current set of the most useful metrics are as follows:
 *  `total_vm_currently_used' (determines how much virtual memory is
 *  currently in use by the system), `total_vm_used_by_proc'
 *  (determines how much virtual memory is currently in use by the
 *  current process), `total_ram_currently_used' (determines how much
 *  RAM is currently in use by the system), `get_cpu_load (determines
 *  the current load the CPU is experiencing)'.
 */
class system_metrics {
public:
   /**
    * Destructor; normal operation.
    */
   system_metrics();

   /**
    * Print the total virtual memory on the machine.
    */
   void total_vm();

   /**
    * Print the total virtual memory currently used on the machine.
    */
   void total_vm_currently_used();

   /**
    * Print the total virtual memory currently used by a process.
    */
   void total_vm_used_by_proc();

   /**
    * Return the total RAM on a machine.
    */
   size_t total_ram();

   /**
    * Return the RAM currently used by the machine as a coefficient
    * <= 1.
    */
   double total_ram_currently_used();

   /**
    * Helper function used to help calculate the total CPU load.
    * TODO: Remove this from the interface.
    */
   double calculate_cpu_load();

   /**
    * Helper function used to help calculate the total CPU load.
    * TODO: Remove this from the interface.
    */
   double get_cpu_load();

   /**
    * Return the CPU load currently used by the machine as a
    * coefficient < 1.
    */
   double calculate_cpu_load(size_t idle_ticks, size_t total_ticks);

private:
   /**
    * Holds the previous CPU ticks by the machine.
    */
   size_t _prev_total_ticks;

   /**
    * Holds the previous idle CPU ticks by the machine.
    */
   size_t _prev_idle_ticks;
};
