#pragma once

// #include <boost/interprocess/allocators/allocator.hpp> // bip::allocator
// #include <boost/interprocess/containers/map.hpp>
// #include <boost/interprocess/containers/set.hpp>
// #include <boost/interprocess/containers/flat_map.hpp>
// #include <boost/interprocess/containers/deque.hpp>
// #include <boost/interprocess/containers/string.hpp>
// #include <boost/interprocess/sync/interprocess_sharable_mutex.hpp>
// #include <boost/interprocess/sync/sharable_lock.hpp>

// #include <boost/chrono.hpp>
// #include <boost/config.hpp>
#include <boost/filesystem.hpp>
// #include <boost/lexical_cast.hpp>
// #include <boost/throw_exception.hpp>

// #include <array>
// #include <atomic>
// #include <fstream>
// #include <iostream>
// #include <memory>
// #include <stdexcept>
// #include <typeindex>
// #include <typeinfo>
#include <deque>  // std::deque
#include <map>    // std::map
#include <set>    // std::set
#include <vector> // std::vector

#include <rocksdb/db.h>

#ifndef CHAINBASE_NUM_RW_LOCKS
#define CHAINBASE_NUM_RW_LOCKS 10
#endif

#ifdef CHAINBASE_CHECK_LOCKING
#define CHAINBASE_REQUIRE_READ_LOCK(m, t) require_read_lock(m, typeid(t).name())
#define CHAINBASE_REQUIRE_WRITE_LOCK(m, t) require_write_lock(m, typeid(t).name())
#else
#define CHAINBASE_REQUIRE_READ_LOCK(m, t)
#define CHAINBASE_REQUIRE_WRITE_LOCK(m, t)
#endif

namespace chainbase {
   
   namespace bfs = boost::filesystem;
    
   /**
    * Stuff
    */
   // using read_write_mutex = boost::interprocess::interprocess_sharable_mutex;
   // using read_lock        = boost::interprocess::sharable_lock<read_write_mutex>;

   /**
     * Stuff
     */
   class rocksdb_options {
   public:
      rocksdb_options() {
         _options.create_if_missing = true;
         _options.IncreaseParallelism();
         _options.OptimizeLevelStyleCompaction();
      }

      const rocksdb::Options& options() {
         return _options;
      }
      
   private:
      rocksdb::Options _options;
   };

   /**
     * Stuff
     */
   class rocksdb_database {
   public:
      rocksdb_database(const bfs::path& directory)
      : _options{}
      {
         _status = rocksdb::DB::Open(_options.options(), directory.string(), &_database);
         check_status();
      }

      // void get(uint64_t key, std::vector<uint8_t> value) {
      //    _status = _database->Get(rocksdb::ReadOptions(), key, &value);
      //    check_status();
      // }

      // void modify(uint64_t key, std::vector<uint8_t> value) {
      //    _status = _database->Put(rocksdb::WriteOptions(), key, value);
      //    check_status();
      // }

      // void remove(uint64_t key) {
      //    _status = _database->Delete(rocksdb::WriteOptions(), key);
      //    check_status();
      // }

      ~rocksdb_database() {
         delete _database;
      }
      
   private:
      rocksdb::DB*    _database;
      rocksdb::Status _status;
      rocksdb_options _options;

      void check_status() {
         if (_status.ok()) {
            return;
         }
         else {
            BOOST_THROW_EXCEPTION(std::runtime_error("Unhandled `rocksdb` status"));
         }
      }
   };

   /**
    * Stuff
    */
   template<typename T>
   struct oid {
      oid(int64_t i = 0)
      : _id{i}
      {
      }

      oid& operator ++ () {
         ++_id; return *this;
      }

      friend bool operator <  (const oid& a, const oid& b) { return a._id <  b._id; }
      friend bool operator >  (const oid& a, const oid& b) { return a._id >  b._id; }
      friend bool operator == (const oid& a, const oid& b) { return a._id == b._id; }
      friend bool operator != (const oid& a, const oid& b) { return a._id != b._id; }
        
      friend std::ostream& operator << (std::ostream& s, const oid& id) {
         s << boost::core::demangle(typeid(oid<T>).name()) << '(' << id._id << ')';
         return s;
      }

      int64_t _id{};
   };

   /**
    * Stuff
    */
   template<uint16_t TypeNumber, typename Derived>
   struct object
   {
      typedef oid<Derived> id_type;
      static const uint16_t type_id{TypeNumber};
   };

   /**
    * Stuff
    */
   template<typename T>
   struct get_index_type
   {
   };

   /**
    *  Stuff
    */
   #define CHAINBASE_SET_INDEX_TYPE(OBJECT_TYPE, INDEX_TYPE) \
   namespace chainbase {                                     \
      template<>                                             \
      struct get_index_type<OBJECT_TYPE>                     \
      {                                                      \
         using type = INDEX_TYPE;                            \
      };                                                     \
   }

   /**
    *  Stuff
    */
   #define CHAINBASE_DEFAULT_CONSTRUCTOR(OBJECT_TYPE) \
   template<typename Constructor, typename Allocator> \
   OBJECT_TYPE(Constructor&& c, Allocator&&)          \
   {                                                  \
      c{*this};                                       \
   }

   /**
    *  Stuff
    */
   template<typename value_type>
   class undo_state
   {
   public:
      template<typename T>
      undo_state()
      : _old_values{}
      , _removed_values{}
      , _new_ids{}
      {
      }

      using id_value_type_map = std::map<uint64_t, value_type, std::less<uint64_t>>;
      using id_type_set       = std::set<uint64_t,             std::less<uint64_t>>;

      id_value_type_map _old_values;
      id_value_type_map _removed_values;
      id_type_set       _new_ids;
      uint64_t          _old_next_id{};
      int64_t           _revision{};
   };

   /**
    *  Stuff
    */
   class int_incrementer
   {
   public:
      int_incrementer(int32_t& target) : _target{target} {
         ++_target;
      }
        
      ~int_incrementer() {
         --_target;
      }

      int32_t get() const {
         return _target;
      }

   private:
      int32_t& _target;
   };

   /**
    *  Stuff
    */
   class generic_index
   {
   public:
      using index_type      = uint64_t;
      using value_type      = std::vector<uint8_t>;
      using undo_state_type = undo_state<value_type>;

      generic_index()
      : _stack{}
      , _indices{}
      , _size_of_value_type{sizeof(uint64_t)}
      , _size_of_this{sizeof(*this)}
      {
      }

      void validate() const {
         if (sizeof(uint64_t) != _size_of_value_type || sizeof(*this) != _size_of_this) {
            BOOST_THROW_EXCEPTION(std::runtime_error("content of memory does not match data expected by executable"));
         }
      }

      /**
       *  Stuff
       */
      template<typename Constructor>
      const value_type& emplace(Constructor&& c) {
         auto new_id{_next_id};

         auto constructor{[&](value_type& v) { v.id = new_id; c(v); }};

         auto insert_result{_indices.emplace(constructor, _indices.get_allocator())};

         if (!insert_result.second) {
            BOOST_THROW_EXCEPTION(std::logic_error("could not insert object, most likely a uniqueness constraint was violated"));
         }

         ++_next_id;
         on_create(*insert_result.first);
            
         return *insert_result.first;
      }

      /**
       *  Stuff
       */
      template<typename Modifier>
      void modify(const value_type& obj, Modifier&& m) {
         on_modify(obj);
            
         auto ok{_indices.modify(_indices.iterator_to(obj), m)};
            
         if(!ok) {
            std::abort(); // Uniqueness violation.
         }
      }

      /**
       *  Stuff
       */
      void remove(const value_type& obj) {
         on_remove(obj);
            
         _indices.erase(_indices.iterator_to(obj));
      }

      /**
       *  Stuff
       */
      const value_type* find(uint64_t&& key) const {
         auto iter{_indices.find(std::forward<uint64_t>(key)};
                
         if(iter != _indices.end()) {
            return &*itr;
         }   
         return nullptr;
      }

      template<typename CompatibleKey>
         const value_type& get(CompatibleKey&& key) const {
         auto ptr{find(key)};
         if(!ptr) {
            std::stringstream ss;
            ss << "key not found (" << boost::core::demangle(typeid(key).name()) << "): " << key;
            BOOST_THROW_EXCEPTION(std::out_of_range(ss.str().c_str()));
         }
         return *ptr;
      }

      const index_type& indices() const {
         return _indices;
      }

      class session {
      public:
         session(session&& mv)
         : _index{mv._index}
         , _apply{mv._apply}
         {
            mv._apply = false;
         }

         ~session() {
            if(_apply) {
               _index.undo();
            }
         }

         /** leaves the UNDO state on the stack when session goes out of scope */
         void push() {
            _apply = false;
         }
            
         /** combines this session with the prior session */
         void squash() {
            if (_apply) {
               _index.squash();
            }
            _apply = false;
         }
            
         void undo() {
            if (_apply) {
               _index.undo();
            }
            _apply = false;
         }

         session& operator = (session&& mv) {
            if(this == &mv) {
               return *this;
            }
                
            if(_apply) {
               _index.undo();
            }
                
            _apply    = mv._apply;
            mv._apply = false;
                
            return *this;
         }

         int64_t revision() const {
            return _revision;
         }

      private:
         friend class generic_index;

         session(generic_index& idx, int64_t revision)
         : _index(idx)
         , _revision(revision)
         {
            if(revision == -1) {
               _apply = false;
            }
         }

         generic_index& _index;
         bool           _apply{true};
         int64_t        _revision{};
      };

      session start_undo_session(bool enabled) {
         if(enabled) {
            _stack.emplace_back(_indices.get_allocator());
             
            _stack.back().old_next_id = _next_id;
            _stack.back().revision    = ++_revision;
             
            return session{*this, _revision};
         }
         else {
            return session{*this, -1};
         }
      }

      int64_t revision() const {
         return _revision;
      }

      /**
       *  Stuff
       */
      void undo() {
         if(!stuff_to_undo()) {
            return;
         }

         const auto& head{_stack.back()};

         for(auto id : head.new_ids) {
            _indices.erase( _indices.find(id));
         }
            _next_id = head.old_next_id;

         for(auto& item : head.old_values) {
            auto ok{_indices.modify(_indices.find(item.second.id), [&](value_type& v) { v = std::move(item.second); })};
            if(!ok) {
               std::abort(); // uniqueness violation
            }
         }

         for(auto& item : head.removed_values) {
            bool ok{_indices.emplace(std::move(item.second)).second};
            if(!ok) {
               std::abort(); // uniqueness violation
            }
         }

         _stack.pop_back();
         --_revision;
      }

      /**
       *  Stuff
       */
      void squash() {
         if(!stuff_to_undo()) {
            return;
         }
            
         if(_stack.size() == 1) {
            _stack.pop_front();
            --_revision;
            return;
         }

         auto& state{_stack.back()};
         auto& prev_state{_stack[_stack.size()-2]};

         /**
          *  Stuff
          */
         for(const auto& item : state.old_values){
            if (prev_state.new_ids.find(item.second.id) != prev_state.new_ids.end()) {
               // new+upd -> new, type A
               continue;
            }
               
            if(prev_state.old_values.find(item.second.id) != prev_state.old_values.end()) {
               // upd(was=X) + upd(was=Y) -> upd(was=X), type A
               continue;
            }
            // del+upd -> N/A
            assert(prev_state.removed_values.find(item.second.id) == prev_state.removed_values.end());
               
            // nop+upd(was=Y) -> upd(was=Y), type B
            prev_state.old_values.emplace(std::move(item));
         }

         // *+new, but we assume the N/A cases don't happen, leaving type B nop+new -> new
         for(auto id : state.new_ids) {
            prev_state.new_ids.insert(id);
         }   

         // *+del
         for(auto& obj : state.removed_values) {
            if(prev_state.new_ids.find(obj.second.id) != prev_state.new_ids.end()) {
               // new + del -> nop (type C)
               prev_state.new_ids.erase(obj.second.id);
               continue;
            }
               
            auto iter{prev_state.old_values.find(obj.second.id)};
               
            if(iter != prev_state.old_values.end()) {
               // upd(was=X) + del(was=Y) -> del(was=X)
               prev_state.removed_values.emplace( std::move(*iter) );
               prev_state.old_values.erase(obj.second.id);
               continue;
            }
               
            // del + del -> N/A
            assert(prev_state.removed_values.find(obj.second.id) == prev_state.removed_values.end());
               
            // nop + del(was=Y) -> del(was=Y)
            prev_state.removed_values.emplace(std::move(obj)); //[obj.second->id] = std::move(obj.second);
         }
            
         _stack.pop_back();
         --_revision;
      }

      /**
      * Stuff
      */
      void commit(int64_t revision)
      {
         while(_stack.size() && _stack[0].revision <= revision) {
            _stack.pop_front();
         }
      }

      /**
      * Stuff
      */
      void undo_all() {
         while(stuff_to_undo()) {
            undo();
         }
      }

      /**
      * Stuff
      */
      void set_revision(uint64_t revision) {
         if(_stack.size() != 0) {
            BOOST_THROW_EXCEPTION(std::logic_error("cannot set revision while there is an existing undo stack"));
         }
            
         if(revision > std::numeric_limits<int64_t>::max()) {
            BOOST_THROW_EXCEPTION(std::logic_error("revision to set is too high"));
         }
            
         _revision = static_cast<int64_t>(revision);
      }

      /**
      * Stuff
      */
      void remove_object(int64_t id) {
         const value_type* val{std::find(typename value_type::id_type(id))};
         if(!val) {
            BOOST_THROW_EXCEPTION(std::out_of_range(std::to_string(id)));
         }
         remove(*val);
      }

      /**
      * Stuff
      */
      std::pair<int64_t, int64_t> undo_stack_revision_range() const {
         int64_t begin{_revision};
         int64_t end{_revision};

         if(_stack.size() > 0) {
            begin = _stack.front().revision - 1;
            end   = _stack.back().revision;
         }

         return {begin, end};
      }

      /**
      * Stuff
      */
      const auto& stack() const {
         return _stack;
      }

   private:
      /**
      * Stuff
      */
      bool stuff_to_undo() const {
         return _stack.size();
      }

      /**
      * Stuff
      */
      void on_modify(const value_type& value) {
         if (!stuff_to_undo()) {
            return;
         }

         auto& head{_stack.back()};

         if(head.new_ids.find(value.id) != head.new_ids.end()) {
            return;
         }
            
         auto iter{head.old_values.find(value.id)};
         if(iter != head.old_values.end()) {
            return;
         }
            
         head.old_values.emplace(std::pair<typename value_type::id_type, const value_type&>(value.id, value)); // RocksDB insert
      }

      /**
      * Stuff
      */
      void on_remove(const value_type& value) {
         if(!stuff_to_undo()) {
            return;
         }

         auto& head{_stack.back()};
         if(head.new_ids.count(value.id)) {
            head.new_ids.erase(value.id);
            return;
         }

         auto iter{head.old_values.find(value.id)};
         if (iter != head.old_values.end()) {
            head.removed_values.emplace(std::move(*iter));
            head.old_values.erase(value.id);
            return;
         }

         if(head.removed_values.count(value.id)) {
            return;
         }   

         head.removed_values.emplace(std::pair<typename value_type::id_type, const value_type&>(value.id, value)); // Don't need id_type cuz RocksDB
      }

      /**
      * Stuff
      */
      void on_create(const value_type& value) {
         if(!stuff_to_undo()) {
            return;
         }
         auto& head{_stack.back()};
         head.new_ids.insert(value.id);
      }

      /**
      * Stuff
      */
      std::deque<undo_state_type>  _stack{};
      index_type                   _next_id{};
      int64_t                      _revision{};
      index_type                   _indices{};
      uint32_t                     _size_of_value_type{};
      uint32_t                     _size_of_this{};
   };
}
