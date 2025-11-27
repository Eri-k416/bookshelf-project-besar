#ifndef USERLIST_HPP
#define USERLIST_HPP

#include "common.hpp"

class UserList {
public:
    vector<shared_ptr<User>> Users;
    int userid = 0;

    void addUser(shared_ptr<User> user);
    int getUserIndex(int id);
    void deleteUser(int id); // This implementation is complex and will go in main.cpp
};

#endif