/**
 *  @file chainbase_interface.hpp
 *  @copyright defined in eosio/LICENSE.txt
 */

#pragma once

#include <boost/multi_index/member.hpp>        // boost::multi_index::member
#include <boost/multi_index/ordered_index.hpp> // boost::multi_index::ordered_non_unique|ordered_unique
#include <boost/multi_index_container.hpp>     // boost::multi_index_container

#include <chainbase/chainbase.hpp> // chainbase::database

#include "abstract_database.hpp" // abstract_database
#include "common.hpp"            // arbitrary_datum
#include "generated_data.hpp"    // generated_data

/**
 * Data structure to be used for testing `chainbase'. In the future
 * this should be separated out from the test itself and be used as a
 * template parameter to class `database_test' itself.
 */
struct account : public chainbase::object<0,account> {
   template<typename Constructor, typename Allocator>
   account(Constructor&& c, Allocator&& a) { c(*this); }
   id_type id;
   arbitrary_datum _account_key;
   arbitrary_datum _account_value;
};

/**
 * Boiler plate type-alias used for testing `chainbase'.
 */
using account_index = boost::multi_index_container<
   account,
   boost::multi_index::indexed_by<
      boost::multi_index::ordered_unique<boost::multi_index::member<account,account::id_type,&account::id>>,
      boost::multi_index::ordered_non_unique<BOOST_MULTI_INDEX_MEMBER(account,arbitrary_datum,_account_key)>,
      boost::multi_index::ordered_non_unique<BOOST_MULTI_INDEX_MEMBER(account,arbitrary_datum,_account_value)>
      >,
   chainbase::allocator<account>
   >;

CHAINBASE_SET_INDEX_TYPE(account, account_index)

/**
 * Implementation of the `chainbase' database interface.
 *
 * Implements the two required API calls with the necessary logic
 * required in order for `chainbase' to perform such operations.
 */
class chainbase_interface : public abstract_database<chainbase::database> {
public:
   chainbase_interface(const boost::filesystem::path& database_dir)
      : _db{database_dir, chainbase::database::read_write, (4096ULL*100000000ULL)}
   {
      _db.add_index<account_index>();
   }
   
   virtual ~chainbase_interface() final
   {   
   }
   
   virtual inline void put(arbitrary_datum key, arbitrary_datum value, void* ctx = nullptr) final {
      _db.create<account>([&](account& acc) {
         acc._account_key = key;
         acc._account_value = value;
      });
   }

   virtual inline void swap(const generated_data& gen_data, size_t i) final {
      const auto& rand_account0{_db.get(account::id_type(gen_data.swaps0()[i]))};
      const auto& rand_account1{_db.get(account::id_type(gen_data.swaps1()[i]))};
      arbitrary_datum tmp{rand_account0._account_value};

      _db.modify(rand_account0, [&](account& acc) {
         acc._account_value = rand_account1._account_value;
      });
      _db.modify(rand_account1, [&](account& acc) {
         acc._account_value = tmp;
      });
   }

   virtual inline void write() final
   {
   }

   inline auto start_undo_session(bool enabled) {
      return _db.start_undo_session(enabled);
   }
   
private:
   chainbase::database _db;
};
