#include "database.h"


DataBase::DataBase(QString dbname, QString tbname, QString parameters, QString primaryKey) : \
    m_dbname(dbname), m_tbname(tbname), m_parameters(parameters), m_primaryKey(primaryKey)
{

}

DataBase::~DataBase()
{

}

//根据用户类别初始化成员
DataBase::DataBase(int type, QString primaryKey):m_primaryKey(primaryKey)
{
    m_dbname = "Users";
    m_tbname = Identity[type];
    if (type == 0)
    {
        m_parameters = customer_create_value;
    }
    else if (type == 2)
    {
        m_parameters = cook_create_value;
    }
    else if (type == 3)
    {
        m_parameters = waiter_create_value;
    }
    else
    {
        m_parameters = user_create_value;
    }
}

//将values内部的数据插入数据表
void DataBase::insert(QString values)
{
    m_database = openDatabase(m_dbname);
    QSqlQuery sql_query(m_database);
    QString create_sql = \
            QString("create table %1 %2").arg(m_tbname).arg(m_parameters);
    sql_query.exec(create_sql);
    QString insert_sql = QString("INSERT INTO %1 VALUES %2").arg(m_tbname).arg(values);
    if (!sql_query.exec(insert_sql))
    {
        qDebug() << sql_query.lastError();
    }
}

//将满足表达式expression的条目的parameter项的值改为par_value
void DataBase::update(QString parameter, QString par_value, QString expression)
{
    m_database = openDatabase(m_dbname);
    QSqlQuery sql_query(m_database);
    QString create_sql = \
            QString("create table %1 %2").arg(m_tbname).arg(m_parameters);
    if (!sql_query.exec(create_sql))
        qDebug() << sql_query.lastError();
    QString update_sql = QString("update %1 set %2 = :%2 where %3").arg(m_tbname).arg(parameter).arg(expression);
    sql_query.prepare(update_sql);
    sql_query.bindValue(QString(":%2").arg(parameter), par_value);
    if (!sql_query.exec())
    {
        qDebug() << sql_query.lastError();
    }
    m_database.close();
}

//提取所有的条目中的parameter项
QStringList DataBase::select(QString parameter)
{
    QStringList result;
    m_database = openDatabase(m_dbname);
    QSqlQuery sql_query(m_database);
    QString create_sql = \
            QString("create table %1 %2").arg(m_tbname).arg(m_parameters);
    sql_query.exec(create_sql);
    QString select_sql = QString("select %1 from %2").arg(parameter).arg(m_tbname);
    if (sql_query.exec(select_sql))
    {
        while(sql_query.next())
        {
            result.append(sql_query.value(0).toString());
        }
    }
    return result;
}

//提取所有条目中的parameter项并转为整值
QVector<int> DataBase::selectInt(QString parameter)
{
    QVector<int> result;
    m_database = openDatabase(m_dbname);
    QSqlQuery sql_query(m_database);
    QString create_sql = \
            QString("create table %1 %2").arg(m_tbname).arg(m_parameters);
    sql_query.exec(create_sql);
    QString select_sql = QString("select %1 from %2").arg(parameter).arg(m_tbname);
    if (sql_query.exec(select_sql))
    {
        while(sql_query.next())
        {
            result.append(sql_query.value(0).toString().toInt());
        }
    }
    return result;
}

//返回parameter的最大值
int DataBase::selectMax(QString parameter)
{
    m_database = openDatabase(m_dbname);
    QSqlQuery sql_query(m_database);
    QString create_sql = \
            QString("create table %1 %2").arg(m_tbname).arg(m_parameters);
    sql_query.exec(create_sql);
    QString select_max_sql = QString("select max(%1) from %2").arg(parameter).arg(m_tbname);
    int max = 0;
    sql_query.prepare(select_max_sql);
    if (!sql_query.exec())
    {
        qDebug() << sql_query.lastError();
    }
    else
    {
        while(sql_query.next())
        {
            max = sql_query.value(0).toInt();
        }
    }
    m_database.close();
    return max;
}

void DataBase::deleteline(QString parameter, QString value)
{
    m_database=openDatabase(m_dbname);
    QSqlQuery sql_query(m_database);
    QString create_sql=\
            QString("create table %1 %2").arg(m_tbname).arg(m_parameters);
    sql_query.exec(create_sql);
    QString delete_sql = QString("delete from %1 where %2 = ?").arg(m_tbname).arg(parameter);
    sql_query.prepare(delete_sql);
    sql_query.addBindValue(value);
    if(!sql_query.exec())
    {
        qDebug()<<sql_query.lastError();
    }
    m_database.close();
}

//清空数据表
void DataBase::clear()
{
    m_database = openDatabase(m_dbname);
    QSqlQuery sql_query(m_database);
    QString create_sql = \
            QString("create table %1 %2").arg(m_tbname).arg(m_parameters);
    sql_query.exec(create_sql);
    QString clear_sql = QString("delete from %1").arg(m_tbname);
    sql_query.prepare(clear_sql);
    if (!sql_query.exec())
    {
        qDebug() << sql_query.lastError();
    }
    m_database.close();
}

//返回数据表行数
int DataBase::count()
{
    m_database = openDatabase(m_dbname);
    return select(m_primaryKey).count();
}

//打开数据库的初始操作
QSqlDatabase DataBase::openDatabase(QString name)
{
    QSqlDatabase database;
    if (QSqlDatabase::contains(name))
    {
        database = QSqlDatabase::database(name);
    }
    else
    {
        database = QSqlDatabase::addDatabase("QSQLITE", name);
        database.setDatabaseName(name);
    }
    if (!database.open())
    {
        qDebug() << database.lastError();
    }
    return database;
}
