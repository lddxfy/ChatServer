#ifndef GROUP_H
#define GROUP_H
#include <iostream>
#include <string>
#include <vector>
#include "groupuser.hpp"
using namespace std;

class Group
{
public:
    Group(int id = -1,string groupname = "",string groupdesc = "")
    {
        this->id = id;
        this->groupname = groupname;
        this->groupdesc = groupdesc;
    }
    string getgroupname(){ return this->groupname; }
    string getgroupdesc(){ return this->groupdesc; }
    vector<GroupUser> getUsers(){   return groupusers; }
    int getId(){ return this->id; }
    void setgroupname(string name){ this->groupname = name;}
    void setgroupdesc(string groupdesc){ this->groupdesc = groupdesc;}
    void setId(int id){this->id = id;}
private:
    int id;
    string groupname;
    string groupdesc;
    vector<GroupUser> groupusers;

};

#endif