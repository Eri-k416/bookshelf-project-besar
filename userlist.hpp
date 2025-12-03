#ifndef USERLIST_HPP
#define USERLIST_HPP

#include <memory>
#include "common.hpp"

class UserList {
public:
    vector<shared_ptr<User>> Users;
    int userid = 0;

    void addUser(string name, string borrowDate);
    int getUserIndex(int id);
    int getUserFromName(string str);
    void deleteUser(int id); 
};

#endif