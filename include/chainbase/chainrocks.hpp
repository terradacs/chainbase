// clang++ -g -Wall -Wextra -std=c++17 -o prog rocks2.cpp -lboost_system; ./prog

/// [ ] TODO: change all ordinal numbering to cardinal numbering.
/// [ ] TODO: test the RAII `undo` functionality.
/// [ ] TODO: perhaps come up with a few more tests (multiple multiple unde sessions).
/// [ ] TODO: finish writing sufficient comments.
/// [X] TODO: convert tests to use boost test framework.
/// [ ] TODO: why won't `erase` erase the actualy key with the value as well?
/// [ ] TODO: revisit all comments to ensure accuracy.
/// [ ] TODO: revisit all branch to appropriately maximize branch prediction speed.
/// [ ] TODO: revisit tests to keep them logically consistent.
/// [ ] TODO: figure out why the key itself is not being erased and change respective tests.
/// [ ] TODO: integrate `std::optional`.

#pragma once

#include <boost/core/demangle.hpp>   // boost::core::demangle
#include <boost/throw_exception.hpp> // BOOST_THROW_EXCEPTION

#include <deque>    // std::deque
#include <iostream> // std::cout
#include <limits>   // std::numeric_limits
#include <map>      // std::map
#include <set>      // std::set
#include <sstream>  // std::stringstream

/// There are a few things to keep in mind when reviewing the
/// implementation of the undo functionality in chainbase. There
/// are a myriad of terms thrown around that might seem arbitrary,
/// but in fact they are vital to understand. The first of which
/// is the idea of `undo`. This is the core idea around chainbase.

/// |---------------------The Undo Deque--------------------|
///
/// |-------------------------------------------------------|
/// |       |       |       |       |       |       |       |
/// |       |       |       |       |       |       |       |
/// |-------------------------------------------------------|
///
/// TODO: Continue with a more detailed ASCII drawing later.

namespace chainrocks {

   /// Stuff.
   template<typename T>
   class oid {
   public:
      oid(int64_t i = 0)
         : _id{i}
      {
      }

      oid& operator++() { ++_id; return *this; }

      friend bool operator< ( const oid& a, const oid& b ) { return a._id  < b._id; }
      friend bool operator> ( const oid& a, const oid& b ) { return a._id  > b._id; }
      friend bool operator==( const oid& a, const oid& b ) { return a._id == b._id; }
      friend bool operator!=( const oid& a, const oid& b ) { return a._id != b._id; }
      friend std::ostream& operator<<(std::ostream& s, const oid& id) {
         s << boost::core::demangle(typeid(oid<T>).name()) << '(' << id._id << ')'; return s;
      }

      int64_t _id{};
   };

   /// Stuff.
   template<uint16_t TypeNumber, typename Derived>
   struct object {
      using id_type = oid<Derived> id_type;
      static const uint16_t type_id = TypeNumber;
   };

   /// Holds the current undo state of a particular session.
   /// For exmple: whenever `start_undo_session` gets called
   /// and set to true, a new `undo_state` object is created
   /// for that particular session. In turn, whenever the
   /// current `_state` is modified, these changes get recorded
   /// in the `undo_state` object. Then if a user chooses to undo
   /// whatever changes they've made, the information is readily
   /// available to revert back safely.
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

      /// Stuff.
      id_type _old_next_id{};

      /// Stuff.
      int64_t _revision{};
   };

   class index {
   private:
      /// The current state of the index. This is where the real meat
      /// of the data lies.
      std::map<uint64_t, std::string> _state{};
      
      /// Holds multiple `undo_state` objects to keep track of the undo
      /// sessions in the given program.
      std::deque<undo_state> _stack{};

      /// Stuff.
      int64_t _revision{};
   
   public:
      /// Stuff.
      index()
      {
      }

      /// Stuff.
      ~index()
      {
      }

      ////////////////
      /// TEMP HELPERS
      void print_state() {
         std::cout << "_state: ";
         for (const auto& value : _state) {
            std::cout << value.first << value.second << ' ';
         }
         std::cout << '\n';
      }

      void print_keys() {
         for (const auto& undo_state_obj : _stack) {
            std::cout << "_new_keys: ";
            for (const auto& key : undo_state_obj._new_keys) {
               std::cout << key << ' ';
            }
            std::cout << '\n';
         }
      }
      /// END TEMP HELPERS
      ////////////////////

      /// Stuff.
      const auto& state() const {
         return _state;
      }

      /// Stuff.
      const auto& stack() const {
         return _stack;
      }
      
      /// Add a new value the current system state or modifies a currently
      /// existing value.
      void put(const uint64_t& key, const std::string& value) {
         _on_put(key, value);
         _state[key] = value;
      }

      /// Remove a value in the current system state.
      void remove(const uint64_t& key) {
         _on_remove(key);
         _state.erase(key);
      }

      /// Look for a key in the current system state. If a value is not found
      /// a `nullptr` is returned; note that this function will not throw if it
      /// does not find the key that it's looking for.
      auto find(const uint64_t& key) -> decltype(&*_state.find(key)) {
         auto itr = _state.find(key);
         if (itr != _state.cend()) {
            return &*itr;
         }
         else {
            return nullptr;
         }
      }

      /// Look for a key in the current system state by calling the function `find`
      /// on its behalf. If a value is not found this function will throw an error.
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

      /// Undo any combination thereof: adding key value pairs, modifying key value pairs,
      /// and removing key value pairs within a particular undo session.
      void undo() {
         if(!_enabled()) {
            return;
         }

         /// Note that this is taking the back of the undo deque,
         /// this is because if it were to take the front, we would be
         /// grabbing the oldest `undo_state` object, and this would
         /// defeat the purpose of having an undo history.
         const auto& head{_stack.back()};

         _undo_new_keys(head);
         _undo_modified_values(head);
         _undo_removed_values(head);

         _stack.pop_back();
         --_revision;
      }

      /// After each `undo_state` is acted upon by a call to `undo`, it shall get popped off of the undo stack.
      /// Therefore, a call to `undo_all` will continually undo and pop states off of the stack until it is
      /// empty. The analagous function to this is `commit`; which pops states off of the stack, but does not
      /// undo the state because the said datum has been committed to the system state.
      void undo_all() {
         while (_enabled()) {
            undo();
         }
      }

      /// Commit all `undo_state`s to the system state by effectively not acting upon them in
      /// the context of an undo session.
      void commit() {
         while (_stack.size()) {
            _stack.pop_front();
         }
      }

      /// ===================================================================================================
      /// All possible results from squashing all combinations of two undo sessions given one key-value pair:
      /// ===================================================================================================
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
      ///   4.  Operation 2: (delete,K)                                       ///   4.  Operation 2: (delete,K)
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
      ///   8.  Operation 4: (delete,K)                                       ///   8.  Operation 4: (delete,K)
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
      ///   4.  Operation 2: (delete,K)                                       ///   4.  Operation 2: (delete,K)
      ///   5.  State 2:     IMPOSSIBLE                                       ///   5.  State 2:     S = {}, U = [{(removed,K,V)}]
      ///   6.  Operation 3: IMPOSSIBLE                                       ///   6.  Operation 3: (start_undo_session)
      ///   7.  State 3:     IMPOSSIBLE                                       ///   7.  State 3:     S = {}, U = [{(removed,K,V)}, {}]
      ///   8.  Operation 4: IMPOSSIBLE                                       ///   8.  Operation 4: (delete,K)
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
      ///   4.  Operation 2: (delete,K)                                       ///   4.  Operation 2: (delete,K)
      ///   5.  State 2:     IMPOSSIBLE                                       ///   5.  State 2:     S = {}, U = [{(removed,K,V)}]
      ///   6.  Operation 3: IMPOSSIBLE                                       ///   6.  Operation 3: (start_undo_session)
      ///   7.  State 3:     IMPOSSIBLE                                       ///   7.  State 3:     S = {}, U = [{(removed,K,V)}, {}]
      ///   8.  Operation 4: IMPOSSIBLE                                       ///   8.  Operation 4: (put,K,V')
      ///   9.  State 4:     IMPOSSIBLE                                       ///   9.  State 4:     S = {(K,V')}, U = [{(removed,K,V)}, {(new,K)}]
      ///   10. Operation 5: IMPOSSIBLE                                       ///   10. Operation 5: (squash)
      ///   11. State 5:     IMPOSSIBLE                                       ///   11. State 5:     S = {(K,V')}, U = [{(modifed,K,V)}]
      ///   12. Operation 6: IMPOSSIBLE                                       ///   12. Operation 6: (undo)
      ///   13. State 6:     IMPOSSIBLE                                       ///   13. State 6:     S = {(K,V)}, U = []

      /// Sqaushing is the act of taking the first two recent `undo_state`s and combining them into one.
      void squash() {
         if(!_enabled()) {
            return;
         }

         if(_stack.size() == 1) {
            _stack.pop_front();
            --_revision;
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

      /// Stuff.
      void set_revision(uint64_t revision)
      {
         if (_stack.size() != 0) {
            BOOST_THROW_EXCEPTION(std::logic_error{"cannot set revision while there is an existing undo stack"});
         }

         if (revision > std::numeric_limits<int64_t>::max()) {
            BOOST_THROW_EXCEPTION(std::logic_error{"revision to set is too high"});
         }

         _revision = static_cast<int64_t>(revision);
      }

      /// Stuff.
      std::pair<int64_t, int64_t> undo_stack_revision_range() const {
         int64_t begin{_revision};
         int64_t end{_revision};

         if (_stack.size() > 0) {
            begin = _stack.front().revision - 1;
            end   = _stack.back().revision;
         }

         return {begin, end};
      }

      /// Stuff.
      class session {
      public:
         /// Stuff.
         session(session&& s)
            : _index{s._index}, _apply{s._apply}
         {
            s._apply = false;
         }

         /// Stuff.
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

         /// Stuff.
         ~session() {
            if(_apply) {
               _index.undo();
            }
         }

         /// Stuff.
         void push() {
            _apply = false;
         }

         /// Stuff.
         void undo() {
            if (_apply) {
               _index.undo();
               _apply = false;
            }
         }

         /// Stuff.
         void squash() {
            if (_apply) {
               _index.squash();
               _apply = false;
            }
         }

         /// Stuff.
         int64_t revision() const {
            return _revision;
         }
   
      private:
         friend class index;

         /// Stuff.
         session(index& idx, int64_t revision)
            : _index{idx}
            , _revision{revision}  
         {
            if(revision == -1) {
               _apply = false;
            }  
         }

         /// Stuff.
         index& _index;

         /// Stuff.
         int64_t _revision{};

         /// Stuff.
         bool _apply{true};
      };

      session start_undo_session(bool enabled) {
         if (enabled) {
            _stack.emplace_back(undo_state{});
            _stack.back().revision = ++_revision;
            return session{*this, _revision};
         } else {
            return session{*this, -1};
         }
      }
   
   private:
      /// Update the mapping `_new_keys` to make the current undo session
      /// aware that a new value has been added to the system state.
      void _on_create(const uint64_t& key, const std::string& value) {
         if (!_enabled()) {
            return;
         }
         auto& head = _stack.back();
         head._new_keys.insert(key);
      }
   
      /// Update the mapping `_modified_values` to make the current undo session
      /// aware that a new value has been modified in the system state.
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

      /// Update the mapping `_removed_values` to make the current undo session
      /// aware that a value has been removed in the system state.
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

      /// Effectively erase any new key value pair introduced to the system state.
      void _undo_new_keys(const undo_state& head) {
         for (const auto& key : head._new_keys) {
            _state.erase(key);
         }
      }

      /// Effectively replace any modified key value pair with its previous value
      /// to the system state.
      void _undo_modified_values(const undo_state& head) {
         for (const auto& modified_value : head._modified_values) {
            _state[modified_value.first] = modified_value.second;
         }
      }

      /// Effectively reintroduce any removed key value pair from the system state
      /// back into the system state.
      void _undo_removed_values(const undo_state& head) {
         for (const auto& removed_value : head._removed_values) {
            _state[removed_value.first] = removed_value.second;
         }
      }

      /// Effectively squash new keys two individual sessions together.
      void _squash_new_keys(const undo_state& head, undo_state& head_minus_one) {
         for (const auto& key : head._new_keys) {
            head_minus_one._new_keys.insert(key);
         }
      }

      /// Effectively squash modified values from two individual sessions together.
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

      /// Effectively squash removed values from two individual sessions together.
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
   
      /// Stuff.
      bool _enabled() const {
         return _stack.size();
      }
   };

   /// Stuff.
   template<typename SessionType>
   class abstract_session {
   public:
      abstract_session(SessionType&& s)
         : _session{std::move(s)}
      {
      }
      
      ~abstract_session()
      {
      }

      void    push()           { _session.push();            }
      void    undo()           { _session.undo();            }
      void    squash()         { _session.squash();          }
      int64_t revision() const { return _session.revision(); }
   private:
      SessionType _session;
   };

   /// Stuff.
   template<typename BaseIndex>
   class abstract_index {
   public:
      abstract_index(BaseIndex& base)
         : abstract_index{&base}
         , _base{base}
      {
      }
      
      abstract_index(void* i)
         : _idx_ptr{i}
      {
      }
      
      ~abstract_index()
      {
      }
      
      void set_revision(uint64_t revision) {
         _base.set_revision(revision);
      }
      
      std::unique_ptr<abstract_session<BaseIndex>> start_undo_session(bool enabled) {
         return std::unique_ptr<abstract_session<BaseIndex>>(new abstract_session<typename BaseIndex::session>{_base.start_undo_session(enabled)});
      }

      int64_t revision() const {
         return _base.revision();
      }
      
      void undo() const {
         _base.undo();
      }
      
      void squash() const {
         _base.squash();
      }
      
      void commit(int64_t revision) const {
         _base.commit(revision);
      }
      
      void undo_all() const {
         _base.undo_all();
      }
      
      std::pair<int64_t, int64_t> undo_stack_revision_range() const {
         return _base.undo_stack_revision_range();
      }

      void* get() const {
         return _idx_ptr;
      }
      
   private:
      void* _idx_ptr;
      BaseIndex& _base;
      std::string BaseIndex_name = boost::core::demangle(typeid(typename BaseIndex::value_type).name());
   };

   /// Stuff.
   class database
   {
   public:
      /// Stuff.
      database()
      {
      }

      /// Stuff.
      ~database() {
         _index_list.clear();
         _index_map.clear();
      }

      /// Stuff.
      database(database&&) = default;

      /// Stuff.
      database& operator=(database&&) = default;

      /// Stuff.
      struct session {
      public:
         /// Stuff.
         session(session&& s)
            : _index_sessions{std::move(s._index_sessions)}
            , _revision(s._revision)
         {
         }

         /// Stuff.
         session(std::vector<std::unique_ptr<session>>&& s)
            : _index_sessions{std::move(s)}
         {
            if(_index_sessions.size()) {
               _revision = _index_sessions[0]->revision();
            }
         }

         /// Stuff.
         ~session() {
            undo();
         }

         /// Stuff.
         void push()
         {
            for (auto& i : _index_sessions) {
               i->push();
            }
            _index_sessions.clear();
         }

         /// Stuff.
         void undo() {
            for (auto& i : _index_sessions) {
               i->undo();
            }
            _index_sessions.clear();
         }

         /// Stuff.
         void squash()
         {
            for (auto& i : _index_sessions) {
               i->squash();
            }
            _index_sessions.clear();
         }

         /// Stuff.
         int64_t revision() const {
            return _revision;
         }

      private:
         /// Stuff.
         friend class database;

         /// Stuff.
         session()
         {
         }

         /// Stuff.
         std::vector<std::unique_ptr<>> _index_sessions;

         /// Stuff.
         int64_t _revision{-1};
      };

      /// Stuff.
      session start_undo_session(bool enabled) {
         if (enabled) {
            std::vector<std::unique_ptr<abstract_session>> sub_sessions;
            sub_sessions.reserve(_index_list.size());
            for (auto& item : _index_list) {
               sub_sessions.push_back(item->start_undo_session(enabled));
            }
            return session{std::move(sub_sessions)};
         } else {
            return session{};
         }
      }

      /// Stuff.
      void undo() {
         for(auto& item : _index_list) {
            item->undo();
         }
      }

      /// Stuff.
      void undo_all() {
         for (auto& item : _index_list) {
            item->undo_all();
         }
      }

      /// Stuff.
      void commit(int64_t revision) {
         for (auto& item : _index_list) {
            item->commit(revision);
         }
      }

      /// Stuff.
      void squash() {
         for (auto& item : _index_list) {
            item->squash();
         }
      }

      /// Stuff.
      int64_t revision() const {
         if (_index_list.size() == 0) {
            return -1;
         }
         return _index_list[0]->revision();
      }

      /// Stuff.
      void add_index(std::map<uint64_t, std::string>&& index) {
         if (_index_list.size() > 0) {
            auto expected_revision_range = _index_list.front()->undo_stack_revision_range();
            auto added_index_revision_range = idx_ptr->undo_stack_revision_range();

            if (added_index_revision_range.first != expected_revision_range.first ||
                added_index_revision_range.second != expected_revision_range.second) {

               index->set_revision(static_cast<uint64_t>(expected_revision_range.first));
               
               while(index.revision() < expected_revision_range.second) {
                  index.start_undo_session(true).push();
               }
            }
         }
         
         auto new_index{new index<index_type>(index)};
         _index_map[type_id].reset(new_index);
         _index_list.push_back(new_index);
      }

      // template<typename MultiIndexType, typename ByIndex>
      // auto get_index()const -> decltype( ((generic_index<MultiIndexType>*)( nullptr ))->indices().template get<ByIndex>() )
      // {
      //    CHAINBASE_REQUIRE_READ_LOCK("get_index", typename MultiIndexType::value_type);
      //    typedef generic_index<MultiIndexType> index_type;
      //    typedef index_type*                   index_type_ptr;
      //    assert( _index_map.size() > index_type::value_type::type_id );
      //    assert( _index_map[index_type::value_type::type_id] );
      //    return index_type_ptr( _index_map[index_type::value_type::type_id]->get() )->indices().template get<ByIndex>();
      // }

      // generic_index<MultiIndexType>& get_mutable_index()
      // {
      //    typedef generic_index<MultiIndexType> index_type;
      //    typedef index_type*                   index_type_ptr;
      //    assert( _index_map.size() > index_type::value_type::type_id );
      //    assert( _index_map[index_type::value_type::type_id] );
      //    return *index_type_ptr( _index_map[index_type::value_type::type_id]->get() );
      // }
      
      // const ObjectType* find( CompatibleKey&& key )const {
      //    typedef typename get_index_type< ObjectType >::type index_type;
      //    const auto& idx = get_index< index_type >().indices().template get< IndexedByType >();
      //    auto itr = idx.find( std::forward< CompatibleKey >( key ) );
      //    if( itr == idx.end() ) return nullptr;
      //    return &*itr;
      // }

      // const ObjectType* find( oid< ObjectType > key = oid< ObjectType >() ) const {
      //    typedef typename get_index_type< ObjectType >::type index_type;
      //    const auto& idx = get_index< index_type >().indices();
      //    auto itr = idx.find( key );
      //    if( itr == idx.end() ) return nullptr;
      //    return &*itr;
      // }

      // const ObjectType& get( CompatibleKey&& key ) const {
      //    auto obj = find< ObjectType, IndexedByType >( std::forward< CompatibleKey >( key ) );
      //    if( !obj ) {
      //       std::stringstream ss;
      //       ss << "unknown key (" << boost::core::demangle( typeid( key ).name() ) << "): " << key;
      //       BOOST_THROW_EXCEPTION( std::out_of_range( ss.str().c_str() ) );
      //    }
      //    return *obj;
      // }

      // const uint64_t& get(const oid< ObjectType >& key = oid< ObjectType >() ) const {
      //    auto obj = find< ObjectType >( key );
      //    if( !obj ) {
      //       std::stringstream ss;
      //       ss << "unknown key (" << boost::core::demangle( typeid( key ).name() ) << "): " << key._id;
      //       BOOST_THROW_EXCEPTION( std::out_of_range( ss.str().c_str() ) );
      //    }
      //    return *obj;
      // }

      // /// Stuff.
      // template<typename ObjectType, typename Modifier>
      // void put(const ObjectType& obj, Modifier&& m) {
      //    CHAINBASE_REQUIRE_WRITE_LOCK("modify", ObjectType);
      //    typedef typename get_index_type<ObjectType>::type index_type;
      //    get_mutable_index<index_type>().modify(obj, m);
      // }

      // /// Stuff.
      // template<typename ObjectType>
      // void remove(const ObjectType& obj) {
      //    CHAINBASE_REQUIRE_WRITE_LOCK("remove", ObjectType);
      //    typedef typename get_index_type<ObjectType>::type index_type;
      //    return get_mutable_index<index_type>().remove(obj);
      // }

   private:
      /// Stuff.
      // pinnable_mapped_file _db_file; // `rocksdb` goes here

      /// Stuff.
      std::vector<index*> _index_list;

      /// Stuff.
      std::vector<unique_ptr<index>> _index_map;
   };
}
