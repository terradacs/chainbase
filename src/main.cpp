// cd /Users/john.debord/chainbase/src/.; clang++ -std=c++17 -I../include/. -g -o prog main.cpp -lrocksdb -lboost_system; ./prog
// clang++ -std=c++17 -I../include/. -g3 -O0 -o prog main.cpp -lrocksdb -lboost_system; ./prog
#include <iostream>
#include <rocksdb/db.h>
#include <chainbase/chainrocks.hpp>

int main() {
   std::cout << "ok\n";
   
   return 0;
}

#include <cassert>
#include <iostream>
#include <string>
#include <boost/filesystem.hpp>
#include <rocksdb/db.h>

// namespace bfs = boost::filesystem;

// class rocksdb_options {
// public:
//    rocksdb_options() {
//       /// Customize general options
//       _general_options.create_if_missing = true;
//       _general_options.IncreaseParallelism();
//       _general_options.OptimizeLevelStyleCompaction();
//    }

//    const rocksdb::Options& general_options() {
//       return _general_options;
//    }

//    const rocksdb::ReadOptions& read_options() {
//       return _read_options;
//    }
      
//    const rocksdb::WriteOptions& write_options() {
//       return _write_options;
//    }
      
// private:
//    rocksdb::Options      _general_options;
//    rocksdb::ReadOptions  _read_options;
//    rocksdb::WriteOptions _write_options;
// };

// class key {
// public:
//    key(uint64_t key)
//       : _key{key}
//    {
//    }
   
//    using key_type = uint64_t;
   
//    explicit operator rocksdb::Slice() const {
//       return rocksdb::Slice{std::to_string(_key)};
//    }
   
// private:
//    key_type _key;
// };

// class value {
// public:
//    value(const std::vector<uint8_t>& value)
//    : _value{std::vector<char>(value.cbegin(), value.cend())}
//    {
//    }
   
//    using value_type = std::vector<char>;
   
//    explicit operator rocksdb::Slice() const {
//       return rocksdb::Slice{_value.data(), _value.size()};
//    }
   
// private:
//    value_type _value;
// };

// class rocksdb_database {
// public:
//    enum class database_mode : uint8_t {
//       read_only  = 1,
//       read_write = 2
//    };
      
//    rocksdb_database(const bfs::path& directory, database_mode mode)
//    {
//       if (mode == database_mode::read_write) {
//          _status = rocksdb::DB::Open(_options.general_options(), directory.string(), &_database);
//          _check_status();
//       }
//       else if (mode == database_mode::read_only) {
//          _status = rocksdb::DB::OpenForReadOnly(_options.general_options(), directory.string(), &_database);
//          _check_status();
//       }
//    }

//    ~rocksdb_database() {
//       _database->Close();
//       _check_status();
//       delete _database;
//    }

//    void get(const class key& key, std::string &value) {
//       _status = _database->Get(_options.read_options(), slice_it_up(key), &value);
//       _check_status();
//    }

//    void put(const class key& key, const value& value) {
//       _status = _database->Put(_options.write_options(), slice_it_up(key), slice_it_up(value));
//       _check_status();
//    }

//    void remove(const class key& key) {
//       _status = _database->Delete(_options.write_options(), slice_it_up(key));
//       _check_status();
//    }

//    // Deteremined by user-defined merge operator
//    void merge(const class key& key, const value& value) {
//       _status = _database->Merge(_options.write_options(), slice_it_up(key), slice_it_up(value));
//       _check_status();
//    }
      
// private:
//    rocksdb::DB*    _database;
//    rocksdb::Status _status;
//    rocksdb_options _options;

//    inline void _check_status() {
//       if (_status.ok()) { // TODO: Use the individual status numbers
//          return;
//       }
//       else {
//          std::cerr << _status.ToString() << std::endl;
//          BOOST_THROW_EXCEPTION(std::runtime_error("Unhandled `rocksdb` status"));
//       }
//    }

//    template<typename T>
//    inline const rocksdb::Slice slice_it_up(const T& t) {
//       return static_cast<const rocksdb::Slice>(t);
//    }
// };

// int main() {
//    bfs::path path{"/tmp/test"};
//    rocksdb_database rdb{path, rocksdb_database::database_mode::read_write};

//    std::vector<uint8_t> vec{'h','e','l','l','o'};
//    std::vector<uint8_t> vec2{'s','u','p'};
//    rdb.put(0ULL, vec);
//    rdb.put(1ULL, vec);
//    rdb.put(5ULL, vec);
//    rdb.put(6ULL, vec2);
//    rdb.put(7ULL, vec2);
   
//    return 0;
// }
