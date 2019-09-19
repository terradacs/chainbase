/**
 *  @file chainrocks_interface.hpp
 *  @copyright defined in eosio/LICENSE.txt
 */

#pragma once

#include <string> // std::string

#include <chainrocks/chainrocks.hpp> // chainrocks::database

#include "abstract_database.hpp" // abstract_database
#include "common.hpp"            // arbitrary_datum|clocker|logger
#include "generated_data.hpp"    // generated_data

/**
 * Implementation of the `chainrocks' database interface.
 *
 * Implements the two required API calls with the necessary logic
 * required in order for `chainbase' to perform such operations.
 */
class chainrocks_interface : public abstract_database<chainbase::database> {
public:
   chainrocks_interface(const boost::filesystem::path& database_dir)
      : _db{database_dir}
   {
   }
   
   virtual ~chainrocks_interface() final
   {   
   }
   
   virtual inline void put(arbitrary_datum key, arbitrary_datum value, void* ctx = nullptr) final {
      _db.put_batch(key, value);
   }

   virtual inline void swap(const generated_data& gen_data, size_t i) final {
      std::string string_value0;
      std::string string_value1;
      
      const arbitrary_datum rand_account0{gen_data.accounts()[gen_data.swaps0()[i]]};
      const arbitrary_datum rand_account1{gen_data.accounts()[gen_data.swaps1()[i]]};

      _db._state.get(rand_account0, string_value0);
      _db._state.get(rand_account1, string_value1);

      chainrocks::rocksdb_datum datum_value0{string_value0}; // Should not need this because of implicit conversion operator
      chainrocks::rocksdb_datum datum_value1{string_value1}; // Should not need this because of implicit conversion operator
         
      _db.put_batch(rand_account0, datum_value1);
      _db.put_batch(rand_account1, datum_value0);
   }

   virtual inline void write() final {
      _db.write_batch();
   }

   inline auto start_undo_session(bool enabled) {
      return _db.start_undo_session(enabled);
   }
   
private:
   chainrocks::database _db;
};
