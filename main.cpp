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
};

void User::updateUserBooks(shared_ptr<Book>& book) {
    if (!userBook.borrowedBook) {
        userBook.borrowedBook = book;
    } else if (!userBook.queuingBook) {
        userBook.queuingBook = book;
    };
};

void User::queueAdvanceBook() {
    userBook.borrowedBook.swap(userBook.queuingBook);
};

void User::emptyBorrowedBook() {
    userBook.borrowedBook.reset();
    borrowDate = "";
};

int User::countBookFine(string nowDate) {
    int fine = 0;
    int days = kalkulasi_hari(nowDate) - kalkulasi_hari(borrowDate);

    if (days > 3) {
        fine = (days-3)*5000;
    };
    return fine;
};

string User::printUserStatus() {
    if (Status == userStatus::MEMINJAM) {
        return "Meminjam";
    };
    if (Status == userStatus::MENGANTRI) {
        return "Mengantri";
    };
    return "Idle";
};

User::~User() {}

// book class
Book::Book(int& bookIdTether, string isbn, string title, string author, int year, string createdate) {
    Id = bookIdTether++;
    Isbn = isbn;
    Title = title;
    Author = author;
    Year = year;
    createDate = createdate;
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
int Book::findQueuingUserIndex(shared_ptr<User>& queueUser) {
    deque<shared_ptr<User>>& queueArray = bookQueue.QueueOfUsers;
    auto it = find_if(queueArray.begin(), queueArray.end(), [&](shared_ptr<User>& user) {
        return user->Id == queueUser->Id;
    });

    if (it != queueArray.end()) {
        int index = distance(queueArray.begin(), it);
        return index;
        
    } else {
        return -1;
    };
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
void Bookshelf::addBook(string isbn, string title, string author, int year, string createDate) {
    bookshelf.push_back(make_shared<Book>(bookId, isbn, title, author, year, createDate));
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
    };

    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
};

void getValidatedString(string& variable, const string& prompt = "Enter input: ") {
    cout << prompt;
    
    // Check if the previous operation left a newline in the buffer
    if (cin.peek() == '\n') {
        // If so, consume the newline before calling getline
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    };
    
    // Read the entire line, including spaces
    getline(cin, variable);
    
    // Simple loop to handle empty/whitespace-only input if needed
    while (variable.empty()) {
        cout << "Input cannot be empty. Please try again: ";
        getline(std::cin, variable);
    };
}

string getValidDate() {
    string date;

    cout << "Masukkan tanggal peminjaman (format DDMMYYYY) : \n";
    cin >> date;
    while (true) {
        if (date.length() != 8) {
            cout << "Panjang input tidak valid (harus 8 digit: DDMMYYYY), coba lagi: \n";
        } else {
            try {
                stoi(date);
                break;
            } catch (const invalid_argument& e) {
                cout << "Input harus hanya digit (tidak mengandung karakter lain), coba lagi : \n";
            } catch (const out_of_range& e) {
                cout << "Input terlalu panjang, coba lagi: \n";
            }
        };

         // Read new input for the next iteration
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        getline(cin, date); // Use getline for robustness
    };

    return date;
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

    // catch user inputting something not in the bookshelf list
    try {
        shared_ptr<Book>& bookToShow = bookshelf.bookshelf[idx];
        bookToShow->Id = bookToShow->Id;
    } catch (out_of_range& e) {
        return;
    };
    
    shared_ptr<Book>& bookToShow = bookshelf.bookshelf[idx];
    int userChoice;
    string name = "";
    string date;
    int idxUser;
    int validateBorrowHandle;
    
    // showBook loop
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
            cout << ((bookToShow->Status == bookStatus::TERSEDIA)? "Buku tersedia, ingin meminjam?" : "Buku sedang dipinjan, ingin mengantri?") << endl;
            cout << "1. " << ((bookToShow->Status == bookStatus::TERSEDIA)? "Pinjam " : "Mengantri ") << endl;
            cout << "9. Kembali\n";
            getValidatedInput(validateBorrowHandle, "Ketik Pilihan :");

            if (validateBorrowHandle == 1) {
                getValidatedString(name, "Masukkan nama peminjam : ");
                
                while (name.length() > 100 || name.length() < 3) {
                    cout << "Nama tidak boleh kurang dari 3 karakter atau lebih dari 100 karakter, coba lagi : \n";
                    cin.clear();
                    cin.ignore(10000, '\n');
                    getline(cin, name);
                };
                date = getValidDate();

                // LOGIC
                idxUser = patrons.getUserFromName(name);

                if (idxUser == -1) {
                    patrons.addUser(name, date);
                    idxUser = patrons.getUserFromName(name);
                };

                // get pointer
                shared_ptr<User>& borrowOrQueueUser = patrons.Users[idxUser]; 

                // 3. Check borrow/queue constraints
                if (!(borrowOrQueueUser->userBook.borrowedBook || borrowOrQueueUser->userBook.queuingBook)) {
                    
                    borrowOrQueueUser->updateUserBooks(bookToShow);

                    if (!bookToShow->bookQueue.currentBorrower) {
                        borrowOrQueueUser->borrowDate = date;
                        borrowOrQueueUser->Status = userStatus::MEMINJAM;
                        bookToShow->Status = bookStatus::DIPINJAM;

                    } else {
                        borrowOrQueueUser->Status = userStatus::MENGANTRI;
                    };

                    // 4. Pass the actual shared_ptr to the book
                    bookToShow->updateBookQueues(borrowOrQueueUser); 

                    cout << "Berhasil memproses peminjaman/antrian untuk " << name << endl;
                    this_thread::sleep_for(chrono::seconds(3));
                    return;
                } else {
                    cout << "Gagal menambahkan user. User sudah meminjam atau mengantri buku lain.\n\n";
                    this_thread::sleep_for(chrono::seconds(3));
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
            cout << "Pilihan tidak valid!.\n\n";

            this_thread::sleep_for(chrono::seconds(3));
        };
    };
};
void takeFromQueue(UserList& patrons) {
    string username;
    string borrowDate;
    
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

    borrowDate = getValidDate();

    shared_ptr<User>& queuingUser = patrons.Users[patrons.getUserFromName(username)];

    if (queuingUser->userBook.borrowedBook) {
        cout << "User ini sedang meminjam buku... Mohon dikembalikan buku yang sedang dipinjam terlebih dahulu.\n\n";

        this_thread::sleep_for(chrono::seconds(3)); // pause for 3 secs

        return;
    } else if (!queuingUser->userBook.queuingBook) {
        cout << "User ini sedang tidak mengantri...\n\n";

        this_thread::sleep_for(chrono::seconds(3));

        return;
    } else {
        // logic
        if (queuingUser == queuingUser->userBook.queuingBook->bookQueue.QueueOfUsers.front()) {
            // jika user ada di barisan paling depan dan mau meminjam buku yang sudah diantrekan
            queuingUser->queueAdvanceBook();
            queuingUser->userBook.borrowedBook->queueAdvance();
            queuingUser->borrowDate = borrowDate;
            queuingUser->Status = userStatus::MEMINJAM;

            cout << "Berhasil meminjam buku!\n\n";
        } else {
            cout << "Tidak dapat mengambil buku, anda ada di antrian : " << (queuingUser->userBook.queuingBook->findQueuingUserIndex(queuingUser) + 1) << endl;
            
        };

        this_thread::sleep_for(chrono::seconds(3));
    };
};
void returnBook(UserList& patrons) {
    string username;
    string returnDate;
    int denda;
    
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
    shared_ptr<User>& borrowingUser = patrons.Users[patrons.getUserFromName(username)];

    returnDate = getValidDate();

    while (kalkulasi_hari(returnDate) < kalkulasi_hari(borrowingUser->borrowDate)) {
        cout << "Tanggal pengembalian tidak valid! Harus tanggal setelah tanggal peminjaman.\n";
        returnDate = getValidDate();
    };

    // logic
    borrowingUser->userBook.borrowedBook->Status = bookStatus::TERSEDIA;
    borrowingUser->userBook.borrowedBook->emptyBookBorrower();
    borrowingUser->emptyBorrowedBook();

    if (borrowingUser->userBook.queuingBook) {
        borrowingUser->Status = userStatus::MENGANTRI;
    } else {
        borrowingUser->Status = userStatus::PENDING;
    };

    cout << "Berhasil mengembalikan buku!\n";
    denda = borrowingUser->countBookFine(returnDate);
    cout << (denda < 5000? "Anda tidak dikenakan biaya untuk peminjaman ini." : "Anda telat " + to_string(denda/5000) + " hari. Anda dikenakan denda : Rp." + to_string(denda)) << endl;
};

void borrowScreen(Bookshelf& bookshelf, UserList& patrons) {

    int page = 1;
    string userChoice;
    
    cout << endl << endl;

    while (true) {
        clearScreen();
        header(); 
        cout << "| NO | \t\t\t\t JUDUL \t\t\t\t |\t   AUTHOR   \t| TAHUN |  STATUS  | ANTREAN |\n";
        cout << "\n";
        int startIndex = (page - 1) * 10;
        int endIndex = min(startIndex + 10, (int)bookshelf.bookshelf.size());
        for (int i = startIndex; i < endIndex; i++) {
            shared_ptr<Book>& currentBook = bookshelf.bookshelf[i];
            string noshow = to_string(i).length() == 1? "0" + to_string(i) : to_string(i);
            cout << "| " << noshow;
            cout << " | " << ((currentBook->Title.length() > 54)? currentBook->Title.substr(0, 54) + "..." : currentBook->Title + countBlankSpace(currentBook->Title));
            cout << " | " << currentBook->Author;
            cout << " | " << currentBook->Year;
            cout << " | " << currentBook->printBookStatus();
            cout << " | " << currentBook->bookQueue.QueueOfUsers.size() << " |" << endl;
        };

        cout << endl << endl;
        cout << "Ketik no. buku untuk meminjam, atau ketik pilihan lainnya dibawah berikut : \n";
        cout << "a. Ambil buku dari antrean\n";
        cout << "b. Kembalikan buku\n";
        cout << "c. Kembali\n\n";

        if (page*10 < (int)bookshelf.bookshelf.size()) {
            cout << "x. Halaman berikutnya\n";
        };
        if (page != 1) {
            cout << "z. Halaman sebelumnya\n";
        };
        
        getValidatedInput(userChoice, "Ketik Pilihan : ");

        if (isStringDigit(userChoice)) {
            showBook(bookshelf, patrons, stoi(userChoice));
            continue;
        } else if (userChoice == "a") {
            takeFromQueue(patrons);
        } else if (userChoice == "b") {
            returnBook(patrons);
        } else if (userChoice == "c") {
            return;
        } else if (userChoice == "x") {
            if (page*10 < (int)bookshelf.bookshelf.size()) {
                page++;
            };
        } else if (userChoice == "z") {
            if (page != 1) {
                page--;
            };
        }
        else {
            cout << "Pilihan tidak valid! \n";

            this_thread::sleep_for(chrono::seconds(3));
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
    cout << "Buku terdaftar pada tanggal: " << bookToShow->createDate << endl;
    cout << "Buku terakhir di edit pada tanggal : " << (bookToShow->lastEditDate.empty()? "Belum pernah di edit" : bookToShow->lastEditDate) << "\n\n";
    cout << "Status buku: " << bookToShow->printBookStatus() << endl;
    cout << "Peminjam buku : " << ((bookToShow->bookQueue.currentBorrower)? bookToShow->bookQueue.currentBorrower->Name : "Tidak ada") << endl;
    cout << "Antrean : (" << bookToShow->bookQueue.QueueOfUsers.size() << " )\n";
    if (!(bookToShow->bookQueue.QueueOfUsers.empty())) {
        for (long long unsigned int i = 0; i < bookToShow->bookQueue.QueueOfUsers.size(); i++) {
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
            getValidatedString(title, "Masukkan Judul buku yang baru : ");
            cout << "Penulis buku: " << bookToShow->Author << endl;
            getValidatedInput(author, "Masukkan Penulis buku yang baru : ");
            cout << "Tahun Terbit buku: " << bookToShow->Year << endl;
            getValidatedInput(year, "Masukkan Tahun buku yang baru : ");

            bookToShow->editBook(isbn, title, author, year);

            break;
        } else if (userChoice == 2) {
            if (!(bookToShow->bookQueue.QueueOfUsers.empty())) {
                bookToShow->queueAdvance();
                bookToShow->bookQueue.currentBorrower->queueAdvanceBook();
            };
            break;
        } else if (userChoice == 3) {
            if (bookToShow->bookQueue.currentBorrower) {
                bookToShow->bookQueue.currentBorrower->emptyBorrowedBook();
                bookToShow->emptyBookBorrower();
            };
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
    string createDate;
    getValidatedInput(isbn, "Masukkan ISBN buku yang baru : ");
    getValidatedString(title, "Masukkan Judul buku yang baru : ");
    getValidatedString(author, "Masukkan Penulis buku yang baru : ");
    getValidatedInput(year, "Masukkan Tahun buku yang baru : ");
    getValidatedInput(createDate, "Masukkan tanggal buku ini masuk ke perpustakaan ini (format DDMMYYYY) : ");

    bookshelfTether.addBook(isbn, title, author, year, createDate);

};
void adminBooks(Bookshelf& bookshelf) {
    int page = 1;
    string userChoice;

    while (true) {
        clearScreen();
        header();
        cout << "| NO | ID |      ISBN      | \t\t\t\t JUDUL \t\t\t\t |\t   AUTHOR   \t| TAHUN |  STATUS  | ANTREAN |\n";
        cout << endl;
        int startIndex = (page - 1) * 10;
        int endIndex = min(startIndex + 10, (int)bookshelf.bookshelf.size());
        for (int i = startIndex; i < endIndex; i++) {
            shared_ptr<Book>& currentBook = bookshelf.bookshelf[i];
            string noshow = to_string(i+1).length() == 1? "0" + to_string(i+1) : to_string(i+1);
            string idshow = to_string(currentBook->Id).length() == 1? "0" + to_string(currentBook->Id) : to_string(currentBook->Id);
            cout << "| " << noshow;
            cout << " | " << idshow ;
            cout << " | " << currentBook->Isbn;
            cout << " | " << ((currentBook->Title.length() > 54)? currentBook->Title.substr(0, 54) + "..." : currentBook->Title + countBlankSpace(currentBook->Title));
            cout << " | " << currentBook->Author;
            cout << " | " << currentBook->Year;
            cout << " | " << currentBook->printBookStatus();
            cout << " | " << currentBook->bookQueue.QueueOfUsers.size() << " |" << endl;
        };

        cout << endl << endl;
        cout << "Ketik no. buku untuk melihat atau edit detail buku lebih lanjut, atau ketik pilihan lainnya dibawah berikut : \n";
        cout << "a. Tambah Buku\n";
        cout << "b. Hapus Buku\n";
        cout << "c. Kembali\n";

        if (page*10 < (int)bookshelf.bookshelf.size()) {
            cout << "x. Halaman berikutnya\n";
        };
        if (page != 1) {
            cout << "z. Halaman sebelumnya\n";
        };

        getValidatedInput(userChoice, "Ketik Pilihan: "); 
        if (isStringDigit(userChoice)) {
            showBookAdmin(bookshelf.bookshelf, stoi(userChoice)-1);
        } else if (userChoice == "a") {
            adminCreateBook(bookshelf);
        } else if (userChoice == "b") {
            int bookToDeleteId;
            getValidatedInput(bookToDeleteId, "Hapus ID buku ke : ");
            bookshelf.deleteBook(bookToDeleteId);
        } else if (userChoice == "c") {
            return;
        } else if (userChoice == "x") {
            if (page*10 < (int)bookshelf.bookshelf.size()) {
                page++;

            };
        } else if (userChoice == "z") {
            if (page != 1) {
                page--;
            };
        } else {
           cout << "Pilihan tidak valid"; 

           this_thread::sleep_for(chrono::seconds(3));
        };
    };
};
void adminUsers(UserList& patrons) {
    int page = 1;
    int userChoice;
    string userToRemoveIdInput;
    string removeAllPendingUsersConfirm;

    while (true) {
        clearScreen();
        header();
        cout << endl;
        int startIndex = (page - 1) * 10;
        int endIndex = min(startIndex + 10, (int)patrons.Users.size());
        cout << "| NO | ID | \t\t NAMA \t\t |   STATUS   | TANGGAL PEMINJAMAN |\t   PINJAM/ANTRI   \t|\n";
        for (int i = startIndex; i < endIndex; i++) {
            shared_ptr<User>& currentUser = patrons.Users[i];
            string noshow = to_string(i).length() == 1? "0" + to_string(i+1) : to_string(i+1);
            string idshow = to_string(currentUser->Id).length() == 1? "0" + to_string(currentUser->Id) : to_string(currentUser->Id);

            cout << "| " << noshow;
            cout << " | " << idshow;
            cout << " | " << currentUser->Name;
            cout << " | " << currentUser->printUserStatus();
            cout << " |       " << currentUser->borrowDate;

            string bookTitleDisplay = "Tidak ada buku";

            if (currentUser->userBook.borrowedBook) {
                bookTitleDisplay = currentUser->userBook.borrowedBook->Title;
            } else if (currentUser->userBook.queuingBook) {
                bookTitleDisplay = currentUser->userBook.queuingBook->Title;
            }

            cout << "         | " << ((bookTitleDisplay.length() > 54) ? 
                    bookTitleDisplay.substr(0, 54) + "..." : 
                    bookTitleDisplay + countBlankSpace(bookTitleDisplay)) << " | " << endl;
        };

        cout << endl << endl;
        cout << "Ketik untuk memilih menu di bawah ini. \n";
        cout << "1. Hapus User \n";
        cout << "2. Hapus semua user yang sedang pending \n";
        cout << "3. Kembali\n\n";

        if (page*10 < (int)patrons.Users.size()) {
            cout << "4. Halaman berikutnya\n";
        };
        if (page != 1) {
            cout << "5. Halaman sebelumnya\n";
        };
        
        getValidatedInput(userChoice, "Ketik Pilihan : ");

        if (userChoice == 1) {
            getValidatedInput(userToRemoveIdInput, "Masukkan ID user (q untuk quit): ");
            if (isStringDigit(userToRemoveIdInput)) {
                int userToRemoveId = stoi(userToRemoveIdInput);
                int index = patrons.getUserIndex(userToRemoveId); // Get index once
                
                if (index == -1) { // Correct check for user NOT found
                    cout << "User tidak ditemukan! \n";
                    this_thread::sleep_for(chrono::seconds(3));
                } else {
                    int userToRemoveId = stoi(userToRemoveIdInput);
                    int index = patrons.getUserIndex(userToRemoveId); // Get the index once
                    
                    if (index == -1) { // Check if the index is NOT found
                        cout << "User tidak ditemukan! \n";
                        this_thread::sleep_for(chrono::seconds(3));
                    } else {
                        auto& userToRemove = patrons.Users[index];
                        if (userToRemove->userBook.borrowedBook) {
                            userToRemove->userBook.borrowedBook->emptyBookBorrower();
                            userToRemove->emptyBorrowedBook();
                        };
                        if (userToRemove->userBook.queuingBook) {
                            userToRemove->userBook.queuingBook->bookQueue.QueueOfUsers.erase(userToRemove->userBook.queuingBook->bookQueue.QueueOfUsers.begin() + userToRemove->userBook.queuingBook->findQueuingUserIndex(userToRemove));
                            userToRemove->userBook.queuingBook.reset();
                        };
                        
                        patrons.deleteUser(userToRemoveId); 
                        
                        cout << "User berhasil di hapus!\n";
                        this_thread::sleep_for(chrono::seconds(3));
                    };
                };
            } else {
                cout << "Input tidak valid! \n";\

                this_thread::sleep_for(chrono::seconds(3));
            }; 
            
        } else if (userChoice == 2) {
            cout << "Apakah anda yakin untuk menghapus semua user yang sedang pending (Y/N) ?";
            while (true) {
                cout << "Ketik Pilihan : ";
                cin >>  removeAllPendingUsersConfirm;
                if (removeAllPendingUsersConfirm == "Y") {
                    while (true) {
                        auto userToRemove = find_if(patrons.Users.begin(), patrons.Users.end(), [&](shared_ptr<User> user) {
                            return user->Status == userStatus::PENDING;
                        });

                        if (userToRemove != patrons.Users.end()) {
                            int index = distance(patrons.Users.begin(), userToRemove);
                            auto& user = patrons.Users[index];
                            cout << "Menghapus user id : " << user->Id << endl;
                            if (user->userBook.borrowedBook) {
                                user->userBook.borrowedBook->emptyBookBorrower();
                                user->emptyBorrowedBook();
                            };
                            if (user->userBook.queuingBook) {
                                user->userBook.queuingBook->bookQueue.QueueOfUsers.erase(user->userBook.queuingBook->bookQueue.QueueOfUsers.begin() + user->userBook.queuingBook->findQueuingUserIndex(user));
                                user->userBook.queuingBook.reset();
                            };

                            patrons.deleteUser(user->Id);
                        } else {
                          break;
                        };
                    };
                } else if (removeAllPendingUsersConfirm == "N") {
                    break;
                } else {
                    cout << "Pilihan tidak valid! \n";

                    this_thread::sleep_for(chrono::seconds(3));
                };
            };
        }else if (userChoice == 3) {
            return;
        } else if (userChoice == 4) {
            if (page*10 < (int)patrons.Users.size()) {
                page++;
            };
        } else if (userChoice == 5) {
            if (page != 1) {
                page--;
            };
        } else {
            cout << "Pilihan tidak valid! \n";

            this_thread::sleep_for(chrono::seconds(3));
        };
    };

};
void adminPanel(Bookshelf& bookshelf, UserList& patrons) {
    string adminPassInput;
    int adminChoice;
    clearScreen();
    header();

    while (true) {
        getValidatedString(adminPassInput, "Masukkan password admin: ");

        if (adminPassInput == adminPass) {
            while (true) {
                clearScreen();
                header();
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

                        if (adminPassConfirm == "q") {
                            break;
                        } else if (adminPassConfirm == adminPass) {
                            getValidatedInput(adminPass, "Masukkan password baru: ");
                            cout << "\nPassword berhasil diganti!\n";
                            std::this_thread::sleep_for(std::chrono::seconds(2));
                            break;
                        } else {
                            cout << "Password salah, coba ulang! \n";
                            std::this_thread::sleep_for(std::chrono::seconds(2));
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
            getValidatedInput(userChoice, "Ketik pilihan : ");
            if (userChoice == 1) {
                borrowScreen(library, patrons);
                break;
            } else if (userChoice == 2) {
                adminPanel(library, patrons);
                break;
            } else {
                cout << "Pilihan tidak valid! Ketik ulang. \n\n";
            };
        };
    };

    return 0;
};