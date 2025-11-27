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
    bookStatus Status = bookStatus::TERSEDIA;
    bookQueues bookQueue;

    // Constructor
    Book(Bookshelf& bookshelfTether, string isbn, string title, string author, int year);

    // Methods
    void updateBookQueues(shared_ptr<User> user);
    void emptyBookBorrower();
    void queueAdvance();
    string printBookStatus();
    ~Book();
};

#endif