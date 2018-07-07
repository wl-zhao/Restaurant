#ifndef DISH_H
#define DISH_H
#include <QString>
#include <QVector>

//菜品类
class Dish
{
public:
    Dish(QString category, QString name, double price, QString description, \
         QString dishState = "等待中", int count = 0, int table = 0, int id = 0, int cookid = -1)\
        :m_category(category), m_name(name), m_price(price), m_description(description), \
          m_count(count), m_dishState(dishState), m_cookid(cookid), m_table(table), m_id(id)
    {
        m_priceString = QString::number(m_price, 'f', 2);
    }
    Dish(){}
    ~Dish(){}

    //获取菜品属性的接口
    const QString category() const
    { return m_category; }
    QString name() const
    { return m_name; }
    double price() const
    { return m_price; }
    QString priceToString() const
    { return m_priceString; }
    QString description() const
    { return m_description; }
    QString dishState() const
    { return m_dishState; }
    int table() const
    { return m_table; }
    int  count() const
    { return m_count; }
    int id() const
    { return m_id; }
    int cookId() const
    { return m_cookid; }

    //修改菜品属性的接口
    void setCategory(const QString &category)
    { m_category = category; }
    void setName(const QString &name)
    { m_name = name; }
    void setPrice(const double &price)
    {
        m_price = price;
        m_priceString = QString::number(m_price, 'f', 2);
    }
    void setDescription(const QString&description)
    { m_description = description; }
    void setState(const QString &state)
    { m_dishState = state; }
    void setCookId(const int &id)
    {m_cookid = id;}
    void setId(const int &id)
    { m_id = id; }
    void setTable(const int &table)
    { m_table = table; }
    void setCount(int ct)
    { m_count = ct; }

    //保存菜品信息
    void insert();
    void saveCount();
    void save(char type);

    bool operator ==(Dish dish);

private:
    int m_cookid;
    int m_table;//菜品所属餐桌编号, 0值表示该对象为菜单中的菜品
    int m_id;
    QString m_category;
    QString m_name;
    double m_price;
    QString m_priceString;      //字符串形式的价格
    QString m_description;
    QString m_dishState;

    /*
     * 记录菜品被点的次数
     * 当本对象标识菜单中的一道菜时, m_count为这道菜被所有的客人点过的次数
     * 当本对象为顾客某次点餐托盘中的菜品时, m_count为该顾客点这道菜的份数
     */
    int m_count;
};

/*
 * 菜品的聚合类
 * 便于整体操作
 */
class Dishes
{
public:
    Dishes(int table);
    Dishes(){}
    ~Dishes(){}

    static Dishes extractUnfinishedDishes(int cookid);

    //基本操作
    void merge();//合并菜品
    void split();//拆分菜品
    void save();
    void clearDataBase() const;
    void setTable(int table);
    void setCategory(QString category);
    void clear()
    { m_Dishes.clear(); }
    void add(const Dish &dish)
    { m_Dishes.append(dish); }
    void add(const Dishes &dishes);
    void remove(const Dish &dish)
    { m_Dishes.removeOne(dish); }
    void remove(int i)
    { m_Dishes.remove(i); }
    void edit(int index, QString category, QString name, double price, QString description);
    Dishes search(const QString &key);
    QVector<QStringList> toTableContents();//将Dishes对象转换成便于在表格显示的数据

    QString totalPrice();
    int totalCount();
    int maxId();
    int count() const
    { return m_Dishes.count(); }
    bool contains(QString name);
    QHash<QString,  Dishes> categoryHash();
    const QString& category()
    { return m_category; }

    Dish& operator[](int i)
    { return m_Dishes[i]; }
    const Dish&operator[](int i) const
    { return m_Dishes[i]; }
    int indexOf(const Dish &dish)
    { return m_Dishes.indexOf(dish); }
    Dishes& operator=(QHash<QString, Dishes> dishesInCategory);//将以类别为键的Hash表转换为Dishes对象

    static QStringList categoryList;    //存储菜品类别信息
    static void saveCategory();         //保存菜品类别信息

private:
    void extractDishesVector();

    QVector<Dish> m_Dishes;     //存储菜品的数组
    int m_table;
    QString m_category;
public:
    static Dishes MenuData;                             //单App模式下存储菜单
    static QHash<QString, Dishes> MenuDataInCategory;   //单App模式下按类别存储菜单
    static QVector<Dishes> DishesData;                  //单App模式下存储每个餐桌所点菜品
};

#endif // DISH_H
