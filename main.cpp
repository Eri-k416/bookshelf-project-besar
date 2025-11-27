#include "book.hpp"
#include "user.hpp"
#include "bookshelf.hpp"
#include "userlist.hpp"
#include <cstdlib>

vector<int> hari_dalam_bulan = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

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
        if (m == 2 && kabisat(tahun)) {
            tabulasi_hari++;
        }
    }

    tabulasi_hari += hari;
    return tabulasi_hari;
}

// user class
User::User(UserList& userListTether, string name, string date) {
    Id = userListTether.userid++;
    Name = name;
    returnDate = date;
}

void User::updateUserBooks(shared_ptr<Book> book) {
    if (!userBook.borrowedBook) {
        userBook.borrowedBook = book;
    } else if (!userBook.queuingBook) {
        userBook.queuingBook = book;
    };
}

int User::countBookFine(string nowDate) {
    int fine = 1;
    int days = kalkulasi_hari(nowDate) - kalkulasi_hari(returnDate);

    if (days > 3) {
        fine *= 5000;
    };
    return fine;
}

string User::printUserStatus() {
    if (userBook.borrowedBook) {
        return "Meminjam";
    };
    if (userBook.queuingBook) {
        return "Mengantri";
    }
    return "Idle";
}

User::~User() {}

// book class
Book::Book(Bookshelf& bookshelfTether, string isbn, string title, string author, int year) {
    Id = bookshelfTether.bookId++;
    Isbn = isbn;
    Title = title;
    Author = author;
    Year = year;
}

void Book::updateBookQueues(shared_ptr<User> user) {
    if (!bookQueue.currentBorrower) {
        bookQueue.currentBorrower = user;
    } else {
        bookQueue.QueueOfUsers.push_back(user);
    };
}

void Book::emptyBookBorrower() {
    bookQueue.currentBorrower.reset();
}

void Book::queueAdvance() {
    bookQueue.currentBorrower.swap(bookQueue.QueueOfUsers.front());
    bookQueue.QueueOfUsers.pop_front();
}

string Book::printBookStatus() {
    if (Status == bookStatus::TERSEDIA) {
        return "Tersedia";
    } else {
        return "Dipinjam";
    };
}

Book::~Book() {}

// userlist class
void UserList::addUser(shared_ptr<User> user) {
    Users.push_back(user);
}

int UserList::getUserIndex(int id) {
    auto it = find_if(Users.begin(), Users.end(), [&](shared_ptr<User> user) {
        return user->Id == id;
    });

    if (it != Users.end()) {
        int index = distance(Users.begin(), it);
        return index;
        
    } else {
        return -1;
    };
}

void UserList::deleteUser(int id) {
    int index = getUserIndex(id);
    if (index == -1) {
        return;
    }
    shared_ptr<User> userToDelete = Users[index];

    if (userToDelete->userBook.borrowedBook) {
        userToDelete->userBook.borrowedBook->emptyBookBorrower();
        userToDelete->userBook.borrowedBook.reset(); 
    };

    if (userToDelete->userBook.queuingBook) {
        auto& queue = userToDelete->userBook.queuingBook->bookQueue.QueueOfUsers;

        auto it = find_if(queue.begin(), queue.end(), [&](shared_ptr<User> user) {
            return user->Id == id;
        });

        if (it != queue.end()) {
            queue.erase(it);
        };
        userToDelete->userBook.queuingBook.reset();
    }
    
    Users.erase(Users.begin() + index);
}

// bookshelf class
void Bookshelf::addBook(shared_ptr<Book> book) {
    bookshelf.push_back(book);
}

int Bookshelf::getBookIndex(int id) {
    auto it = find_if(bookshelf.begin(), bookshelf.end(), [&](shared_ptr<Book> book) {
        return book->Id == id;
    });

    if (it != bookshelf.end()) {
        // Calculate the index using std::distance
        int index = distance(bookshelf.begin(), it);
        return index;
        
    } else {
        return -1;
    };
};

// terminal UI
string adminPass = "admin123";
void header() {
    cout << "=====================================================================================================================\n";
    cout << "\t\t\t\t\t   ZARIMAN BOOKSHELF APP\n";
    cout << "\t\t\t\t 'library of alexandria got nothing on this'\n";
    cout << "\t\t\t\t\t -Erik, Coder of this program  \n\n";
    cout << "=====================================================================================================================\n";

};
void clearScreen() {
    #ifdef _WIN32
        system("cls"); // windows
    #else
        system("clear"); // linux/macOS
    #endif
};
void borrowScreen() {
    string statusPrint;
    header();
    cout << endl << endl;
    
    cout << "| NO | \t\t\t\t JUDUL \t\t\t\t |\t   AUTHOR   \t| TAHUN |  STATUS  | ANTREAN |\n";
    cout << "\t\t\t\t\t\t\t\t\t\t\n";
};
void adminBook(vector<shared_ptr<Book>>& bookshelf) {
    clearScreen();
    header();

    while (true) {
        cout << "| NO | \t\t\t\t JUDUL \t\t\t\t |\t   AUTHOR   \t| TAHUN |  STATUS  | ANTREAN |\n";
        for (int i = 0; i < bookshelf.size() < 10? bookshelf.size() : 10; i++) {
            string noshow = to_string(i).length() == 1? "0" + to_string(i) : to_string(i);
            string idshow = to_string(bookshelf[i]->Id).length() == 1? "0" + to_string(bookshelf[i]->Id) : to_string(bookshelf[i]->Id);
        cout << "| " << noshow << " |";
        cout << "| " << idshow << " |";
        cout << "| " << idshow << " |";
        };
    }
};
void adminPanel() {
    string adminPassInput;
    clearScreen();
    header();

    while (true) {
        cout << "Masukkan password admin (q untuk quit): \n";
        cin >> adminPassInput;

        if (adminPassInput != adminPass) {
            cout << "Coba ulang";
        } else {

        };
    };
};

// main finally GOD DAMN THAT TOOK SO FKIN LONG
int main() {
    cout << "god help me please\n\n";
    Bookshelf library;
    UserList patrons;
    
    int userChoice;
    
    string shutdownConfirm;
    
    // screen loop

    while (true) {
        header();
        
        cout << "Pilih menu : \n";
        cout << "1. Dashboard Peminjaman\n";
        cout << "2. Admin Panel\n";
        cout << "3. Shutdown System (HATI-HATI)\n\n";
        while (true) {
            cout << "Ketik pilihan : ";
            cin >> userChoice;

            if (userChoice == 1) {
                borrowScreen();
                break;
            } else if (userChoice == 2) {
                break;
            } else if (userChoice == 3) {
                break;
            } else {
                cout << "Pilihan tidak valid! Ketik ulang.\n";
            };
        };
        if (userChoice == 3) {
            clearScreen();
            header();
            cout << "Apakah anda yakin untuk mematikan sistem?\nSeluruh data peminjam dan buku akan hilang, apakah anda yakin?\nEnter password admin untuk mematikan : ";
            cin >> shutdownConfirm;

            if (shutdownConfirm == adminPass) {
                break;
            };
            clearScreen();
        };
        
    };
    return 0;
};