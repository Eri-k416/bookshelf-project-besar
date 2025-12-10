#ifndef COMMON_HPP
#define COMMON_HPP

#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <memory>
#include <deque>
using namespace std; 

// date data
extern vector<int> hari_dalam_bulan;

class Book;
class User;
class Bookshelf;
class UserList;
struct userBooks;
struct bookQueues;

// enums
enum class bookStatus {
    TERSEDIA,
    DIPINJAM
};
enum class userStatus {
    PENDING,
    MEMINJAM,
    MENGANTRI
};

bool kabisat(int tahun);
int kalkulasi_hari(string tanggal);

#endif