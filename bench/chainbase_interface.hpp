/**
 *  @file chainbase_interface.hpp
 *  @copyright defined in eosio/LICENSE.txt
 */

#pragma once

#include <boost/filesystem.hpp>                // boost::filesystem::path
#include <boost/multi_index/member.hpp>        // boost::multi_index::member
#include <boost/multi_index/ordered_index.hpp> // boost::multi_index::ordered_non_unique|ordered_unique
#include <boost/multi_index_container.hpp>     // boost::multi_index_container

#include <chainbase/chainbase.hpp> // chainbase::database

#include "abstract_database.hpp" // abstract_database
#include "common.hpp"            // arbitrary_datum
#include "generated_data.hpp"    // generated_data

/**
 * Data structure for which the `multi_index' table operates on during
 * the benchmark `chainbase' to operate on.
 */
struct account : public chainbase::object<0,account> {
   template<typename Constructor, typename Allocator>
   account(Constructor&& c, Allocator&& a) { c(*this); }
   id_type id;
   arbitrary_datum _account_key;
   arbitrary_datum _account_value;
};

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
 * Implementation of `chainbase' interface for the benchmark.
 *
 * `chainbase' operates purely in RAM. This is not suitable for
 * vertically scalability, but may be the fastest alternative out
 * there.
 */
class chainbase_interface : public abstract_database<chainbase::database> {
public:
   /**
    * Constructor; takes the path for which to store the state of
    * `chainbase' database.
    */
   chainbase_interface(const boost::filesystem::path& database_dir);

   /**
    * Destructor; normal operation.
    */
   virtual ~chainbase_interface() final;

   /**
    * Put both the `key' and the `value' in the database. In the case
    * of `chainbase', it shall be put into RAM. `ctx' pointer has no
    * use here; therefore it is not used.
    */
   virtual void put(arbitrary_datum key, arbitrary_datum value, void* ctx = nullptr) final;

   /**
    * Modify the state of `chainbase' by perfoming a canonical
    * swap. It is essentially taking two indices in the underlying
    * `multi_index_table' and performing a swap on the given accounts.
    */
   virtual void swap(const generated_data& gen_data, size_t i) final;

   /**
    * Performs a no-op, since `chainbase' does not provide a
    * batch-writing facility.
    */
   virtual void write() final;

   /**
    * Returns the underlying `session' object that `chainbase'
    * provides. TODO: Factor this out; the user should not be aware of
    * this abstraction.
    */
   auto start_undo_session(bool enabled) {
      return _db.start_undo_session(enabled);
   }
   
private:
   /**
    * The underlying database of `chainbase'. Note that the state will
    * get destroyed upon a successful exit of the program. Therefore,
    * it cannot be inspected.
    */
   chainbase::database _db;
};
