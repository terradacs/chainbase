/**
 *  @file abstract_database.hpp
 *  @copyright defined in eosio/LICENSE.txt
 */

#pragma once

#include "common.hpp"         // arbitrary_datum
#include "generated_data.hpp" // generated_data

/**
 * Implementation of the database abstraction.
 *
 * This test will be database agnostic. Where the interface shall
 * shall only need to provide the put operation for the purpose of
 * this test. In the future more operations may be added.
 */
template<typename Database>
class abstract_database {
public:
   virtual ~abstract_database() = default;

   /**
    * Implementation of a database's put operation.
    *
    *
    */
   virtual inline void put(arbitrary_datum key, arbitrary_datum value, void* ctx = nullptr)=0;

   /**
    * Implementation of a database's swap operation.
    *
    *
    */
   virtual inline void swap(const generated_data& gen_data, size_t i)=0;

   /**
    * Implementation of a database's write operation.
    *
    *
    */
   virtual inline void write()=0;
};
