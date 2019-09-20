/**
 *  @file common.hpp
 *  @copyright defined in eosio/LICENSE.txt
 */

#pragma once

#include "clocker.hpp"
#include "logger.hpp"

#define LIKELY(x)   __builtin_expect((size_t)!!(x), 1ULL)
#define UNLIKELY(x) __builtin_expect((size_t)!!(x), 0ULL)

/**
 * Alias for which to describe a vector of random bytes. Since this
 * test doesn't care what the data is, we use an arbitrary amount of
 * bytes with arbitrary values to be used as fillers for benchmarking
 * the database.
 */
using arbitrary_datum = std::vector<uint8_t>;

/**
 * Enum documentation...
 */
enum class window : size_t {
   expanding_window = 0,
   narrow_window    = 1,
   rolling_window   = 2
};

/**
 * Unused variables; may need for future use when a more
 * comprehensible user-friendly API gets developed.
 */
__attribute__((unused)) static const size_t byte{1};
__attribute__((unused)) static const size_t kilobyte{byte*1024};
__attribute__((unused)) static const size_t megabyte{kilobyte*1024};
__attribute__((unused)) static const size_t gigabyte{megabyte*1024};

/**
 * Clocking facility to handle the logic of when to log specified test
 * metrics.
 */
extern std::unique_ptr<clocker> clockerman;


/**
 * Logging facility to handle all logging operations. Ranging from
 * printing output to the console to logging the metrics to their
 * respective files.
 */
extern std::unique_ptr<logger> loggerman;
