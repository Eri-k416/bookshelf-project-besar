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
    string returnDate; // DDMMYYYYY format
    userBooks userBook;

    // Constructor
    User(UserList& userListTether, string name, string date);

    // Methods
    void updateUserBooks(shared_ptr<Book> book);
    int countBookFine(string nowDate);
    string printUserStatus();
    ~User();
};

#endif