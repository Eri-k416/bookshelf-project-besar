#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <optional>
using namespace std;

enum class bookStatus {
    TERSEDIA,
    DIPINJAM          
};
enum class userStatus {
    PENDING,
    MEMINJAM,
    MENGANTRI
};
enum class getChoices {
	VALUE,
	INDEX
};
class UserList {
    public:
        vector<reference_wrapper<User>> Users;
        int userid = 0;

        void addUser(User user) {
            Users.push_back(&user);
            userid++;
        };
        int getUserIndex(int id) {
            auto it = find_if(Users.begin(), Users.end(), [&](User& user) {
                return user.Id == id;
            });

            if (it != Users.end()) {
                // Calculate the index using std::distance
                int index = distance(Users.begin(), it);
                return index;
                
            } else {
                return -1;
            };
        };
        void deleteUser(int id) {
            User& userToDelete = Users[getUserIndex(id)];

            if (userToDelete.userBook.borrowedBook) {
                userToDelete.userBook.borrowedBook.bookQueues.currentBorrower = nullopt;
            } else {
                find(userToDelete.userBook.borrowedBook.bookQueues.QueueOfUsers.begin(), userToDelete.userBook.borrowedBook.bookQueues.QueueOfUsers.end(), );
            };
        };
};

class Bookshelf {
    public:
        vector<Book> bookshelf;
        int bookId = 0;

        void printBookshelf() {
            for (int i = 0; i = bookshelf.size() - 1; i++) {
                auto& selectedBook = bookshelf[i];
                cout << "| " << i << " | " << selectedBook.Id << " | " << selectedBook.Title << 
                " | " << selectedBook.Author << " | " << selectedBook.Year << " | " << printBookStatus(selectedBook);
            };
        };
};

class User {
    public:
        int Id;
        string Name;
        userBooks userBook;
        // constructor
        User(UserList userListTether, string name) {
            Id = userListTether.userid++;
            Name = name;
        };
        void updateUserBooks(Book book) {
            if (!userBook.borrowedBook) {
                userBook.borrowedBook = book;
            } else {
                userBook.queuingBook = book;
            };
        };

        //jika ingin meminjam atau masuk antrean suatu buku
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

        // constructor
        Book() = default;
        Book(Bookshelf bookshelfTether, string isbn, string title, string author, int year) {
            Id = bookshelfTether.bookId;
            Isbn = isbn;
            Title = title;
            Author = author;
            Year = year;
        };

        void updateBookQueues(User user) {
            if (!bookQueue.currentBorrower) {
                bookQueue.currentBorrower = user;
            } else {
                bookQueue.QueueOfUsers.push_back(user);
            };
        };
};
struct userBooks {
    optional<Book> borrowedBook;
    optional<Book> queuingBook;
};
struct bookQueues {
    optional<User> currentBorrower;
    vector<reference_wrapper<User>> QueueOfUsers;
};


string printBookStatus(Book book) {
    if (book.Status == bookStatus::TERSEDIA) {
        return "Tersedia";
    } else {
        return "Dipinjam";
    };
}
