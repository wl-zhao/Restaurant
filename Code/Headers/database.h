#ifndef DATABASE_H
#define DATABASE_H
#include <QString>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVector>
#include "user.h"

//创建数据库表格时的参数
static const QString user_create_value = "(id int primary key, phonenumber varchar(11), password varchar(16))";
static const QString customer_create_value = "(id int primary key, phonenumber varchar(11), password varchar(16), tableid int)";
static const QString cook_create_value = "(id int primary key, phonenumber varchar(11), password varchar(16), taskct int, scorect int, timect int)";
static const QString waiter_create_value = "(id int primary key, phonenumber varchar(11), password varchar(16), taskct int, scorect int, tablect int, timect int)";
static const QString menu_create_value = "(id int primary key, category varchar(30), name varchar(30) , price varchar(30), description varchar(200), ct int)";
static const QString table_create_value = "(id int primary key, tablestate varchar(10), message varchar(100), waiterid int)";//删去了paysatate
static const QString dishes_create_value = "(id int primary key, category varchar(30), name varchar(30), price varchar(30), description varchar(200), dishstate varchar(15), cookid int)";
static const QString menuCategory_create_value = "(category varchar(30))";

/*
 * 封装了数据库的常用操作
 */
class DataBase
{
public:
    DataBase(QString dbname, QString tbname, QString parameters, QString primaryKey = "id");
    DataBase(int type, QString primaryKey="id");//专门用来打开用户数据库, type标识用户类别
    DataBase(){}
    ~DataBase();
    void insert(QString values);                                        //插入
    void update(QString parameter, QString par_value, QString expression);//更新
    QStringList select(QString parameter);                              //提取所有的条目中的parameter项
    QVector<int> selectInt(QString parameter);                          //提取所有条目中的parameter项并转为整值
    int selectMax(QString parameter);                                   //返回parameter的最大值
    void deleteline(QString parameter,QString value);
    void clear();                                                       //清空数据表
    int count();

private:
    QSqlDatabase openDatabase(QString name);

    QString m_dbname;           //数据库名称
    QString m_tbname;           //表格名称
    QString m_parameters;       //创建表格所需参数
    QString m_primaryKey;       //主键
    QSqlDatabase m_database;
};

#endif // DATABASE_H
