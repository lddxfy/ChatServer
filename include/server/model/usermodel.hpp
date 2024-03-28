#ifndef USERMODEL_H
#define USERMODEL_H
#include "user.hpp"
#include "mysqldb.hpp"

class UserModel
{
public:
    bool insert(User &user,shared_ptr<MySQL> mysql);

    User query(int id,shared_ptr<MySQL> mysql);

    bool updateState(User &user,shared_ptr<MySQL> mysql);

    void resetState(shared_ptr<MySQL> mysql);
private:

};

#endif