/**
 *  @file clocker.hpp
 *  @copyright defined in eosio/LICENSE.txt
 */

#pragma once

#include <chrono>  // std::chrono::high_resolution_clock|time_point
#include <deque>   // std::deque
#include <memory>  // std::unique_ptr
#include <numeric> // std::accumulate

/**
 * Implementation of this test's clocking facility.
 *
 * The clocking facility is responsible for all things related to time
 * and the differing ways of returning/interpreting the time.
 */
class clocker {
private:
   struct rolling_average;
   
public:
   clocker(size_t interval_in_seconds)
      : _rolling_average{std::make_unique<rolling_average>()}
      , _interval_in_seconds{interval_in_seconds*1000}
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

   // Currently only a 5 second rolling window is provided.
   inline size_t rolling_window(size_t term) {
      _rolling_average->push_term(term);
      return _rolling_average->get_rolling_average();
   }
   
private:
   std::unique_ptr<rolling_average> _rolling_average;
   size_t _interval_in_seconds;
   size_t _original_time;
   size_t _old_time;
   size_t _new_time;

   struct rolling_average {
      std::deque<size_t> _last_five_terms{0,0,0,0,0}; // Will be `_last_n_terms'.

      void push_term(size_t term) {
         _last_five_terms.push_front(term);
         _last_five_terms.pop_back();
      }

      size_t get_rolling_average() {
         return (std::accumulate(_last_five_terms.cbegin(),_last_five_terms.cend(),0) / 5);
      }
   };

   inline long long _retrieve_time() {
      std::chrono::time_point tp{std::chrono::high_resolution_clock::now()};
      std::chrono::milliseconds d{std::chrono::time_point_cast<std::chrono::milliseconds>(tp).time_since_epoch()};
      long long ms{std::chrono::duration_cast<std::chrono::milliseconds>(d).count()};
      return ms;
   }
};
