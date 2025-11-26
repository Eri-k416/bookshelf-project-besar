#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <memory>
#include <deque>
using namespace std;

vector<int> hari_dalam_bulan = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
// fungsi-fungsi
bool kabisat(int tahun) {
    return (tahun % 4 == 0 && tahun % 100 != 0) || (tahun % 400 == 0);
};
int kalkulasi_hari(string tanggal) {
    int tahun = stoi(tanggal.substr(4));
    int bulan = stoi(tanggal.substr(2, 2));
    int hari = stoi(tanggal.substr(0, 2));

    long long tabulasi_hari = 0;

    for (int y = 1; y < tahun; ++y) {
        tabulasi_hari += kabisat(y) ? 366 : 365;
    }

    for (int m = 1; m < bulan; ++m) {
        tabulasi_hari += hari_dalam_bulan[m];
        // tambahkan satu hari untuk Februari jika ini tahun kabisat
        if (m == 2 && kabisat(tahun)) {
            tabulasi_hari++;
        }
    }

    // tambahkan hari di bulan ini
    tabulasi_hari += hari;

    return tabulasi_hari;
}

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
        vector<unique_ptr<User>> Users;
        int userid = 0;

        void addUser(unique_ptr<User> user) {
            Users.push_back(user);
            userid++;
        };
        int getUserIndex(int id) {
            auto it = find_if(Users.begin(), Users.end(), [&](unique_ptr<User> user) {
                return user->Id == id;
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
            unique_ptr<User> userToDelete = move(Users[getUserIndex(id)]);
            Users.erase(Users.begin() + getUserIndex(id));

            if (userToDelete->userBook.borrowedBook) {
                userToDelete->userBook.borrowedBook->bookQueue.currentBorrower.release();
                userToDelete->userBook.borrowedBook.release();
            };
            if (userToDelete->userBook.queuingBook) {
                userToDelete->userBook.borrowedBook->bookQueue.QueueOfUsers[distance(userToDelete->userBook.borrowedBook->bookQueue.QueueOfUsers.begin(), find_if(userToDelete->userBook.borrowedBook->bookQueue.QueueOfUsers.begin(), userToDelete->userBook.borrowedBook->bookQueue.QueueOfUsers.end(), [&](unique_ptr<User> user){
                    return user->Id == id;
                }))].release();
            };
            userToDelete.reset();
            
            
        };
};

class Bookshelf {
    public:
        vector<unique_ptr<Book>> bookshelf;
        int bookId = 0;

        void addBook(unique_ptr<Book> book) {
            bookshelf.push_back(book);
            bookId++;

        };

};

class User {
    public:
        int Id;
        string Name;
        string returnDate; // DDMMYYYYY format
        userBooks userBook;
        // constructor
        User(UserList& userListTether, string name, string date) {
            Id = userListTether.userid++;
            Name = name;
            returnDate = date;
        };
        //jika ingin meminjam atau masuk antrean suatu buku
        void updateUserBooks(Book& book) {
            if (!userBook.borrowedBook) {
                userBook.borrowedBook = make_unique<Book>(book);
            } else if (!userBook.queuingBook) {
                userBook.queuingBook = make_unique<Book>(book);
            };
        };
        int countBookFine(string nowDate) {
            int fine = 1;
            int days = kalkulasi_hari(nowDate) - kalkulasi_hari(returnDate);

            if (days > 3) {
                fine *= 5000;
            };

            return fine;
        };

        ~User() {};
    
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
        Book(Bookshelf& bookshelfTether, string isbn, string title, string author, int year) {
            Id = bookshelfTether.bookId++;
            Isbn = isbn;
            Title = title;
            Author = author;
            Year = year;
        };

        void updateBookQueues(User user) {
            if (!bookQueue.currentBorrower) {
                bookQueue.currentBorrower = make_unique<User>(user);
            } else {
                bookQueue.QueueOfUsers.push_back(make_unique<User>(user));
            };
        };
        void emptyBookBorrower() {
            bookQueue.currentBorrower.release();
        };
        void queueAdvance() {
            bookQueue.currentBorrower = move(bookQueue.QueueOfUsers.front());
            bookQueue.QueueOfUsers.pop_front();
        };

        ~Book() {};
};
struct userBooks {
    unique_ptr<Book> borrowedBook;
    unique_ptr<Book>  queuingBook;
};
struct bookQueues {
    unique_ptr<User>  currentBorrower;
    deque<unique_ptr<User>> QueueOfUsers;
};


string printBookStatus(Book book) {
    if (book.Status == bookStatus::TERSEDIA) {
        return "Tersedia";
    } else {
        return "Dipinjam";
    };
}
