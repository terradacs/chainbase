/**
 *  @file generated_data.hpp
 *  @copyright defined in eosio/LICENSE.txt
 */

#pragma once

#include <random> // std::default_random_engine|uniform_int_distribution
#include <vector> // std::vector

#include "common.hpp" // arbitrary_datum

/**
 * Implementation of benchmark's random data generation facility.
 *
 * Random numbers are generated with a uniform distribution by using
 * the standard's random number generation facilities (this is subject
 * to be cusomizable in the future). It's important to note here, that
 * the user is able to choose how large or small he/she wishes the
 * keys/values to be. But not only can the size be specified, but the
 * filler value can also be specified. The filler value is important
 * to note and must have an appropriate degree of randomness to it. If
 * this weren't so, some database implementations may take advantage
 * of the lack of entropy.
 */
class generated_data {
public:
   /**
    * Constructor; normal operation.
    */
   generated_data();

   /**
    * Constructor; generates the randomly genereted data needed for
    * the benchmark.
    */
   generated_data(unsigned int seed,
                  size_t lower_bound_inclusive,
                  size_t upper_bound_inclusive,
                  size_t num_of_accounts,
                  size_t num_of_swaps,
                  size_t max_key_length,
                  size_t max_key_value,
                  size_t max_value_length,
                  size_t max_value_value);

   /**
    * Used to generate the randomly generated data for the benchmark
    * if it has not yet done so via the constructor.
    */
   void init(unsigned int seed,
             size_t lower_bound_inclusive,
             size_t upper_bound_inclusive,
             size_t num_of_accounts,
             size_t num_of_swaps,
             size_t max_key_length,
             size_t max_key_value,
             size_t max_value_length,
             size_t max_value_value);

   /**
    * Return the number of accounts in the accounts vector.
    */
   const size_t num_of_accounts() const;
   
   /**
    * Return the number of swaps in the swaps vector.
    */
   const size_t num_of_swaps() const;

   /**
    * Return a reference to the vector holding the randomly generated
    * accounts (`arbitrary_datum').
    */
   const std::vector<arbitrary_datum>& accounts() const;

   /**
    * Return a reference to the vector holding the randomly generated
    * values (`arbitrary_datum').
    */
   const std::vector<arbitrary_datum>& values() const;

   /**
    * Return a reference to the vector holding the randomly generated
    * values (`arbitrary_datum').
    */
   const std::vector<size_t>& swaps0() const;

   /**
    * Return a reference to the vector holding the randomly generated
    * values (`arbitrary_datum').
    */
   const std::vector<size_t>& swaps1() const;

private:
   /**
    * Holds the random number to generate the arbitrary data for the
    * benchmark.
    */
   std::default_random_engine _dre;

   /**
    * Holds the distribution used for generating the random data.
    * TODO: Make this a customizable user option in the future. 
    */
   std::uniform_int_distribution<size_t> _uid;

   /**
    * Holds the number of accounts the benchmark will work with.
    */
   size_t _num_of_accounts;

   /**
    * Holds the number of swap operations the benchmark will work
    * with.
    */
   size_t _num_of_swaps;

   /**
    * Holds the maximum key length (in bytes) that the benchmark will
    * work with.
    */
   size_t _max_key_length;

   /**
    * Holds the maximum key value that the benchmark will work with in
    * the range [0-`_max_key_value').
    */
   size_t _max_key_value;

   /**
    * Holds the maximum value length (in bytes) that the benchmark
    * will work with.
    */
   size_t _max_value_length;

   /**
    * Holds the maximum value value that the benchmark will work with
    * in the range [0-`_max_value_value').
    */
   size_t _max_value_value;

   /**
    * Holds the `arbitrary_data' relating to accounts.
    */
   std::vector<arbitrary_datum> _accounts;

   /**
    * Holds the `arbitrary_data' relating to values.
    */
   std::vector<arbitrary_datum> _values;

   /**
    * Holds the indices deciding which first account to swap.
    * TODO: Combine the two swap vectors.
    */
   std::vector<size_t> _swaps0;

   /**
    * Holds the indices deciding which second account to swap.
    * TODO: Combine the two swap vectors.
    */
   std::vector<size_t> _swaps1;

   /**
    * Helper function for generating the random data.
    */
   void _generate_values();

   /**
    * Helper function for generating the random data, individually.
    */
   size_t _generate_value();
};
