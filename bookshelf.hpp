#ifndef BOOKSHELF_HPP
#define BOOKSHELF_HPP

#include "common.hpp"

// --- CLASS DEFINITION ---
class Bookshelf {
public:
    vector<shared_ptr<Book>> bookshelf;
    int bookId = 0;

    void addBook(string isbn, string title, string author, int year);
    int getBookIndex(int id);
    void deleteBook(int id);
};

#endif // BOOKSHELF_HPP