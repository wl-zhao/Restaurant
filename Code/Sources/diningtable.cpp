#include "diningtable.h"
#include "database.h"

DiningTables::DiningTables(bool extract)//extract 标识是否提取所有的餐桌
{
    if (extract)
    {
        if (SignWidget::useDataBase)//如果为多App模式, 则从数据库提取
        {
            extractDiningTables();
        }
        else//否则从静态变量TablesData中提取
        {
            *this = DiningTables::TablesData;
        }
    }
}

//从数据库提取所有餐桌
void DiningTables::extractDiningTables()
{
    m_tables.clear();
    DataBase db("DiningTable","diningtable",table_create_value);
    auto ids = db.selectInt("id");
    auto states = db.select("tablestate");
    auto messages = db.select("message");
    auto waiterids = db.selectInt("waiterid");
    for (int i=0;i<db.count();++i)
    {
        DiningTable table(ids[i],messages[i],waiterids[i],states[i]);
        m_tables.append(table);
    }
}

//提取所有待认领餐桌
DiningTables DiningTables::extractWaitingTables()
{
    DiningTables diningTables;
    DiningTables waitingTables(false);
    for (int i=0; i<diningTables.count(); ++i)
    {
        //餐桌有顾客占用且无服务员认领
        if (((diningTables[i].tableState()=="点菜中") || (diningTables[i].tableState()=="已下单")\
           || (diningTables[i].tableState()=="结账中")) && (diningTables[i].waiterId()==-1))
        {
            waitingTables.add(diningTables[i]);
        }
    }
    return waitingTables;
}

//提取编号为waiterId的服务员认领的所有餐桌
DiningTables DiningTables::extractServingTables(int waiterid)
{
    DiningTables servingTables(false);
    DiningTables diningTables;
    for (int i=0; i<diningTables.count(); ++i)
    {
        if (diningTables[i].waiterId() == waiterid)
        {
            servingTables.add(diningTables[i]);
        }
    }
    return servingTables;
}

void DiningTables::save()
{
    if (SignWidget::useDataBase)//如果当前模式为多App模式, 则保存到数据库
    {
        DataBase db("DiningTable","diningtable",table_create_value);
        db.clear();
        for (int i=0; i<m_tables.count(); ++i)
        {
            db.insert(QString("(%1,\"%2\",\"%3\",%4)").arg(m_tables[i].id()).arg(m_tables[i].tableState()).arg(m_tables[i].message()).arg(m_tables[i].waiterId()));
        }
    }
    else//否则保存在静态变量TablesData中
    {
        DiningTables::TablesData = *this;
        Dishes::DishesData = QVector<Dishes>(DiningTables::TablesData.count()); //重置DishesData的大小
    }
}

/*
 * 用于更改餐桌数量
 * 分为强制模式和非强制模式
 *
 * 强制模式下, 直接将餐桌个数设为count, 返回true
 * 如果count比原来的餐桌数量少, 则丢弃多余的餐桌
 *
 * 非强制模式下, 如果丢弃的餐桌中有顾客占用, 则放弃更改, 返回false
 * 反之返回true
 */
bool DiningTables::setTableNumber(int count,bool compulsory)
{
    if (m_tables.count() <= count)//如果要增加餐桌
    {
        //直接增加
        for (int i=m_tables.count();i<count;++i)
        {
            m_tables.append(DiningTable(i+1));
        }
        return true;
    }
    else//如果要减少餐桌
    {
        if (compulsory)//强制模式下
        {
            for (int i=m_tables.count()-1; i>= count; --i)//丢弃多余的餐桌
            {
                m_tables.removeAt(i);
            }
            return true;
        }
        else//非强制模式下
        {
            for (int i=count; i<m_tables.count(); ++i)
            {
                //若丢弃的餐桌中有顾客占用, 返回false
                if (m_tables[i].tableState()!="空闲" && m_tables[i].tableState()!="禁用")
                {
                    return false;
                }
            }
            for (int i=m_tables.count()-1; i>= count; --i)//丢弃多余的餐桌
            {
                m_tables.removeAt(i);
            }
            return true;
        }
        return true;
    }
}

DiningTable DiningTable::extractTable(int tableid)
{
    DiningTables tables;
    return tables[tableid-1];
}

/*
 * 保存餐桌数据
 * 保存的成员由type来标识
 * s代表保存状态
 * m代表保存消息
 * w代表保存服务员编号
 */
void DiningTable::save(char type)
{
    if (SignWidget::useDataBase)//如果为多App模式, 保存到数据库
    {
        DataBase db("DiningTable","diningtable",table_create_value);
        //根据type保存不同的成员
        if (type == 's')
        {
            db.update("tablestate",m_tableState,QString("id = %1").arg(m_id));
        }
        else if (type == 'm')
        {
            db.update("message",m_message,QString("id = %1").arg(m_id));
        }
        else if (type == 'w')
        {
            db.update("waiterid",QString::number(m_waiterid),QString("id = %1").arg(m_id));
        }
    }
    else//如果为单App模式, 保存在静态变量中
    {
        if (type == 's')
        {
            DiningTables::TablesData[m_id-1].setState(m_tableState);
        }
        else if (type == 'm')
        {
            DiningTables::TablesData[m_id-1].setMessage(m_message);
        }
        else if (type == 'w')
        {
            DiningTables::TablesData[m_id-1].setWaiterId(m_waiterid);
        }
    }
}

//静态变量初始化为空值
DiningTables DiningTables::TablesData = DiningTables(false);
