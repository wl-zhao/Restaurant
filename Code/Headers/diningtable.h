#ifndef DININGTABLE_H
#define DININGTABLE_H
#include <QVector>
#include "dish.h"

//餐桌类
class DiningTable
{
public:
    DiningTable(int id = 0, QString message = "", int waiterid = -1, QString tableState = "空闲") : \
        m_id(id), m_message(message), m_waiterid(waiterid), m_tableState(tableState)
    {

    }
    void setWaiterId(int id)
    { m_waiterid = id; }
    void setState(QString state)
    { m_tableState = state; }
    void setMessage(QString message)
    { m_message = message; }
    void save(char type);
    int waiterId() const
    { return m_waiterid; }
    QString tableState() const
    { return m_tableState; }
    QString message() const
    { return m_message; }
    int id()
    { return m_id; }
    static DiningTable extractTable(int tableid);
private:
    int m_id;               //餐桌编号
    QString m_message;      //餐桌消息
    int m_waiterid;         //服务员编号
    QString m_tableState;   //餐桌状态
};

/*
 * 餐桌的聚合类
 * 便于整体操作
 */
class DiningTables
{
public:
    DiningTables(bool extract = true);
    ~DiningTables(){}


    static DiningTables extractWaitingTables();             //提取待认领餐桌
    static DiningTables extractServingTables(int waiterid); //提取编号为waiterId服务的餐桌

    bool setTableNumber(int count, bool compulsory = false);   //设定餐桌数目
    void save();//保存餐桌数据
    int count()
    { return m_tables.count(); }
    void add(DiningTable table)
    { m_tables.append(table); }

    DiningTable& operator[](int i)
    { return m_tables[i]; }
    const DiningTable& operator[](int i) const
    { return m_tables[i]; }

public:
    static DiningTables TablesData; //单App模式下存储餐桌数据

private:
    void extractDiningTables();      //提取所有餐桌
    QVector<DiningTable> m_tables;  //存储餐桌的数组
};

#endif // DININGTABLE_H
