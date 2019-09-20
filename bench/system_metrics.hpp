/**
 *  @file system_metrics.hpp
 *  @copyright defined in eosio/LICENSE.txt
 */

#pragma once

#include <iostream> // std::cout

#ifdef __APPLE__
#include <mach/mach.h>
#include <sys/mount.h>
#include <sys/sysctl.h>
#endif // __APPLE__

#ifdef __linux__
// ...
#endif // __linux__

/**
 * Implementation of this test's system-metric measuring facilities.
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
         for (size_t i{}; i < CPU_STATE_MAX; i++) {
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
