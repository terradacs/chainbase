/**
 *  @file chainrocks_interface.hpp
 *  @copyright defined in eosio/LICENSE.txt
 */

#pragma once

#include <boost/filesystem.hpp> // boost::filesystem::path

#include <chainrocks/chainrocks.hpp> // chainrocks::database

#include "abstract_database.hpp" // abstract_database
#include "common.hpp"            // arbitrary_datum
#include "generated_data.hpp"    // generated_data

/**
 * Implementation of `chainrocks' interface for the benchmark.
 *
 * `chainrocks' is an abstraction layer over the well-known database
 * `RocksDB'; which claims to operate efficiently while both in and
 * out of RAM.
 */
class chainrocks_interface : public abstract_database<chainrocks::database> {
public:
   /**
    * Constructor; takes the path for which to store the state of
    * `chainrocks' database.
    */
   chainrocks_interface(const boost::filesystem::path& database_dir);

   /**
    * Destructor; normal operation.
    */
   virtual ~chainrocks_interface() final;

   /**
    * Put both the `key' and the `value' in the database. In the case
    * of `chainrocks', it shall be put into an underlying `RocksDB'
    * memory table. `ctx' pointer has no use here; therefore it is not
    * used.
    */
   virtual void put(arbitrary_datum key, arbitrary_datum value, void* ctx = nullptr) final;

   /**
    * Modify the state of `chainbase' by perfoming a canonical
    * swap. It is essentially taking two indices in the underlying
    * memory table and performing a swap on the given accounts.
    */
   virtual void swap(const generated_data& gen_data, size_t i) final;

   /**
    * Performs a `RocksDB' `put_batch', which write all of the
    * contents of its memory tables to a `.sst' (Solid State).
    */
   virtual void write() final;

   /**
    * Returns the underlying `session' object that `chainrocks'
    * provides. TODO: Factor this out; the user should not be aware of
    * this abstraction.
    */
   auto start_undo_session(bool enabled) {
      return _db.start_undo_session(enabled);
   }
   
private:
   /**
    * The underlying database of `chainrocks'. Note that the state will
    * get destroyed upon a successful exit of the program. Therefore,
    * it cannot be inspected.
    */
   chainrocks::database _db;
};
