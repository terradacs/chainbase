#pragma once

#include <boost/core/demangle.hpp>   // boost::core::demangle
#include <boost/filesystem.hpp>      // boost::filesystem::path
#include <boost/throw_exception.hpp> // BOOST_THROW_EXCEPTION

#include <rocksdb/db.h>          // rocksdb::DB
#include <rocksdb/write_batch.h> // rocksdb::WriteBatch

#include <deque>    // std::deque
#include <iostream> // std::cout
#include <limits>   // std::numeric_limits
#include <map>      // std::map
#include <set>      // std::set
#include <sstream>  // std::stringstream
#include <vector>   // std::vector

namespace chainrocks {

   /// Holds the current undo state of a particular session.  For
   /// example: whenever `start_undo_session` gets called, a new
   /// `session` object is returned, and with that an `undo_state`
   /// object gets pushed onto the `_undo_stack` signifying that
   /// particular session with its own unique `_revision` number. In
   /// turn, whenever the current `_state` is modified, these changes
   /// get recorded in the `undo_state` object. Then if a user chooses
   /// to undo whatever changes they've made, the information is
   /// readily available to revert back to the original state of
   /// `_state` safely due to this data structure.
   struct undo_state {
      undo_state()
      {
      }

      /// Mapping to hold any modifications made to `_state`.
      std::map<uint64_t, std::string> _modified_values{};

      /// Mapping to hold any removed values from `_state`.
      std::map<uint64_t, std::string> _removed_values{};

      /// Set representing new keys that have been edded to `_state`.
      std::set<uint64_t> _new_keys{};

      /// The unique revision number held by each `undo_state` object.
      /// Note that this revision number will never be less than or
      /// equal to, to that of the `index` it is associated with.
      int64_t _revision{};
   };

   class index {
   private:
      /// The current state of the `index` object.
      std::map<uint64_t, std::string> _state{};

      /// Stack to hold multiple `undo_state` objects to keep track of
      /// the modifications made to `_state`.
      std::deque<undo_state> _stack{};

      /// The unique revision number held by each `index` object.
      int64_t _revision{};

   public:
      index()
      {
      }

      ~index()
      {
      }

      index(const index&)= delete;
      index& operator= (const index&) = delete;

      index(index&&)= delete;
      index& operator= (index&&) = delete;

      //////////////////////////////////
      /// Temporary helper; remove later
      void print_state() {
         std::cout << "_state:\n";
         for (const auto& value : _state) {
            std::cout << "value.first: " << value.first << ' ' << "value.second: " << value.second << '\n';
            // std::cout << value.first << value.second << ' ';
         }
         std::cout << '\n';
      }

      //////////////////////////////////
      /// Temporary helper; remove later
      void print_keys() {
         std::cout << "print_keys()\n";
         for (const auto& undo_state_obj : _stack) {
            std::cout << "_new_keys: ";
            for (const auto& key : undo_state_obj._new_keys) {
               std::cout << key << ' ';
            }
            std::cout << '\n';
         }
      }

      const auto& state() const {
         return _state;
      }

      const auto& stack() const {
         return _stack;
      }

      /// Add a new value to `_state` or modifies an existing value.
      void put(const uint64_t& key, const std::string& value) {
         _on_put(key, value);
         _state[key] = value;
      }

      /// Remove a value from `_state`.
      void remove(const uint64_t& key) {
         _on_remove(key);
         _state.erase(key);
      }

      /// Look for a key in `_state`. If a value is not found a
      /// `nullptr` is returned. note that this function will not
      /// throw if it does not find the key that it's looking for.
      auto find(const uint64_t& key) -> decltype(&*_state.find(key)) {
         auto itr = _state.find(key);
         if (itr != _state.cend()) {
            return &*itr;
         }
         else {
            return nullptr;
         }
      }

      /// Look for a key in `_state` by calling the function `find` on
      /// its behalf. If a value is not found this function will throw
      /// an error.
      auto get(const uint64_t& key) -> decltype(*find(key)) {
         auto ptr = find(key);
         if(!ptr) {
            std::stringstream ss;
            ss << "Key not found!\n"
               << "(" << boost::core::demangle(typeid(key).name()) << "): " << key;
            BOOST_THROW_EXCEPTION(std::out_of_range{ss.str().c_str()});
         }
         else {
            return *ptr;
         }
      }

      /// Undo any combination of the following actions done to the
      /// `_state`: adding key/value pairs, modifying key/value pairs,
      /// and removing keys (key/value pairs).
      void undo() {
         if(!_enabled()) {
            return;
         }

         const auto& head{_stack.back()};

         _undo_new_keys(head);
         _undo_modified_values(head);
         _undo_removed_values(head);

         _stack.pop_back();
         --_revision;
      }

      /// After each `undo_state` is acted on appropriately by a call
      /// to `undo`, it shall get popped off of the `_undo_stack`.
      /// Therefore, a call to `undo_all` will continually undo and
      /// pop states off of the stack until `_undo_stack` is
      /// empty. The analagous function to this is `commit`; which
      /// clears the `_undo_stack`, and does not act upon any of the
      /// pushed `undo_state` objects.
      void undo_all() {
         while (_enabled()) {
            undo();
         }
      }

      /// Commit all `undo_state`s to the `_state` by effectively not
      /// acting upon any of the `undo_state` objects on the `_stack`.
      void commit() {
         while (_stack.size()) {
            _stack.clear();
         }
      }

      /// All possible combinations of squashing two `undo_state` objects together:
      ///
      /// Case 1:                                                             /// Case 1':
      ///   1.  State 0:     S = {}, U = []                                   ///   1.  State 0:     S = {(K,V)}, U = []
      ///   2.  Operation 1: (start_undo_session)                             ///   2.  Operation 1: (start_undo_session)
      ///   3.  State 1:     S = {}, U = [{}]                                 ///   3.  State 1:     S = {(K,V)}, U = [{}]
      ///   4.  Operation 2: ()                                               ///   4.  Operation 2: ()
      ///   5.  State 2:     S = {}, U = [{}]                                 ///   5.  State 2:     S = {(K,V)}, U = [{}]
      ///   6.  Operation 3: (start_undo_session)                             ///   6.  Operation 3: (start_undo_session)
      ///   7.  State 3:     S = {}, U = [{},{}]                              ///   7.  State 3:     S = {(K,V)}, U = [{},{}]
      ///   8.  Operation 4: ()                                               ///   8.  Operation 4: ()
      ///   9.  State 4:     S = {}, U = [{},{}]                              ///   9.  State 4:     S = {(K,V)}, U = [{},{}]
      ///   10. Operation 5: (squash)                                         ///   10. Operation 5: (squash)
      ///   11. State 5:     S = {}, U = [{}]                                 ///   11. State 5:     S = {(K,V)}, U = [{}]
      ///   12. Operation 6: (undo)                                           ///   12. Operation 6: (undo)
      ///   13. State 6:     S = {}, U = []                                   ///   13. State 6:     S = {(K,V)}, U = []
      ///                                                                     ///
      /// Case 2:                                                             /// Case 2':
      ///   1.  State 0:     S = {}, U = []                                   ///   1.  State 0:     S = {(K,V)}, U = []
      ///   2.  Operation 1: (start_undo_session)                             ///   2.  Operation 1: (start_undo_session)
      ///   3.  State 1:     S = {}, U = [{}]                                 ///   3.  State 1:     S = {(K,V)}, U = [{}]
      ///   4.  Operation 2: (put,K,V)                                        ///   4.  Operation 2: (put,K,V')
      ///   5.  State 2:     S = {(K,V)}, U = [{(new,K)}]                     ///   5.  State 2:     S = {(K,V')}, U = [{(update, K,V)}]
      ///   6.  Operation 3: (start_undo_session)                             ///   6.  Operation 3: (start_undo_session)
      ///   7.  State 3:     S = {(K,V)}, U = [{(new,K)}, {}]                 ///   7.  State 3:     S = {(K,V')}, U = [{(modified, K,V)}, {}]
      ///   8.  Operation 4: ()                                               ///   8.  Operation 4: ()
      ///   9.  State 4:     S = {(K,V)}, U = [{(new,K)}, {}]                 ///   9.  State 4:     S = {(K,V')}, U = [{(modified, K,V)}, {}]
      ///   10. Operation 5: (squash)                                         ///   10. Operation 5: (squash)
      ///   11. State 5:     S = {(K,V)}, U = [{(new,K)}]                     ///   11. State 5:     S = {(K,V')}, U = [{(modified, K,V)}]
      ///   12. Operation 6: (undo)                                           ///   12. Operation 6: (undo)
      ///   13. State 6:     S = {}, U = []                                   ///   13. State 6:     S = {(K,V)}, U = []
      ///                                                                     ///
      /// Case 3:                                                             /// Case 3':
      ///   1.  State 0:     S = {}, U = []                                   ///   1.  State 0:     S = {K,V}, U = []
      ///   2.  Operation 1: (start_undo_session)                             ///   2.  Operation 1: (start_undo_session)
      ///   3.  State 1:     S = {}, U = [{}]                                 ///   3.  State 1:     S = {K,V}, U = [{}]
      ///   4.  Operation 2: ()                                               ///   4.  Operation 2: ()
      ///   5.  State 2:     S = {}, U = [{}]                                 ///   5.  State 2:     S = {}, U = [{}]
      ///   6.  Operation 3: (start_undo_session)                             ///   6.  Operation 3: (start_undo_session)
      ///   7.  State 3:     S = {}, U = [{}, {}]                             ///   7.  State 3:     S = {}, U = [{}, {}]
      ///   8.  Operation 4: (put,K,V)                                        ///   8.  Operation 4: (put,K,V')
      ///   9.  State 4:     S = {(K,V)}, U = [{}, {(new, K)}]                ///   9.  State 4:     S = {(K,V')}, U = [{}, {(modified,K,V)}]
      ///   10. Operation 5: (squash)                                         ///   10. Operation 5: (squash)
      ///   11. State 5:     S = {(K,V)}, U = [{(new, K)}]                    ///   11. State 5:     S = {(K,V')}, U = [{(modified,K,V)}]
      ///   12. Operation 6: (undo)                                           ///   12. Operation 6: (undo)
      ///   13. State 6:     S = {}, U = []                                   ///   13. State 6:     S = {K,V}, U = []
      ///                                                                     ///
      /// Case 4:                                                             /// Case 4':
      ///   1.  State 0:     S = {}, U = []                                   ///   1.  State 0:     S = {K,V}, U = []
      ///   2.  Operation 1: (start_undo_session)                             ///   2.  Operation 1: (start_undo_session)
      ///   3.  State 1:     S = {}, U = [{}]                                 ///   3.  State 1:     S = {}, U = [{}]
      ///   4.  Operation 2: (put,K,V)                                        ///   4.  Operation 2: (put,K,V')
      ///   5.  State 2:     S = {(K,V)}, U = [{(new,K)}]                     ///   5.  State 2:     S = {(K,V')}, U = [{(modified,K,V)}]
      ///   6.  Operation 3: (start_undo_session)                             ///   6.  Operation 3: (start_undo_session)
      ///   7.  State 3:     S = {(K,V)}, U = [{(new,K)}, {}]                 ///   7.  State 3:     S = {(K,V')}, U = [{(modified,K,V)}, {}]
      ///   8.  Operation 4: (put,K,V')                                       ///   8.  Operation 4: (put,K,V'')
      ///   9.  State 4:     S = {(K,V')}, U = [{(new,K)}, {(modified,K,V)}]  ///   9.  State 4:     S = {(K, V'')}, U = [{(modified,K,V)}, {(modified,K,V')}]
      ///   10. Operation 5: (squash)                                         ///   10. Operation 5: (squash)
      ///   11. State 5:     S = {(K,V')}, U = [{(new,K)}]                    ///   11. State 5:     S = {(K, V'')}, U = [{(modified,K,V)}]
      ///   12. Operation 6: (undo)                                           ///   12. Operation 6: (undo)
      ///   13. State 6:     S = {}, U = []                                   ///   13. State 6:     S = {K,V}, U = []
      ///                                                                     ///
      /// Case 5:                                                             /// Case 5':
      ///   1.  State 0:     S = {}, U = []                                   ///   1.  State 0:     S = {(K,V)}, U = []
      ///   2.  Operation 1: (start_undo_session)                             ///   2.  Operation 1: (start_undo_session)
      ///   3.  State 1:     S = {}, U = [{}]                                 ///   3.  State 1:     S = {(K,V)}, U = [{}]
      ///   4.  Operation 2: (remove,K)                                       ///   4.  Operation 2: (remove,K)
      ///   5.  State 2:     IMPOSSIBLE                                       ///   5.  State 2:     S = {}, U = [{(removed,K,V)}]
      ///   6.  Operation 3: IMPOSSIBLE                                       ///   6.  Operation 3: (start_undo_session)
      ///   7.  State 3:     IMPOSSIBLE                                       ///   7.  State 3:     S = {}, U = [{(removed,K,V)}, {}]
      ///   8.  Operation 4: IMPOSSIBLE                                       ///   8.  Operation 4: ()
      ///   9.  State 4:     IMPOSSIBLE                                       ///   9.  State 4:     S = {}, U = [{(removed,K,V)}, {}]
      ///   10. Operation 5: IMPOSSIBLE                                       ///   10. Operation 5: (squash)
      ///   11. State 5:     IMPOSSIBLE                                       ///   11. State 5:     S = {}, U = [{(removed,K,V)}]
      ///   12. Operation 6: IMPOSSIBLE                                       ///   12. Operation 6: (undo)
      ///   13. State 6:     IMPOSSIBLE                                       ///   13. State 6:     S = {(K,V)}, U = []
      ///                                                                     ///
      /// Case 6:                                                             /// Case 6':
      ///   1.  State 0:     S = {}, U = []                                   ///   1.  State 0:     S = {(K,V)}, U = []
      ///   2.  Operation 1: (start_undo_session)                             ///   2.  Operation 1: (start_undo_session)
      ///   3.  State 1:     S = {}, U = [{}]                                 ///   3.  State 1:     S = {(K,V)}, U = [{}]
      ///   4.  Operation 2: ()                                               ///   4.  Operation 2: ()
      ///   5.  State 2:     S = {}, U = [{}]                                 ///   5.  State 2:     S = {(K,V)}, U = [{}]
      ///   6.  Operation 3: (start_undo_session)                             ///   6.  Operation 3: (start_undo_session)
      ///   7.  State 3:     S = {}, U = [{}, {}]                             ///   7.  State 3:     S = {(K,V)}, U = [{}, {}]
      ///   8.  Operation 4: (remove,K)                                       ///   8.  Operation 4: (remove,K)
      ///   9.  State 4:     IMPOSSIBLE                                       ///   9.  State 4:     S = {}, U = [{}, {(removed,K,V)}]
      ///   10. Operation 5: IMPOSSIBLE                                       ///   10. Operation 5: (squash)
      ///   11. State 5:     IMPOSSIBLE                                       ///   11. State 5:     S = {}, U = [{(removed,K,V)}]
      ///   12. Operation 6: IMPOSSIBLE                                       ///   12. Operation 6: (undo)
      ///   13. State 6:     IMPOSSIBLE                                       ///   13. State 6:     S = {(K,V)}, U = []
      ///                                                                     ///
      ///
      /// Case 7:                                                             /// Case 7':
      ///   1.  State 0:     S = {}, U = []                                   ///   1.  State 0:     S = {(K,V)}, U = []
      ///   2.  Operation 1: (start_undo_session)                             ///   2.  Operation 1: (start_undo_session)
      ///   3.  State 1:     S = {}, U = [{}]                                 ///   3.  State 1:     S = {(K,V)}, U = [{}]
      ///   4.  Operation 2: (remove,K)                                       ///   4.  Operation 2: (remove,K)
      ///   5.  State 2:     IMPOSSIBLE                                       ///   5.  State 2:     S = {}, U = [{(removed,K,V)}]
      ///   6.  Operation 3: IMPOSSIBLE                                       ///   6.  Operation 3: (start_undo_session)
      ///   7.  State 3:     IMPOSSIBLE                                       ///   7.  State 3:     S = {}, U = [{(removed,K,V)}, {}]
      ///   8.  Operation 4: IMPOSSIBLE                                       ///   8.  Operation 4: (remove,K)
      ///   9.  State 4:     IMPOSSIBLE                                       ///   9.  State 4:     IMPOSSIBLE
      ///   10. Operation 5: IMPOSSIBLE                                       ///   10. Operation 5: IMPOSSIBLE
      ///   11. State 5:     IMPOSSIBLE                                       ///   11. State 5:     IMPOSSIBLE
      ///   12. Operation 6: IMPOSSIBLE                                       ///   12. Operation 6: IMPOSSIBLE
      ///   13. State 6:     IMPOSSIBLE                                       ///   13. State 6:     IMPOSSIBLE
      ///                                                                     ///
      /// Case 8:                                                             /// Case 8':
      ///   1.  State 0:     S = {}, U = []                                   ///   1.  State 0:     S = {K,V}, U = []
      ///   2.  Operation 1: (start_undo_session)                             ///   2.  Operation 1: (start_undo_session)
      ///   3.  State 1:     S = {}, U = [{}]                                 ///   3.  State 1:     S = {K,V}, U = [{}]
      ///   4.  Operation 2: (put,K,V)                                        ///   4.  Operation 2: (put,K,V')
      ///   5.  State 2:     S = {(K,V)}, U = [{(new,K)}]                     ///   5.  State 2:     S = {(K,V')}, U = [{(modified,K,V)}]
      ///   6.  Operation 3: (start_undo_session)                             ///   6.  Operation 3: (start_undo_session)
      ///   7.  State 3:     S = {(K,V)}, U = [{(new, K)}, {}]                ///   7.  State 3:     S = {(K,V)}, U = [{(modified,K,V)}, {}]
      ///   8.  Operation 4: (del,K)                                          ///   8.  Operation 4: (del,K)
      ///   9.  State 4:     S = {}, U = [{(new,K)}, {(removed,K,V)}]         ///   9.  State 4:     S = {}, U = [{(modified,K,V)}, {(removed,K,V')}]
      ///   10. Operation 5: (squash)                                         ///   10. Operation 5: (squash)
      ///   11. State 5:     S = {}, U = [{}]                                 ///   11. State 5:     S = {}, U = [{modifed,K,V}]
      ///   12. Operation 6: (undo)                                           ///   12. Operation 6: (undo)
      ///   13. State 6:     S = {}, U = []                                   ///   13. State 6:     S = {(K,V)}, U = []
      ///                                                                     ///
      /// Case 9:                                                             /// Case 9':
      ///   1.  State 0:     S = {}, U = []                                   ///   1.  State 0:     S = {(K,V)}, U = []
      ///   2.  Operation 1: (start_undo_session)                             ///   2.  Operation 1: (start_undo_session)
      ///   3.  State 1:     S = {}, U = [{}]                                 ///   3.  State 1:     S = {(K,V)}, U = [{}]
      ///   4.  Operation 2: (remove,K)                                       ///   4.  Operation 2: (remove,K)
      ///   5.  State 2:     IMPOSSIBLE                                       ///   5.  State 2:     S = {}, U = [{(removed,K,V)}]
      ///   6.  Operation 3: IMPOSSIBLE                                       ///   6.  Operation 3: (start_undo_session)
      ///   7.  State 3:     IMPOSSIBLE                                       ///   7.  State 3:     S = {}, U = [{(removed,K,V)}, {}]
      ///   8.  Operation 4: IMPOSSIBLE                                       ///   8.  Operation 4: (put,K,V')
      ///   9.  State 4:     IMPOSSIBLE                                       ///   9.  State 4:     S = {(K,V')}, U = [{(removed,K,V)}, {(new,K)}]
      ///   10. Operation 5: IMPOSSIBLE                                       ///   10. Operation 5: (squash)
      ///   11. State 5:     IMPOSSIBLE                                       ///   11. State 5:     S = {(K,V')}, U = [{(modifed,K,V)}]
      ///   12. Operation 6: IMPOSSIBLE                                       ///   12. Operation 6: (undo)
      ///   13. State 6:     IMPOSSIBLE                                       ///   13. State 6:     S = {(K,V)}, U = []

      /// Sqaushing is the act of taking the first two recent
      /// `undo_state`s and combining them into one.
      void squash() {
         if(!_enabled()) {
            return;
         }

         if(_stack.size() == 1) {
            _stack.pop_front();
            return;
         }

         auto& head = _stack.back();
         auto& head_minus_one = _stack[_stack.size()-2];

         _squash_new_keys(head, head_minus_one);
         _squash_modified_values(head, head_minus_one);
         _squash_removed_values(head, head_minus_one);

         _stack.pop_back();
         --_revision;
      }

      /// A `session` is allowed to hold only one `index`. And is
      /// allowed to specify if its given `index` is eligible to acted
      /// upon be `undo` or `squash` via the boolean value `_apply`.
      /// Note that the session be explicitly told not to do so, by
      /// calling the method `push`.
      class session {
      public:
         session(const session&) = delete;
         session& operator= (const session&) = delete;

         session(session&& s)
            : _index{s._index}
            , _apply{s._apply}
         {
            s._apply = false;
         }

         session& operator= (session&& s) {
            if (this == &s) {
               return *this;
            }
      
            if (_apply) {
               _index.undo();
            }
         
            _apply = s._apply;
            s._apply = false;
            return *this;
         }

         /// RAII functionality; upon the destruction of `session` it
         /// shall be determined whether or not the `_index` is acted
         /// upon by `undo`.
         ~session() {
            if(_apply) {
               _index.undo();
            }
         }

         void push() {
            _apply = false;
         }

         void undo() {
            if (_apply) {
               _index.undo();
               _apply = false;
            }
         }

         void squash() {
            if (_apply) {
               _index.squash();
               _apply = false;
            }
         }

         int64_t revision() const {
            return _revision;
         }

      private:
         friend class index;

         /// Note that it is not possible to directly construct a
         /// session object because this responsibility shall be left
         /// to the method `start_undo_session`.
         session(index& idx, int64_t revision)
            : _index{idx}
            , _revision{revision}
         {
            if(_revision == -1) {
               _apply = false;
            }
         }

         /// The given `index` for this session to hold.
         index& _index;

         /// The predicate determining whether or not this `session`
         /// is elligle to be acted upon by either `undo` or `squash`.
         bool _apply{true};

         /// The unique revision number held by each `session` object.
         /// Note that this `_revision` number and the `_revision`
         /// number of the held `index` will be exactly identical to
         /// eachother.
         int64_t _revision{};
      };

      /// Probably the most interested method in the chainbase
      /// repository. It can be essentially thought of as one of two
      /// ideas: the start of a block (containing transactions) in a
      /// blockchain, or the start of a transaction (containing
      /// actions) within a block. Each of which have the ability to
      /// `commit` if valid or to `undo` if invalid due to the nature
      /// of these data structures.
      session start_undo_session(bool enabled) {
         if (enabled) {
            _stack.emplace_back(undo_state{});
            _stack.back()._revision = ++_revision;
            return session{*this, _revision};
         } else {
            return session{*this, -1};
         }
      }

   private:
      /// Update the mapping `_new_keys` of the most recently created
      /// `undo_state` object.
      void _on_create(const uint64_t& key, const std::string& value) {
         if (!_enabled()) {
            return;
         }
         auto& head = _stack.back();
         head._new_keys.insert(key);
      }

      /// Update the mapping `_modified_values` of the most recently
      /// created `undo_state` object.
      void _on_put(const uint64_t& key, const std::string& value) {
         if (!_enabled()) {
            return;
         }

         auto& head = _stack.back();

         if (head._new_keys.find(key) != head._new_keys.cend()) {
            return;
         }

         if (head._modified_values.find(key) != head._modified_values.cend()) {
            return;
         }

         if ((head._new_keys.find(key) == head._new_keys.cend()) &&
             (head._modified_values.find(key) == head._modified_values.cend()) &&
             (_state.find(key) == _state.cend())) {
            _on_create(key, value);
            return;
         }

         head._modified_values[key] = _state[key];
      }

      /// Update the mapping `_removed_values` of the most recently
      /// created `undo_state` object.
      void _on_remove(const uint64_t& key) {
         if (!_enabled()) {
            return;
         }
         else {
            auto& head = _stack.back();

            if (head._removed_values.find(key) != head._removed_values.cend()) {
               BOOST_THROW_EXCEPTION(std::runtime_error{"on_remove"});
            }

            head._removed_values[key] = _state[key];
         }
      }

      /// Effectively erase any new key/value pairs introduced to
      /// `_state`.
      void _undo_new_keys(const undo_state& head) {
         for (const auto& key : head._new_keys) {
            _state.erase(key);
         }
      }

      /// Effectively replace any modified key/value pairs with its
      /// previous value to `_state`.
      void _undo_modified_values(const undo_state& head) {
         for (const auto& modified_value : head._modified_values) {
            _state[modified_value.first] = modified_value.second;
         }
      }

      /// Effectively reintroduce any removed key/value pairs from
      /// `_state` back into `_state`.
      void _undo_removed_values(const undo_state& head) {
         for (const auto& removed_value : head._removed_values) {
            _state[removed_value.first] = removed_value.second;
         }
      }

      /// Effectively squash `_new_keys` of the previous two
      /// `undo_state` objects on the `_stack` together.
      void _squash_new_keys(const undo_state& head, undo_state& head_minus_one) {
         for (const auto& key : head._new_keys) {
            head_minus_one._new_keys.insert(key);
         }
      }

      /// Effectively squash `_modifed_values` of the previous two
      /// `undo_state` objects on the `_stack` together.
      void _squash_modified_values(const undo_state& head, undo_state& head_minus_one) {
         for (const auto& value : head._modified_values) {
            if (head_minus_one._new_keys.find(value.first) != head_minus_one._new_keys.cend()) {
               continue;
            }
            if (head_minus_one._modified_values.find(value.first) != head_minus_one._modified_values.cend()) {
               continue;
            }
            assert(head_minus_one._removed_values.find(value.first) == head_minus_one._removed_values.cend());
            head_minus_one._modified_values[value.first] = value.second;
         }
      }

      /// Effectively squash `_removed_values` of the previous two
      /// `undo_state` objects on the `_stack` together.
      void _squash_removed_values(const undo_state& head, undo_state& head_minus_one) {
         for (const auto& value : head._removed_values) {
            if (head_minus_one._new_keys.find(value.first) != head_minus_one._new_keys.cend()) {
               head_minus_one._new_keys.erase(value.first);
               continue;
            }

            auto iter{head_minus_one._modified_values.find(value.first)};
            if (iter != head_minus_one._modified_values.cend()) {
               head_minus_one._removed_values[iter->first] = iter->second;
               head_minus_one._modified_values.erase(value.first);
               continue;
            }
            assert(head_minus_one._removed_values.find(value.first) == head_minus_one._removed_values.cend());
            head_minus_one._removed_values[value.first] = value.second;
         }
      }

      /// If the `_stack` is not empty, then it is eligible to be
      /// acted upon by `undo` or `squash`.
      bool _enabled() const {
         return _stack.size();
      }
   };

   /// The data structure representing the options available to
   /// modify/tune/adjust the behavior of RocksDB.
   class rocksdb_options {
   public:
      rocksdb_options() {
         _general_options.create_if_missing = true;
         _general_options.IncreaseParallelism();
         _general_options.OptimizeLevelStyleCompaction();
      }

      ~rocksdb_options()
      {
      }

      rocksdb_options(const rocksdb_options&) = delete;
      rocksdb_options& operator= (const rocksdb_options&) = delete;

      rocksdb_options(rocksdb_options&&) = delete;
      rocksdb_options& operator= (rocksdb_options&&) = delete;

      const rocksdb::Options& general_options() {
         return _general_options;
      }

      const rocksdb::ReadOptions& read_options() {
         return _read_options;
      }

      const rocksdb::WriteOptions& write_options() {
         return _write_options;
      }

   private:
      rocksdb::Options      _general_options;
      rocksdb::ReadOptions  _read_options;
      rocksdb::WriteOptions _write_options;
   };

   /// The data structure representing a RocksDB database itself. It
   /// has the ability to introduce/modify (by `put`) new key/value
   /// pairs to `_state`, as well as removed them (by `remove`).
   class rocksdb_database {
   public:
      rocksdb_database(const boost::filesystem::path& data_dir)
         : _data_dir{data_dir}
      {
         _status = rocksdb::DB::Open(_options.general_options(), _data_dir.string().c_str(), &_databaseman);
         _check_status();
      }

      ~rocksdb_database() {
         _databaseman->Close();
         delete _databaseman;
         _check_status();
      }

      rocksdb_database(const rocksdb_database&) = delete;
      rocksdb_database& operator= (const rocksdb_database&) = delete;

      rocksdb_database(rocksdb_database&&) = delete;
      rocksdb_database& operator= (rocksdb_database&&) = delete;

      void put(const uint64_t key, const std::string& value) {
         _status = _databaseman->Put(_options.write_options(), std::to_string(key), value);
         _check_status();
      }

      void remove(const uint64_t key) {
         _status = _databaseman->Delete(_options.write_options(), std::to_string(key));
         _check_status();
      }

      void get(const uint64_t key, std::string &value) {
         _status = _databaseman->Get(_options.read_options(), _databaseman->DefaultColumnFamily(), std::to_string(key), &value);
         _check_status();
      }

      bool does_key_exist(const uint64_t key, std::string tmp = {}) {
         bool ret{_databaseman->KeyMayExist(_options.read_options(), std::to_string(key), &tmp)};
         _check_status();
         return ret;
      }

      // void put_batch(rocksdb::Slice key, rocksdb::Slice value) { // Replace `Slice` with `string`?
      //    _status = _write_batchman.Put(rocksdb::Slice(key.data(), key.size()), rocksdb::Slice(value.data(), value.size()));
      //    _check_status();
      // }

      // void remove_batch(rocksdb::Slice key) { // Replace `Slice` with `string`?
      //    _status = _write_batchman.Delete(rocksdb::Slice(key.data(), key.size()));
      //    _check_status();
      // }

      // void write_batch() {
      //    _status = _databaseman->Write(_options.write_options(), &_write_batchman);
      //    _check_status();
      // }

   private:
      rocksdb::DB* _databaseman;
      // rocksdb::WriteBatch _write_batchman;
      rocksdb::Status _status;
      rocksdb_options _options;
      boost::filesystem::path _data_dir;

      inline void _check_status() const {
         if (_status.ok()) {
            return;
         }
         else {
            std::cout << "Encountered error: " << _status.code() << '\n';
         }
      }
   };

   /// The database.  For now the database will just consist of one
   /// `index` for the time being for testing purposes. In the future,
   /// it is planned that it will exand to an unlimited number of
   /// indices; resource permitting.
   class database : public index
   {
   public:
      database(const boost::filesystem::path& data_dir)
         : _database{data_dir}
      {
      }

      ~database()
      {
      }

      database(const database&) = delete;
      database& operator= (const database&) = delete;

      database(database&&) = delete;
      database& operator= (database&&) = delete;

      //////////////////////////////////////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////////////////////////
      /// `index` stuff.

      void print_state() {
         index::print_state();
      }

      void print_keys() {
         index::print_keys();
      }

      auto state() const -> decltype(index::state()) {
         return index::state();
      }

      auto stack() const -> decltype(index::stack()) {
         return index::stack();
      }

      void put(const uint64_t& key, const std::string& value) {
         index::put(key, value);
      }

      void remove(const uint64_t& key) {
         index::remove(key);
      }

      auto find(const uint64_t& key) -> decltype(&*index::state().find(key)) {
         return index::find(key);
      }

      auto get(const uint64_t& key) -> decltype(*find(key)) {
         return index::get(key);
      }

      void undo() {
         index::undo();
      }

      void undo_all() {
         index::undo_all();
      }

      void commit() {
         index::commit();
      }

      void squash() {
         index::squash();
      }

      session start_undo_session(bool enabled) {
         return index::start_undo_session(enabled);
      }

      //////////////////////////////////////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////////////////////////
      /// `rocksdb` stuff.

      void rocksdb_put(const uint64_t key, const std::string& value) {
         _database.put(key, value);
      }

      void rocksdb_remove(const uint64_t key) {
         _database.remove(key);
      }

      void rocksdb_get(const uint64_t key, std::string &value) {
         _database.get(key, value);
      }

      bool rocksdb_does_key_exist(const uint64_t key, std::string tmp = {}) {
         return _database.does_key_exist(key, tmp);
      }

      // void put_batch(rocksdb::Slice key, rocksdb::Slice value) { // Replace `Slice` with `string`?
      //    _status = _write_batchman.Put(rocksdb::Slice(key.data(), key.size()), rocksdb::Slice(value.data(), value.size()));
      //    _check_status();
      // }

      // void remove_batch(rocksdb::Slice key) { // Replace `Slice` with `string`?
      //    _status = _write_batchman.Delete(rocksdb::Slice(key.data(), key.size()));
      //    _check_status();
      // }

      // void write_batch() {
      //    _status = _databaseman->Write(_options.write_options(), &_write_batchman);
      //    _check_status();
      // }

   private:
      rocksdb_database _database;
   };
}
