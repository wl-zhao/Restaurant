#include "user.h"

//获得对应用户类别的指针, 便于在存入用户时实现多态
User* User::getPointer(const User &user)
{
    User *pointer;

    switch (user.type())
    {
    case 0:
        pointer = new Customer(user);
        break;
    case 2:
        pointer = new Cook(user);
        break;
    case 3:
        pointer = new Waiter(user);
        break;
    default:
        pointer = new User(user);
        break;
    }

    return pointer;
}

/*
 * 存入用户信息
 * 用户类别为管理员或经理时使用
 * 在顾客、厨师、服务员类中被重写
 */
void User::insert()
{
    if (SignWidget::useDataBase)//如果处于多App模式
    {
        //存入数据库
        DataBase db(m_type);
        int max_id  = db.selectMax("id");
        m_id = max_id+1;
        db.insert(QString("(%1, \"%2\", \"%3\")").arg(max_id+1).arg(m_phonenumber).arg(m_password));
    }
    else//如果处于单App模式
    {
        //存入静态变量
        m_id = Users::UsersData[m_type].maxId()+1;
        Users::UsersData[m_type].add(new User(*this));
    }
}

//重载==运算符判断两个用户是否相等
bool User::operator==(User &user)
{
    if (user.id()==m_id && user.phonenumber()==m_phonenumber && \
            user.password()==m_password && user.type()==m_type)
    {
        return true;
    }
    else
    {
        return false;
    }
}

//保存顾客的桌号
void Customer::saveTableId()
{
    if (SignWidget::useDataBase)//如果处于多App模式
    {
        //保存桌号至数据库
        DataBase db(m_type);
        db.update("tableid", QString::number(m_tableid), QString("id = %1").arg(m_id));
    }
    else//如果处于单App模式
    {
        //保存至静态变量
        Customer *customer = dynamic_cast<Customer*>(Users::UsersData[0][m_id-1]);
        customer->setTableId(m_tableid);
    }
}

//存入顾客信息
void Customer::insert()
{
    if (SignWidget::useDataBase)//如果处于多App模式
    {
        //保存至数据库
        DataBase db(m_type);
        int max_id = db.selectMax("id");
        m_id = max_id+1;
        db.insert(QString("(%1, \"%2\", \"%3\", %4)").arg(max_id+1).arg(m_phonenumber).arg(m_password)\
                  .arg(m_tableid));
    }
    else//如果处于单App模式
    {
        //保存至静态变量
        m_id = Users::UsersData[0].maxId()+1;
        Users::UsersData[0].add(new Customer(*this));
    }
}

//厨师构造函数
Cook::Cook(int id, QString phonenumber, QString password, \
           int taskCount, int scoreCount, int timeCount):User(id, phonenumber, password)
{
    m_taskCount = taskCount;
    m_scoreCount = scoreCount;
    m_timeCount = timeCount;
    m_type = 2;
}

Cook::Cook(const User &user, int taskCount, int scoreCount, int timeCount):User(user)
{
    m_taskCount = taskCount;
    m_scoreCount = scoreCount;
    m_timeCount = timeCount;
    m_type = 2;
}

//存入厨师信息
void Cook::insert()
{
    if (SignWidget::useDataBase)//如果处于多App模式
    {
        //保存至数据库
        DataBase db(m_type);
        int max_id = db.selectMax("id");
        m_id = max_id+1;
        db.insert(QString("(%1, \"%2\", \"%3\", %4, %5, %6)").arg(max_id+1).arg(m_phonenumber).arg(m_password)\
                  .arg(m_taskCount).arg(m_scoreCount).arg(m_timeCount));
    }
    else//如果处于单App模式
    {
        //保存至静态变量
        m_id = Users::UsersData[2].maxId()+1;
        Users::UsersData[2].add(new Cook(*this));
    }
}

//保存厨师工作数据
void Cook::saveCook()
{
    if (SignWidget::useDataBase)//如果处于多App模式
    {
        //保存工作数据至数据库
        DataBase db(m_type);
        db.update("taskct", QString::number(m_taskCount), QString("id = %1").arg(m_id));
        db.update("scorect", QString::number(m_scoreCount), QString("id = %1").arg(m_id));
        db.update("timect", QString::number(m_timeCount), QString("id = %1").arg(m_id));
    }
    else//如果处于单App模式
    {
        //保存工作数据至静态变量
        Cook *cook = dynamic_cast<Cook *>(Users::UsersData[2][m_id-1]);
        cook->setTaskCount(m_taskCount);
        cook->setScoreCount(m_scoreCount);
        cook->setTimeCount(m_timeCount);
    }
}

//服务员构造函数
Waiter::Waiter(int id, QString phonenumber, QString password, \
               int taskCount, int scoreCount, int tableCount, int timeCount):User(id, phonenumber, password)
{
    m_taskCount = taskCount;
    m_scoreCount = scoreCount;
    m_tableCount = tableCount;
    m_timeCount = timeCount;
    m_type = 3;
}

Waiter::Waiter(const User &user, int taskCount, int scoreCount, int tableCount, int timeCount):User(user)
{
    m_taskCount = taskCount;
    m_scoreCount = scoreCount;
    m_tableCount = tableCount;
    m_timeCount = timeCount;
    m_type = 3;
}

//存入服务员数据
void Waiter::insert()
{
    if (SignWidget::useDataBase)//如果处于多App模式
    {
        //保存至数据库
        DataBase db(m_type);
        int max_id = db.selectMax("id");
        m_id = max_id+1;
        db.insert(QString("(%1, \"%2\", \"%3\", %4, %5, %6, %7)").arg(max_id+1).arg(m_phonenumber).arg(m_password)\
                  .arg(m_taskCount).arg(m_scoreCount).arg(m_tableCount).arg(m_timeCount));
    }
    else//如果处于单App模式
    {
        //保存至静态变量
        m_id = Users::UsersData[3].maxId()+1;
        Users::UsersData[3].add(new Waiter(*this));
    }
}

//保存服务员工作数据
void Waiter::saveWaiter()
{
    if (SignWidget::useDataBase)//如果处于多App模式
    {
        //保存至数据库
        DataBase db(m_type);
        db.update("taskct", QString::number(m_taskCount), QString("id = %1").arg(m_id));
        db.update("scorect", QString::number(m_scoreCount), QString("id = %1").arg(m_id));
        db.update("tablect", QString::number(m_tableCount), QString("id = %1").arg(m_id));
        db.update("timect", QString::number(m_timeCount), QString("id = %1").arg(m_id));
    }
    else//如果处于单App模式
    {
        //保存至静态变量
        Waiter* waiter = dynamic_cast<Waiter*>(Users::UsersData[3][m_id-1]);
        waiter->setTaskCount(m_taskCount);
        waiter->setScoreCount(m_scoreCount);
        waiter->setTableCount(m_tableCount);
        waiter->setTimeCount(m_timeCount);
    }
}

/*
 * Users的构造函数
 * 提取用户信息
 */
Users::Users(int type):m_type(type)
{
    if (SignWidget::useDataBase)//如果处于多App模式
    {
        //从数据库读取
        extractUsersVector(m_type);
    }
    else//如果处于单App模式
    {
        //从静态变量中读取
        *this = Users::UsersData[m_type];
    }
}

/*
 * 复制构造函数
 * 完成深复制
 */
Users::Users(const Users& users)
{
    clear();

    for (int i=0; i<users.count(); ++i)
    {
        add(users[i]);
    }

    m_type = users.m_type;
}

Users::~Users()
{
    clear();
}

/*
 * 清空Users中所有用户
 * 并释放动态分配的内存
 */
void Users::clear()
{
    for (int i=0; i<m_Users.count(); ++i)
    {
        auto p = m_Users[i];
        delete p;
        m_Users[i] = 0;
    }

    m_Users.clear();
}

//从数据库中提取用户信息, 并存入m_Users中
void Users::extractUsersVector(int type)
{
    DataBase db(type);
    QVector<int> ids = db.selectInt("id");
    QStringList phonenumbers = db.select("phonenumber");
    QStringList passwords = db.select("password");

    if (type==1 || type==4)
    {
        for (int i=0; i<db.count(); ++i)
        {
            auto user = new User(ids[i], phonenumbers[i], passwords[i], type);
            m_Users.append(user);
        }//end of for
    }
    else if (type == 0)
    {
        auto tableid = db.selectInt("tableid");
        for (int i = 0; i<db.count(); ++i)
        {
            auto customer = new Customer(ids[i], phonenumbers[i], passwords[i], tableid[i]);
            m_Users.append(customer);
        }//end of for
    }
    else if (type == 2)
    {
        auto taskct = db.selectInt("taskct");
        auto scorect = db.selectInt("scorect");
        auto timect = db.selectInt("timect");
        for (int i = 0;i<db.count();++i)
        {
            auto cook = new Cook(ids[i], phonenumbers[i], passwords[i], taskct[i], scorect[i], timect[i]);
            m_Users.append(cook);
        }//end of for
    }
    else if (type == 3)
    {
        auto taskct = db.selectInt("taskct");
        auto  scorect = db.selectInt("scorect");
        auto  tablect = db.selectInt("tablect");
        auto  timect = db.selectInt("timect");
        for (int i = 0; i<db.count(); ++i)
        {
            auto waiter = new Waiter(ids[i], phonenumbers[i], passwords[i], taskct[i], scorect[i], tablect[i], timect[i]);
            m_Users.append(waiter);
        }//end of for
    }
}

//保存m_Users中的所有用户信息
void Users::save()
{
    if (SignWidget::useDataBase)//如果处于多App模式
    {
        //清空数据库
        DataBase db(m_type);
        db.clear();
    }
    else//如果处于单App模式
    {
        //清空对应的静态变量
        Users::UsersData[m_type].clear();
    }
    for (User *&i : m_Users)//逐个存入
    {
        i->insert();
    }
}

/*
 * 添加一个用户
 * 需保留该用户的全部信息
 */
void Users::add(const User *user)
{
    User *u;

    switch (user->type())
    {
    case 0:
        u = new Customer(*(Customer *)user);
        break;
    case 2:
        u = new Cook(*(Cook *)user);
        break;
    case 3:
        u = new Waiter(*(Waiter*)user);
        break;
    default:
        u = new User(*user);
        break;
    }

    m_Users.append(u);
}

//将users中的用户全部添加到m_Users中
void Users::add(const Users &users)
{
    for (int i=0; i<users.count(); ++i)
    {
        add(users[i]);
    }
}

//移除用户
void Users::remove(User *user)
{
    for (int i=0; i<count(); ++i)
    {
        if (*m_Users[i] == *user)
        {
            delete m_Users[i];
            m_Users[i] = 0;
            m_Users.removeAt(i);
            return;
        }
    }//end of for
}

//编辑用户信息
void Users::edit(int index, QString phonenumber, QString password)
{
    m_Users[index]->setPhonenumber(phonenumber);
    m_Users[index]->setPasssword(password);
}

//获取当前对象中所有用户的编号最大值
int Users::maxId()
{
    int max = 0;
    for (int i=0; i<count(); ++i)
    {
        if (m_Users[i]->id() > max)
        {
            max = m_Users[i]->id();
        }
    }
    return max;
}

//判断当前对象中所有用户的手机号是否包含phonenumber
bool Users::contains(const QString &phonenumber)
{
    for (User *&i : m_Users)
    {
        if (i->phonenumber() == phonenumber)
        {
            return true;
        }
    }
    return false;
}

/*
 * 在手机号和密码中查找关键字key
 * 返回查找结果的集合
 */
Users Users::search(const QString &key)
{
    Users searchResult;
    for (User *& i : m_Users)
    {
        if (i->phonenumber().contains(key) || i->password().contains(key))
        {
            searchResult.add(i);
        }
    }
    return searchResult;
}

/*
 * 查找user在m_Users中的编号
 * 查询失败返回-1
 */
int Users::indexOf(User *user)
{
    for (int i=0; i<count(); ++i)
    {
        if (*m_Users[i] == *user)
        {
            return i;
        }
    }
    return -1;
}

//找到手机号为phonenumber的用户
User* Users::userValue(const QString &phonenumber)
{
    for (User *&i : m_Users)
    {
        if (i->phonenumber() == phonenumber)
        {
            return i;
        }
    }

    return 0;
}

/*
 * 重载赋值运算符
 * 完成深复制
 */
Users& Users::operator=(const Users &users)
{
    if (this == &users)
        return *this;
    else
    {
        clear();
        for (int i=0; i<users.count(); ++i)
        {
            this->add(users[i]);
        }
        m_type = users.m_type;
        return *this;
    }//end of else
}

//将用户的基本信息装换成便于在表格中显示的数据
QVector<QStringList> Users::toTableContents()
{
    QStringList number;
    QStringList type;
    QStringList phonenumber;
    QStringList password;

    for (int i = 0;i<count();++i)
    {
        number.append(QString::number(i+1));
        type.append(Identity[m_Users[i]->type()]);
        phonenumber.append(m_Users[i]->phonenumber());
        password.append(m_Users[i]->password());
    }

    QVector<QStringList> contents;
    contents.append(number);
    contents.append(type);
    contents.append(phonenumber);
    contents.append(password);

    return contents;
}

//静态变量初始化为空值
Users Users::UsersData[5] = {Users(), Users(), Users(), Users()};
