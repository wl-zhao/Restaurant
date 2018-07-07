#ifndef USER_H
#define USER_H

#include "SignWidget.h"
#include "database.h"

//不同类别的用户名称和下标的对应关系
static const QString Identity[5] = {"顾客", "管理员", "厨师", "服务员", "经理"};

/*
 * 用户类
 * 包含用户的基本信息
 * 用于管理员和经理的信息管理
 * 服务员、顾客和厨师类由用户类派生
 */
class User
{
public:
    User(int id = 0, QString phonenumber = "", QString password = "", int type = 0):\
        m_id(id), m_phonenumber(phonenumber), m_password(password), m_type(type)
    {

    }
    virtual ~User(){}

    const int& id() const
    { return m_id; }
    const int& type() const
    { return m_type; }
    const QString& phonenumber() const
    { return m_phonenumber;}
    const QString& password() const
    { return m_password; }

    void setType(int i)
    { m_type = i; }
    void  setPhonenumber(QString phonenumber)
    { m_phonenumber = phonenumber; }
    void  setPasssword(QString password)
    { m_password = password; }

    virtual void insert();                  //虚函数, 存入用户信息, 在顾客、服务员、厨师类里被重写
    bool operator==(User &user);
    static User* getPointer(const User& user);     //获得用户对应的指针, 便于多态的实现

protected:
    int m_id;
    QString m_phonenumber;
    QString m_password;
    int m_type;
};

/*
 * 顾客类
 * 继承自用户类
 * 新增m_tableid表示顾客选择的桌号(默认值为0)
 */
class Customer : public User
{
public:
    Customer(int id = 0, QString phonenumber = "", QString password = "", int tableid = 0):\
        User(id, phonenumber, password, 0), m_tableid(tableid)
    {
        m_type = 0;
    }
    Customer(const User&user, int tableid = 0):User(user), m_tableid(tableid)
    {
        m_type = 0;
    }
    ~Customer(){}

    const int& tableid() const
    { return m_tableid; }
    void setTableId(int id)
    { m_tableid = id; }

    void saveTableId();         //保存桌号
    void insert();              //存入顾客信息
private:
    int m_tableid;
};

/*
 * 厨师类
 * 继承自用户类
 * 新增数据成员来保存厨师的工作数据
 */
class Cook : public User
{
public:
    Cook(int id = 0, QString phonenumber = "", QString password = "", \
         int taskCount = 0, int scoreCount = 0, int timeCount = 0);
    Cook(const User&user, int taskCount = 0, int scoreCount = 0, int timeCount = 0);
    ~Cook(){}

    const int& taskCount() const
    { return m_taskCount; }
    const int& scoreCount() const
    { return m_scoreCount; }
    const int& timeCount() const
    { return m_timeCount; }

    void setTaskCount(int ct)
    { m_taskCount = ct; }
    void setTimeCount(int ct)
    { m_timeCount = ct; }
    void setScoreCount(int ct)
    { m_scoreCount = ct; }

    void insert();          //存入厨师信息
    void saveCook();        //保存厨师工作数据

private:
    //做菜数量, 总分数, 总时间
    int m_taskCount;
    int m_scoreCount;
    int m_timeCount;
};

/*
 * 服务员类
 * 继承自用户类
 * 新增数据成员来保存服务员的工作数据
 */
class Waiter : public User
{
public:
    Waiter(int id = 0, QString phonenumber = "", QString password = "", \
           int taskCount = 0, int scoreCount = 0, int tableCount = 0, int timeCount = 0);
    Waiter(const User&user, int taskCount = 0, int scoreCount = 0, int tableCount = 0, int timeCount = 0);
    ~Waiter(){}

    const int& taskCount() const
    { return m_taskCount; }
    const int& scoreCount() const
    { return m_scoreCount; }
    const int& timeCount() const
    { return m_timeCount; }
    const int& tableCount() const
    { return m_tableCount; }

    void setTaskCount(int ct)
    { m_taskCount = ct; }
    void setTimeCount(int ct)
    { m_timeCount = ct; }
    void setScoreCount(int ct)
    { m_scoreCount = ct; }
    void setTableCount(int ct)
    { m_tableCount = ct; }

    void insert();          //存入服务员信息
    void saveWaiter();      //保存服务员工作数据

private:
    //服务次数, 服务桌数, 总分数, 总时间
    int m_taskCount;
    int m_tableCount;
    int m_scoreCount;
    int m_timeCount;
};

/*
 * 用户的聚合类
 * 便于整体操作
 */
class Users
{
public:
    Users(int type);
    Users(const Users &users);
    Users(){}
    ~Users();

    //用户基本操作
    void save();                        //保存所有用户
    void add(const User *user);
    void add(const Users &users);
    void remove(User *user);
    void clear();
    void edit(int index, QString phonenumber, QString password);
    int maxId();
    bool contains(const QString &phonenumber);
    Users search(const QString &key);
    Users& operator=(const Users& users);
    QVector<QStringList> toTableContents();     //将用户基本信息转换成便于在表格中显示的数据

    int type() const
    { return m_type; }
    void setType(const int &type)
    { m_type = type; }
    int count() const
    { return m_Users.count(); }
    User*& operator[](int i)
    { return m_Users[i]; }
    const User* operator[](int i) const
    { return m_Users[i]; }
    int indexOf(User *user);
    User* userValue(const QString &phonenumber);

private:
    //提取所有用户
    void extractUsersVector(int type);

    QVector<User *> m_Users;            //为了便于实现多态, Users中所有用户以指针形式存储
    int m_type;

public:
    static Users UsersData[5];          //单App模式下用于保存用户数据
};
#endif // USER_H
