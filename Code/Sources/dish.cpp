#include "dish.h"
#include "database.h"
#include "diningtable.h"

Dishes::Dishes(int table):m_table(table)
{
    if (SignWidget::useDataBase)//如果为多App模式, 从数据库中提取菜品
    {
        extractDishesVector();
    }
    else//如果为单App模式
    {
        if (table == 0)//如果从菜单中提取
        {
            *this = Dishes::MenuData;
        }
        else//从餐桌所点菜品中提取
        {
            *this = Dishes::DishesData[table-1];
        }
    }
}

void Dishes::clearDataBase() const
{
    if (SignWidget::useDataBase)//如果为多App模式, 清空数据库
    {
        DataBase db;
        if (m_table == 0)//如果清空菜单数据库
        {
            for (QString &category:categoryList)//清空每个类别的数据库
            {
                db = DataBase("Menu", category, menu_create_value);
                db.clear();
            }
        }//end of if
        else//如果清空某一餐桌所点菜品
        {
            db = DataBase("Dishes", QString("dining_table_%1").arg(m_table), dishes_create_value);
            db.clear();
        }
    }//end of if
    else//如果为单App模式, 清空对应的静态变量
    {
        if (m_table == 0)//如果清空菜单
        {
            MenuData.clear();
            MenuDataInCategory.clear();
        }
        else//如果清空某一餐桌所点菜品
        {
            DishesData[m_table-1].clear();
        }
    }//end of else
}

//从数据库中提取菜品
void Dishes::extractDishesVector()
{
    DataBase db;
    if (m_table == 0)//如果从菜单提取
    {
        db = DataBase("Menu", "category", menuCategory_create_value);
        categoryList = db.select("category");
        int id = 0;
        for (QString &category : categoryList)//遍历所有菜品类别的数据库
        {
            db = DataBase("Menu", category, menu_create_value);
            //提取数据信息
            auto names = db.select("name");
            auto prices = db.select("price");
            auto descriptions = db.select("description");
            auto dishcount = db.selectInt("ct");
            for (int i=0; i<db.count(); ++i)
            {
                id++;
                m_Dishes.append(Dish(category, names[i], prices[i].toDouble(), descriptions[i], "", dishcount[i], m_table, id));
            }//end of for
        }//end of for
    }//end of if
    else//如果从某一餐桌提取所点菜品
    {
        db = DataBase("Dishes", QString("dining_table_%1").arg(m_table), dishes_create_value);
        //提取数据信息
        auto ids = db.selectInt("id");
        auto categories = db.select("category");
        auto names = db.select("name");
        auto prices = db.select("price");
        auto descriptions = db.select("description");
        auto dishstates = db.select("dishstate");
        auto cookids = db.selectInt("cookid");
        for (int i=0; i<db.count(); ++i)
        {
            m_Dishes.append(Dish(categories[i], names[i], prices[i].toDouble(), descriptions[i], dishstates[i], 0, m_table, ids[i], cookids[i]));
        }//end of for
    }//end of else
}

/*
 * 提取未完成的菜品
 * 包括未被认领的菜品和已经被编号为cookid的厨师认领但未完成的菜品
 */
Dishes Dishes::extractUnfinishedDishes(int cookid)
{
    DiningTables tables;
    Dishes unfinishedDishes;
    for (int i=0; i<tables.count(); ++i)//遍历餐桌
    {
        if (tables[i].tableState() == "已下单")
        {
            Dishes dishes(tables[i].id());
            for (int j=0; j<dishes.count(); ++j)//遍历菜品
            {
                if ((dishes[j].dishState()=="等待中")
                        || ((dishes[j].dishState()=="制作中") && (dishes[j].cookId() == cookid)))
                {
                    unfinishedDishes.add(dishes[j]);
                }
            }//end of for
        }//end of if
    }//end of for
    return unfinishedDishes;
}

//获得该对象中所有菜品的总金额, 考虑每个菜品的数量
QString Dishes::totalPrice()
{
    double totalPrice = 0;
    for (int i=0; i<count(); ++i)
    {
        Dish thisDish = this->operator [](i);
        totalPrice += thisDish.price() * thisDish.count();
    }
    return QString::number(totalPrice, 'f', 2);
}

//获得该对象中所有菜品的总数目, 考虑每个菜品的数量
int Dishes::totalCount()
{
    int totalCount = 0;
    for (int i=0; i<m_Dishes.count(); ++i)
    {
        totalCount += m_Dishes[i].count();
    }
    return totalCount;
}

//将该对象中所有名称相同的菜品合并, 重数计入m_count中
void Dishes::merge()
{
    Dishes dishes;
    for (int i=0; i<count(); ++i)//遍历菜品
    {
        Dish thisDish = this->operator [](i);
        if (!dishes.contains(thisDish.name()))//如果dishes中不存在该菜品
        {
            //加入到dishes中, 设定个数为1
            thisDish.setCount(1);
            dishes.add(thisDish);
        }
        else//如果dishes中已经存在该菜品
        {
            for (int i=0; i<dishes.count(); ++i)//找到该菜品并使数量加一
            {
                if (dishes[i].name() == thisDish.name())
                {
                    dishes[i].setCount(dishes[i].count()+1);
                }
            }//end of for
        }//end of else
    }//end of for

    m_Dishes.clear();
    for (int i=0; i<dishes.count(); i++)
    {
        m_Dishes.append(dishes[i]);
    }
}

/*
 * 将每道菜品按照重数m_count拆分
 * 保证m_Dishes种的每个对象只表示一份菜品
 * 便于厨师制作和顾客查看每道菜品
 */
void Dishes::split()
{
    Dishes dishes;
    int id = 1;
    for (int i=0; i<count(); i++)
    {
        for (int j=0; j<m_Dishes[i].count(); j++)
        {
            m_Dishes[i].setId(id);
            id++;
            dishes.add(m_Dishes[i]);
        }
    }

    m_Dishes.clear();
    for (int i=0; i<dishes.count(); i++)
    {
        dishes[i].setCount(1);
        m_Dishes.append(dishes[i]);
    }
}

//保存
void Dishes::save()
{
    clearDataBase();
    for (int i=0; i<m_Dishes.count(); ++i)//重置编号
    {
        m_Dishes[i].setId(i+1);
        m_Dishes[i].insert();
    }
}

//添加
void Dishes::add(const Dishes &dishes)
{
    for (int i=0; i<dishes.count(); ++i)//将dishes中的Dish对象逐个加入本对象的m_Dishes中
    {
        m_Dishes.append(dishes[i]);
    }
}

//编辑
void Dishes::edit(int index, QString category, QString name, double price, QString description)
{
    m_Dishes[index].setCategory(category);
    m_Dishes[index].setName(name);
    m_Dishes[index].setPrice(price);
    m_Dishes[index].setDescription(description);
}

//获取最大的编号
int Dishes::maxId()
{
    int max = 0;
    for (int i=0; i<count(); ++i)
    {
        if (m_Dishes[i].id() > max)
        {
            max = m_Dishes[i].id();
        }
    }
    return max;
}

//判断是否存在名称为name的菜品
bool Dishes::contains(QString name)
{
    for (int i = 0; i<m_Dishes.count(); ++i)
    {
        if (m_Dishes[i].name() == name)
        {
            return true;
        }
    }
    return false;
}

//查找名称、价格、描述中含有关键字key的Dish对象, 并返回一个由它们组成的Dishes对象
Dishes Dishes::search(const QString &key)
{
    Dishes searchResult;
    for (int i = 0;i<m_Dishes.count();++i)
    {
        if (m_Dishes[i].name().contains(key)
                || m_Dishes[i].priceToString().contains(key)
                || m_Dishes[i].description().contains(key))
            searchResult.add(m_Dishes[i]);
    }
    return searchResult;
}

//将Dishes对象转换成便于在表格显示的数据
QVector<QStringList> Dishes::toTableContents()
{
    QStringList number;
    QStringList category;
    QStringList name;
    QStringList price;
    QStringList description;
    for (int i=0; i<count(); ++i)
    {
        number.append(QString::number(i+1));
        category.append(m_Dishes[i].category());
        name.append(m_Dishes[i].name());
        price.append(m_Dishes[i].priceToString());
        description.append(m_Dishes[i].description());
    }

    QVector<QStringList> contents;
    contents.append(number);
    contents.append(category);
    contents.append(name);
    contents.append(price);
    contents.append(description);

    return contents;
}

//设置桌号
void Dishes::setTable(int table)
{
    m_table = table;
    for (int i = 0;i<m_Dishes.count();++i)
    {
        m_Dishes[i].setTable(table);
    }
}

//修改菜品类别
void Dishes::setCategory(QString category)
{
    m_category = category;
    for (Dish &i : m_Dishes)
    {
        i.setCategory(category);
    }
}

//将菜品信息分类别存储成Hash表并返回
QHash<QString, Dishes> Dishes::categoryHash()
{
    QHash<QString, Dishes> result;
    for (Dish &dish : m_Dishes)
    {
        result[dish.category()].add(dish);
    }
    for (QString &key : result.keys())
    {
        result[key].setCategory(key);
        result[key].setTable(0);
    }
    return result;
}

//重载复制运算符, 使得可以将按菜品类别存储的Hash表直接转换成Dishes对象
Dishes& Dishes::operator=(QHash<QString, Dishes> dishesInCategory)
{
    clear();
    for (QString &category : categoryList)
    {
        add(dishesInCategory[category]);
    }
    return *this;
}

//保存菜品类别信息
void Dishes::saveCategory()
{
    if (SignWidget::useDataBase)//如果为多App模式, 则保存到数据库(单App模式下仍用Dishes::categoryList来维护)
    {
        DataBase db("Menu", "category", menuCategory_create_value);
        db.clear();
        for (QString &category : categoryList)
        {
            db.insert(QString("(\"%1\")").arg(category));
        }
    }
}

//保存菜单中菜品的销售数量
void Dish::saveCount()
{
    if (SignWidget::useDataBase)//如果为多App模式, 更新数据库
    {
        DataBase db("Menu", this->category(), menu_create_value);
        db.update("ct", QString::number(m_count), QString("id = %1").arg(m_id));
    }
    else//如果为单App模式, 修改静态变量
    {
        Dishes::MenuData[m_id-1].setCount(m_count);
    }
}

//插入
void Dish::insert()
{
    if (SignWidget::useDataBase)//如果为多App模式, 插入到数据库
    {
        DataBase db;
        if (m_table == 0)//如果插入菜单中的菜品
        {
            db = DataBase("Menu", category(), menu_create_value);
            db.insert(QString("(%1, \"%2\", \"%3\", \"%4\", \"%5\", %6)").arg(m_id).arg(m_category).arg(m_name).arg(m_price).arg(description()).arg(m_count));
        }
        else//如果插入某一餐桌所点菜品
        {
            db = DataBase("Dishes", QString("dining_table_%1").arg(m_table), dishes_create_value);
            db.insert(QString("(%1, \"%2\", \"%3\", \"%4\", \"%5\", \"%6\", %7)").arg(m_id).arg(m_category).arg(m_name).arg(m_priceString).arg(m_description).arg(m_dishState).arg(m_cookid));
        }
    }//end of if
    else//如果为单App模式
    {
        if (m_table == 0)//如果插入菜单中的菜品
        {
            Dishes::MenuData.add(*this);
            Dishes::MenuDataInCategory[category()].add(*this);
        }
        else//如果插入某一餐桌所点菜品
        {
            Dishes::DishesData[m_table-1].add(*this);
        }
    }
}

/*
 * 保存某一餐桌所点菜品的信息
 * 保存类型由type标识
 * s表示保存状态
 * c表示保存厨师编号
 */
void Dish::save(char type)
{
    if (SignWidget::useDataBase)//如果为多App模式, 保存到数据库
    {
        DataBase db;
        db = DataBase("Dishes", QString("dining_table_%1").arg(m_table), dishes_create_value);
        if (type == 's')
        {
            db.update("dishstate", m_dishState, QString("id = %1").arg(m_id));
        }
        else if (type == 'c')
        {
            db.update("cookid", QString::number(m_cookid), QString("id = %1").arg(m_id));
        }
    }
    else//如果为单App模式
    {
        if (type == 's')
        {
            Dishes::DishesData[m_table-1][m_id-1].setState(m_dishState);
        }
        else if (type == 'c')
        {
            Dishes::DishesData[m_table-1][m_id-1].setCookId(m_cookid);
        }
    }
}

//重载==运算符, 判断两个Dish是否相等
bool Dish::operator ==(Dish dish)
{
    if ((m_category==dish.category()) && (m_name==dish.name()) && (m_price==dish.price())\
            && (m_description==dish.description()) && (m_id==dish.id())\
            && (m_cookid==dish.cookId()) && (m_table==dish.table()) && (m_dishState==dish.dishState()))
    {
        return true;
    }
    else
    {
        return false;
    }
}

//静态变量初始化为空值
QStringList Dishes::categoryList = QStringList();
Dishes Dishes::MenuData = Dishes();
QHash<QString, Dishes> Dishes::MenuDataInCategory = QHash<QString, Dishes>();
QVector<Dishes> Dishes::DishesData = QVector<Dishes>();
