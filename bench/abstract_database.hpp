/**
 *  @file abstract_database.hpp
 *  @copyright defined in eosio/LICENSE.txt
 */

#pragma once

#include "common.hpp"         // arbitrary_datum
#include "generated_data.hpp" // generated_data

/**
 * Implementation of database abstraction.
 *
 * This benchmark shall be database agnostic. Where a user-defined
 * interface need only to provide the operations specified here. In
 * the future these operations may be expanded upon.
 */
template<typename Database>
class abstract_database {
public:
   virtual ~abstract_database() = default;

   /**
    * Implementation of `put' operation.
    *
    * Needed so that the benchmark may fill the database with an
    * arbitrarily amount of data. Note that the the underlying
    * database backend may not provide a `put' operation; but it
    * should at least provide a `modify' operation; use that here. The
    * `ctx' pointer may be left null, as it is only here
    * premeditatively for future use.
    */
   virtual inline void put(arbitrary_datum key, arbitrary_datum value, void* ctx = nullptr)=0;

   /**
    * Implementation of `swap' operation.
    *
    * Needed so that the benchmark may swap the arbitrary data in a random
    * fashion. For now, look at the example usage defined in the example
    * backend database interfaces provided here in this
    * repository. TODO: The parameters for this function are not
    * intuitive and should be changed. `generated_data' should provide
    * a function to extract the `arbitrary_data' value from the i'th
    * index; this should not be left to the user to implement.
    */
   virtual inline void swap(const generated_data& gen_data, size_t i)=0;

   /**
    * Implementation of `write' operation.
    *
    * Needed in case the underlying database backend utilizes a
    * batch-writing system when calling `put' operations. Otherwise
    * this can just be a no-op if the underlying database backend
    * writes directly to disk upon a `put' operation.
    */
   virtual inline void write()=0;
};
