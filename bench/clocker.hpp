/**
 *  @file clocker.hpp
 *  @copyright defined in eosio/LICENSE.txt
 */

#pragma once

#include <deque>  // std::deque
#include <memory> // std::unique_ptr

/**
 * Implementation of the benchmark's clocking facility.
 */
class clocker {
private:
   /**
    * Forward declaration for underlying data structure to keep track
    * of and calculate moving averages. TODO: This should be factored
    * out. The idea behind a clocking facility and rolling averages
    * are loosely related.
    */
   struct rolling_average;
   
public:
   /**
    * Constructor; takes the interval in seconds in which the
    * benchmark shall be clocked. TODO: Let the user specify
    * milliseconds and/or possibly microseconds.
    */
   clocker(size_t interval_in_seconds);

   /**
    * Reset the state of this data structure to that akin of it's
    * original state. Used for logging the progress of the benchmark
    * multiple times.
    */
   void reset_clocker();

   /**
    * Check if the `_interval_in_seconds' variant has been met. If so,
    * the logging logic should be performed.
    */
   bool should_log();

   /**
    * Update this data structure so that the next measure can be taken
    * in the future. This method is here because the other methods
    * that gather the measurements are not allowed to update the time
    * facility.
    */
   void update_clocker();

   /**
    * Returns how many seconds have passed snce the last was last
    * instantiated or reset.
    */
   size_t seconds_since_start_of_benchmark();

   /**
    * Return the TPS calculation of an ever-expanding window. This
    * means that the average of all ticks are taken into account when
    * measuring the TPS.
    */
   size_t expanding_window();

   /**
    * Return the TPS calculation of a narrow window. This means that
    * the TPS will only be measured by a specified window (1 second
    * window, 5 second window, etc.).
    */
   size_t narrow_window();

   /**
    *  Return the calculation of a rolling window. This means that the
    *  TPS calculation will be that of a rolling average/moving
    *  average.
    */
   size_t rolling_window(size_t term);
   
private:
   /**
    * Holds the necessary state required performing rolling average
    * calculations.
    */
   std::unique_ptr<rolling_average> _rolling_average;

   /**
    * Holds the user-specified interval to measure the benchmark.
    */
   size_t _interval_in_seconds;

   /**
    * Holds the time upon the instantiation of this class or if it has
    * been reset.
    */
   size_t _original_time;

   /**
    * Holds the time needed for doing various calculations.
    */
   size_t _old_time;

   /**
    * Holds the time needed for doing various calculations.
    */
   size_t _new_time;

   /**
    * Holds the logic of measuring TPS over a rolling window.
    */
   struct rolling_average {
      /**
       * Specified number of terms in which to perform the rolling
       * average calculation. TODO: Will be `_last_n_terms'.
       */
      std::deque<size_t> _last_five_terms{0,0,0,0,0};

      /**
       * Push the given term to the vector.
       */
      void push_term(size_t term);

      /**
       * Return the rolling average calculation.
       */
      size_t get_rolling_average();
   };

   /**
    * Implementation of receiving the time measurements from the
    * underlying `std::chrono' library.
    */
   long long _retrieve_time();
};
