#ifndef BOOK_HPP
#define BOOK_HPP

#include "common.hpp" 

struct bookQueues {
    shared_ptr<User> currentBorrower;
    deque<shared_ptr<User>> QueueOfUsers;
};

class Book {
public:
    int Id;
    string Isbn;
    string Title;
    string Author;
    int Year;
    string createDate;
    string lastEditDate = "";
    bookStatus Status = bookStatus::TERSEDIA;
    bookQueues bookQueue;

    // Constructor
    Book(int& bookIdTether, string isbn, string title, string author, int year, string createdate);

    // Methods
    void editBook(string isbn, string title, string author, int year);
    void updateBookQueues(shared_ptr<User> user);
    void emptyBookBorrower();
    void queueAdvance();
    int findQueuingUserIndex(int id);
    string printBookStatus();
    ~Book();
};

#endif