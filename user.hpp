#ifndef USER_HPP
#define USER_HPP

#include "common.hpp" 

struct userBooks {
    shared_ptr<Book> borrowedBook;
    shared_ptr<Book> queuingBook;
};

class User {
public:
    int Id;
    string Name;
    string borrowDate = ""; // DDMMYYYYY format
    userBooks userBook;
    userStatus Status = userStatus::PENDING;

    // Constructor
    User(int& userIdTether, string name, string date);

    // Methods
    void updateUserBooks(shared_ptr<Book>& book);
    void queueAdvanceBook();
    void emptyBorrowedBook();
    int countBookFine(string nowDate);
    string printUserStatus();
    ~User();
};

#endif