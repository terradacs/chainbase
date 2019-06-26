// cd /Users/john.debord/chainbase/src/.; clang++ -std=c++17 -I../include/. -g -o prog main.cpp -lrocksdb -lboost_system; ./prog
#include <iostream>
#include <chainbase/chainrocks.hpp>

int main() {
   std::cout << "ok\n";
   return 0;
}
