/**
 *  @file chainbase_interface.cpp
 *  @copyright defined in eosio/LICENSE.txt
 */

#include "chainbase_interface.hpp"

chainbase_interface::chainbase_interface(const boost::filesystem::path& database_dir)
   : _db{database_dir, chainbase::database::read_write, (1024ULL*1024ULL*1024ULL*256ULL)} // 256GB
{
   _db.add_index<account_index>();
}

chainbase_interface::~chainbase_interface() = default;

void chainbase_interface::put(arbitrary_datum key, arbitrary_datum value, void* ctx) {
   _db.create<account>([&](account& acc) {
      acc._account_key = key;
      acc._account_value = value;
   });
}

void chainbase_interface::swap(const generated_data& gen_data, size_t i) {
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

void chainbase_interface::write()
{
}
