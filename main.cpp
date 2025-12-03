#include "book.hpp"
#include "user.hpp"
#include "bookshelf.hpp"
#include "userlist.hpp"
#include <cstdlib>
#include <cctype>
#include <stdexcept>
#include <limits>
#include <thread> 
#include <chrono>
using namespace std;

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
};

bool isStringDigit(string str) {
    for (char& i : str) {
        if (!isdigit(i)) {
            return false;
        };
    };

    return true;
};

// user class
User::User(int& userIdTether, string name, string date) {
    Id = userIdTether++;
    Name = name;
    borrowDate = date;
}

void User::updateUserBooks(shared_ptr<Book>& book) {
    if (!userBook.borrowedBook) {
        userBook.borrowedBook = book;
    } else if (!userBook.queuingBook) {
        userBook.queuingBook = book;
    };
}

void User::queueAdvanceBook() {
    userBook.borrowedBook.swap(userBook.queuingBook);
};

int User::countBookFine(string nowDate) {
    int fine = 0;
    int days = kalkulasi_hari(nowDate) - kalkulasi_hari(borrowDate);

    if (days > 3) {
        fine = days*5000;
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
Book::Book(int& bookIdTether, string isbn, string title, string author, int year) {
    Id = bookIdTether++;
    Isbn = isbn;
    Title = title;
    Author = author;
    Year = year;
};
void Book::editBook(string isbn, string title, string author, int year) {
    Isbn = isbn;
    Title = title;
    Author = author;
    Year = year;
};
void Book::updateBookQueues(shared_ptr<User> user) {
    if (!bookQueue.currentBorrower) {
        bookQueue.currentBorrower = user;
    } else {
        bookQueue.QueueOfUsers.push_back(user);
    };
};
void Book::emptyBookBorrower() {
    bookQueue.currentBorrower.reset();
};
void Book::queueAdvance() {
    bookQueue.currentBorrower.swap(bookQueue.QueueOfUsers.front());
    bookQueue.QueueOfUsers.pop_front();
};
string Book::printBookStatus() {
    if (Status == bookStatus::TERSEDIA) {
        return "Tersedia";
    } else {
        return "Dipinjam";
    };
};

Book::~Book() {}

// userlist class
void UserList::addUser(string name, string returnDate) {
    Users.push_back(make_shared<User>(userid, name, returnDate));
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
};
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
    };
    
    Users.erase(Users.begin() + index);
};
int UserList::getUserFromName(string str) {
    auto it = find_if(Users.begin(), Users.end(), [&](shared_ptr<User> user) {
        return user->Name == str;
    });

    if (it != Users.end()) {
        int index = distance(Users.begin(), it);
        return index;
        
    } else {
        return -1;
    };
};

// bookshelf class
void Bookshelf::addBook(string isbn, string title, string author, int year) {
    bookshelf.push_back(make_shared<Book>(bookId, isbn, title, author, year));
}

int Bookshelf::getBookIndex(int id) {
    auto it = find_if(bookshelf.begin(), bookshelf.end(), [&](shared_ptr<Book> book) {
        return book->Id == id;
    });

    if (it != bookshelf.end()) {
        int index = distance(bookshelf.begin(), it);
        return index;
        
    } else {
        return -1;
    };
};
void Bookshelf::deleteBook(int id) {
    int index = getBookIndex(id);
    if (index == -1) {
        return;
    };
    shared_ptr<Book> bookToDelete = bookshelf[index];

    if (bookToDelete->bookQueue.currentBorrower) {
        bookToDelete->emptyBookBorrower();

        bookToDelete->bookQueue.currentBorrower->userBook.borrowedBook.reset(); 

        bookToDelete->emptyBookBorrower();
    };
    if (!bookToDelete->bookQueue.QueueOfUsers.empty()) {
        for (shared_ptr<User> user : bookToDelete->bookQueue.QueueOfUsers) {
            user->userBook.queuingBook.reset();
        };
        bookToDelete->bookQueue.QueueOfUsers.clear();
    };

    bookshelf.erase(bookshelf.begin() + index);
};

// terminal UI
string shutdownConfirm;
string adminPass = "admin123";
void header() {
    cout << "=========================================================================================================================================================\n";
    cout << "\t\t\t\t\t\t\t   ZARIMAN BOOKSHELF APP\n";
    cout << "\t\t\t\t\t\t 'library of alexandria got nothing on this'\n";
    cout << "\t\t\t\t\t\t\t -Erik, Coder of this program  \n\n";
    cout << "==========================================================================================================================================================\n";

};
void clearScreen() {
    #ifdef _WIN32
        system("cls"); // windows
    #else
        system("clear"); // linux/macOS
    #endif
};
string countBlankSpace(string& str) {
    string returnString;
    for (int i = 0; i < 57-(static_cast<int>(str.length())); i++) {
        returnString = returnString + " ";
    };

    return returnString;
};
template <typename T>
void getValidatedInput(T& variable, const string& prompt = "Enter your choice: ") {
    cout << prompt;
    
    while (!(cin >> variable)) {
        cout << "Invalid input. Please enter the correct format: ";
        cin.clear();
        cin.ignore(10000, '\n');
    }
}


void showBookDetails(shared_ptr<Book>& bookToShow) {
    cout << "ISBN buku: " << bookToShow->Isbn << endl;
    cout << "Judul buku: " << bookToShow->Title << endl;
    cout << "Penulis buku: " << bookToShow->Author << endl;
    cout << "Tahun Terbit buku: " << bookToShow->Year << endl;
    cout << "Status buku: " << bookToShow->printBookStatus() << endl;
    cout << "Antrean : (" << bookToShow->bookQueue.QueueOfUsers.size() << " )\n";
};
void showBook(Bookshelf& bookshelf, UserList& patrons, int idx) {
    try {
        shared_ptr<Book>& bookToShow = bookshelf.bookshelf[idx];
    } catch (out_of_range& e) {
        return;
    };
    
    shared_ptr<Book>& bookToShow = bookshelf.bookshelf[idx];
    int userChoice;
    string name = "";
    string date;
    int idxUser;
    int validateBorrowHandle;
    bool caughtException = false;
    
    while (true) {
        clearScreen();
        header();

        showBookDetails(bookToShow);

        cout << "Ketik pilihan aksi terhadap buku ini : \n" << endl;
        cout << "1. " <<((bookToShow->Status == bookStatus::TERSEDIA)? "Pinjam " : "Mengantri untuk ") << "Buku.\n";
        cout << "9. Kembali\n\n";

        getValidatedInput(userChoice, "Ketik pilihan : ");

        while (userChoice == 1) {
            clearScreen();
            header();
            cout << (bookToShow->Status == bookStatus::TERSEDIA)? "Buku tersedia, ingin meminjam?" : "Buku sedang dipinjan, ingin mengantri?";
            cout << "1. " << ((bookToShow->Status == bookStatus::TERSEDIA)? "Pinjam " : "Mengantri ") << endl;
            cout << "9. Kembali\n";
            getValidatedInput(validateBorrowHandle, "Ketik Pilihan :");

            if (validateBorrowHandle == 1) {
                cout << "Masukkan nama peminjam : \n";
                cin >> name;
                while (name.length() > 100 || name.length() < 3) {
                    cout << "Nama tidak boleh kurang dari 3 karakter atau lebih dari 100 karakter, coba lagi : \n";
                    cin.clear();
                    cin.ignore(10000, '\n');
                    getline(cin, name);
                };
                cout << "Masukkan tanggal peminjaman (format DDMMYYYY) : \n";
                cin >> date;
                while (true) {
                    if (date.length() != 8) {
                        cout << "Panjang input tidak valid (harus 8 digit: DDMMYYYY), coba lagi: \n";
                    } else {
                        try {
                            // kalo ada non-digit, error, tangkap
                            // kalo keluar kapasitas int, error, tangkap (out of range)
                            stoi(date);
                            break; 
                            
                        } catch (const invalid_argument& e) {
                            cout << "Input harus hanya digit (tidak mengandung karakter lain), coba lagi : \n";
                            caughtException = true;
                        } catch (const out_of_range& e) {
                            cout << "Input terlalu panjang, coba lagi: \n";
                            caughtException = true;
                        };
                    };

                    if (!caughtException) {
                        return;
                    };
                    
                    caughtException = false;

                    cin.clear();
                    // membaca input hanya setelah linebreak
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');

                    getline(cin, date);
                };
                
                
                if (patrons.getUserFromName(name) == -1) { //kalau tidak ketemu di database
                    patrons.addUser(name, date);
                    break;
                };
                idxUser = patrons.getUserFromName(name);

                shared_ptr<User>& borrowOrQueueUser = patrons.Users[idxUser];
                if (!(borrowOrQueueUser->userBook.borrowedBook && borrowOrQueueUser->userBook.queuingBook) && (borrowOrQueueUser->userBook.borrowedBook != bookToShow)) {
                    borrowOrQueueUser->updateUserBooks(bookToShow);

                    if (!bookToShow->bookQueue.currentBorrower) {
                        borrowOrQueueUser->borrowDate = date;
                        bookToShow->bookQueue.currentBorrower = borrowOrQueueUser;
                    } else {
                        bookToShow->bookQueue.QueueOfUsers.push_back(borrowOrQueueUser);
                    };

                    return;
                } else {
                    cout << "Gagal menambahkan user. Hal ini dikarenakan user sudah meminjam dan mengantri.\n\n";
                };
            } else if (validateBorrowHandle == 9) {
                break;
            } else {
                cout << "Pilihan tidak valid!.\n\n";
            };
        }; 
        if (userChoice == 9) {
            return;
        } else {
            cout << "Pilihan tidak valid\n";
        };
    };
};
void takeFromQueue(Bookshelf& bookshelf, UserList& patrons) {
    string username;
    string borrowdate;
    bool caughtException = false;
    
    clearScreen();
    header();
    cout << endl;

    cout << "Masukkan nama peminjam (q untuk quit): \n";
    cin >> username;
    if (username == "q") {
        return;
    };

    while (patrons.getUserFromName(username) == -1) {
        cout << "User tidak ada dalam data user, coba lagi : \n";
        cin.clear();
        cin.ignore(10000, '\n');
        getline(cin, username);
    };
    if (username == "q") {
        return;
    };

    cout << "Masukkan tanggal peminjaman (format DDMMYYYY) : \n";
    cin >> borrowdate;
    while (true) {
        if (borrowdate.length() != 8) {
            cout << "Panjang input tidak valid (harus 8 digit: DDMMYYYY), coba lagi: \n";
        } else {
            try {
                // kalo ada non-digit, error, tangkap
                // kalo keluar kapasitas int, error, tangkap (out of range)
                stoi(borrowdate);
                break; 
                
            } catch (const invalid_argument& e) {
                cout << "Input harus hanya digit (tidak mengandung karakter lain), coba lagi : \n";
                caughtException = true;
            } catch (const out_of_range& e) {
                cout << "Input terlalu panjang, coba lagi: \n";
                caughtException = true;
            };

            if (!caughtException) {
                break;
            };
        };

        caughtException = false;
        
        cin.clear();
        // membaca input hanya setelah linebreak
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        getline(cin, borrowdate);
    };

    shared_ptr<User>& queuingUser = patrons.Users[patrons.getUserFromName(username)];

    if (queuingUser->userBook.borrowedBook) {
        cout << "User ini sedang meminjam buku...\n";

        this_thread::sleep_for(chrono::seconds(3)); // pause for 3 secs

        return;
    } else if (!queuingUser->userBook.queuingBook) {
        cout << "User ini sedang tidak mengantri...";
        this_thread::sleep_for(chrono::seconds(3)); // pause for 3 secs

        return;
    } else {
        if (queuingUser == queuingUser->userBook.queuingBook->bookQueue.QueueOfUsers.front()) {

        }
    };




};
void borrowScreen(Bookshelf& bookshelf, UserList& patrons) {

    int page = 1;
    int bookLeftToShow = bookshelf.bookshelf.size();
    string userChoice;
    clearScreen();
    header(); 
    cout << endl << endl;

    while (true) {
        cout << "| NO | \t\t\t\t JUDUL \t\t\t\t |\t   AUTHOR   \t| TAHUN |  STATUS  | ANTREAN |\n";
        cout << "\n";
        for (int i = (page-1)*10; bookLeftToShow < 10? bookLeftToShow : 10; i++) {
            shared_ptr<Book>& currentBook = bookshelf.bookshelf[i];
            string noshow = to_string(i).length() == 1? "0" + to_string(i) : to_string(i);
            cout << "| " << noshow << " |";
            cout << "| " << ((currentBook->Title.length() > 54)? currentBook->Title.substr(0, 54) + "..." : currentBook->Title + countBlankSpace(currentBook->Title)) << " | ";
            cout << "| " << currentBook->Author << " |";
            cout << "| " << currentBook->Year << " |";
            cout << "| " << currentBook->printBookStatus() << " |";
            cout << "| " << currentBook->bookQueue.QueueOfUsers.size() << " |";
        };

        cout << endl << endl;
        cout << "Ketik no. buku untuk meminjam, atau ketik pilihan lainnya dibawah berikut : \n";
        cout << "a. Ambil buku dari antrean\n";
        cout << "b. Kembalikan buku\n";
        cout << "c. Kembali\n\n";
        
        getValidatedInput(userChoice, "Ketik Pilihan : ");

        if (isStringDigit(userChoice)) {
            showBook(bookshelf, patrons, stoi(userChoice));
            continue;
        } else if (userChoice == "a") {
            takeFromQueue(bookshelf, patrons);
        };

    };
    
};




void showBookAdmin(vector<shared_ptr<Book>>& bookshelf, int idx) {
    clearScreen();
    header();

    shared_ptr<Book>& bookToShow = bookshelf[idx];
    int userChoice;
    
    cout << "Id buku: " << bookToShow->Id << endl;
    cout << "ISBN buku: " << bookToShow->Isbn << endl;
    cout << "Judul buku: " << bookToShow->Title << endl;
    cout << "Penulis buku: " << bookToShow->Author << endl;
    cout << "Tahun Terbit buku: " << bookToShow->Year<< endl;
    cout << "Status buku: " << bookToShow->printBookStatus() << endl;
    cout << "Peminjam buku : " << bookToShow->bookQueue.currentBorrower? bookToShow->bookQueue.currentBorrower->Name : "Tidak ada";
    cout << "Antrean : (" << bookToShow->bookQueue.QueueOfUsers.size() << " )\n";
    if (!bookToShow->bookQueue.QueueOfUsers.empty()) {
        for (int i = 0; i < bookToShow->bookQueue.QueueOfUsers.size(); i++) {
            shared_ptr<User>& currentUserToShow = bookToShow->bookQueue.QueueOfUsers[i];
            cout << "\t " << (i+1) << ". " << currentUserToShow->Id << ": " << currentUserToShow->Name << endl;
        };
    };

    cout << "Ketik pilihan aksi terhadap buku ini : \n" << endl;
    cout << "1. Edit buku\n";
    cout << "2. Paksa majukan antrean\n";
    cout << "3. Paksa hapus peminjam\n";
    cout << "9. Kembali\n\n";
    while (true) {
        getValidatedInput(userChoice, "Ketik Pilihan : ");

        if (userChoice == 1) {
            string isbn;
            string title;
            string author;
            int year;

            cout << "ISBN buku: " << bookToShow->Isbn << endl;
            getValidatedInput(isbn, "Masukkan ISBN buku yang baru : ");
            cout << "Judul buku: " << bookToShow->Title << endl;
            getValidatedInput(title, "Masukkan Judul buku yang baru : ");
            cout << "Penulis buku: " << bookToShow->Author << endl;
            getValidatedInput(author, "Masukkan Penulis buku yang baru : ");
            cout << "Tahun Terbit buku: " << bookToShow->Year << endl;
            getValidatedInput(year, "Masukkan Tahun buku yang baru : ");

            bookToShow->editBook(isbn, title, author, year);

            break;
        } else if (userChoice == 2) {
            bookToShow->queueAdvance();

            break;
        } else if (userChoice == 3) {
            bookToShow->emptyBookBorrower();

            break;
        } else if (userChoice == 9) {
            return;
        } else {
            cout << "Pilihan tidak valid! Coba ulang!";   
        };
    };
};
void adminCreateBook(Bookshelf& bookshelfTether) {
    string isbn;
    string title;
    string author;
    int year;
    getValidatedInput(isbn, "Masukkan ISBN buku yang baru : ");
    getValidatedInput(title, "Masukkan Judul buku yang baru : ");
    getValidatedInput(author, "Masukkan Penulis buku yang baru : ");
    getValidatedInput(year, "Masukkan Tahun buku yang baru : ");

    bookshelfTether.addBook(isbn, title, author, year);

};
void adminBooks(Bookshelf& bookshelf) {
    int page = 1;
    int bookLeftToShow = bookshelf.bookshelf.size();
    string userChoice;

    clearScreen();
    header();

    while (true) {
        cout << "| NO | ID |      ISBN      | \t\t\t\t JUDUL \t\t\t\t |\t   AUTHOR   \t| TAHUN |  STATUS  | ANTREAN |\n";
        cout << endl;
        for (int i = (page-1)*10; bookLeftToShow < 10? bookLeftToShow : 10; i++) {
            shared_ptr<Book>& currentBook = bookshelf.bookshelf[i];
            string noshow = to_string(i).length() == 1? "0" + to_string(i) : to_string(i);
            string idshow = to_string(currentBook->Id).length() == 1? "0" + to_string(currentBook->Id) : to_string(currentBook->Id);
            cout << "| " << noshow << " |";
            cout << "| " << idshow << " |";
            cout << "| " << currentBook->Isbn << " |";
            cout << "| " << ((currentBook->Title.length() > 54)? currentBook->Title.substr(0, 54) + "..." : currentBook->Title + countBlankSpace(currentBook->Title)) << " | ";
            cout << "| " << currentBook->Author << " |";
            cout << "| " << currentBook->Year << " |";
            cout << "| " << currentBook->printBookStatus() << " |";
            cout << "| " << currentBook->bookQueue.QueueOfUsers.size() << " |";
        };

        cout << endl << endl;
        cout << "Ketik no. buku untuk melihat atau edit detail buku lebih lanjut, atau ketik pilihan lainnya dibawah berikut : \n";
        cout << "a. Tambah Buku\n";
        cout << "b. Hapus Buku\n";
        cout << "c. Kembali\n";

        if (page != bookLeftToShow) {
            cout << "x. Halaman berikutnya\n";
        };
        if (page != 1) {
            cout << "z. Halaman sebelumnya\n";
        };

        cin >> userChoice;
        if (typeid(userChoice) == typeid(int)) {
            showBookAdmin(bookshelf.bookshelf, stoi(userChoice));
        } else if (userChoice == "a") {
            adminCreateBook(bookshelf);
        } else if (userChoice == "b") {
            int bookToDeleteId;
            getValidatedInput(bookToDeleteId, "Hapus ID buku ke : ");
            bookshelf.deleteBook(bookToDeleteId);
        } else if (userChoice == "c") {
            return;
        } else if (userChoice == "x") {
            page++;
            bookLeftToShow -= 10;
        } else if (userChoice == "z") {
            page--;
            bookLeftToShow += 10;
        } else {
           cout << "Pilihan tidak valid"; 
        };
    };
};
void adminUsers(UserList& patrons) {
    int page = 1;
    int userLeftToShow = patrons.Users.size();
    cout << "| NO | ID | \t\t NAMA \t\t |   STATUS   |\t   PINJAM/ANTRI   \t|\n";
    for (int i = (page-1)*10; userLeftToShow < 10? userLeftToShow : 10; i++) {
        shared_ptr<User>& currentBook = patrons.Users[i];
    };
};
void adminPanel(Bookshelf& bookshelf, UserList& patrons) {
    string adminPassInput;
    int adminChoice;
    clearScreen();
    header();

    while (true) {
        cout << "Masukkan password admin (q untuk quit): \n";
        cin >> adminPassInput;

        if (adminPassInput == adminPass) {
            while (true) {
                cout << "Pilih menu admin: \n";
                cout << "1. Dashboard Buku\n";
                cout << "2. Dashboard User\n";
                cout << "3. Ganti password admin\n";
                cout << "4. Shutdown System (HATI-HATI)\n";
                cout << "9. Kembali\n\n";

                getValidatedInput(adminChoice, "Ketik Pilihan: ");

                if (adminChoice == 1) {
                    adminBooks(bookshelf);
                } else if (adminChoice == 2) {
                    adminUsers(patrons);
                } else if (adminChoice == 3) {
                    string adminPassConfirm;
                    while (adminPassConfirm != "q") {
                        clearScreen();
                        header();

                        getValidatedInput(adminPassConfirm, "Masukkan password admin lama (q untuk quit): ");

                        if (adminPassConfirm == adminPass) {
                            cout << "Masukkan password baru : ";
                            getValidatedInput(adminPass, "Ketik Pilihan: ");
                        } else {
                            cout << "Password salah, coba ulang! \n";
                        };
                    };

                } else if (adminChoice == 4) {
                    clearScreen();
                    header();

                    getValidatedInput(shutdownConfirm, "Apakah anda yakin untuk mematikan sistem?\nSeluruh data peminjam dan buku akan hilang, apakah anda yakin?\nEnter password admin untuk mematikan : ");

                    if (shutdownConfirm == adminPass) {
                        return;
                    };
                    clearScreen();
                } else if (adminChoice == 9) {
                    return;
                };
            };
        };
    };
};




// main finally GOD DAMN THAT TOOK SO FKIN LONG
int main() {
    cout << "god help me please\n\n";
    Bookshelf library;
    UserList patrons;
    
    int userChoice;
    
    // screen loop Pilihan tidak valid! Ketik ulang.

    while (shutdownConfirm != adminPass) {
        clearScreen();
        header();
        
        cout << "Pilih menu : \n";
        cout << "1. Dashboard Peminjaman\n";
        cout << "2. Admin Panel\n";
        while (true) {
            getValidatedInput(shutdownConfirm, "Ketik pilihan : ");
            if (userChoice == 1) {
                borrowScreen(library, patrons);
                break;
            } else if (userChoice == 2) {
                adminPanel(library, patrons);
                break;
            } else {
                cout << "\n";
            };
        };
    };

    return 0;
};