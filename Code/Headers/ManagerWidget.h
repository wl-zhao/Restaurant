#ifndef MANAGERWIDGET_H
#define MANAGERWIDGET_H

#include "stable.h"
#include "ExportExcel.h"
#include "beautify.h"
#include "WinMessage.h"

class CookTable;
class WaiterTable;
class DishTable;

//经理界面
class ManagerWidget : public QWidget
{
    Q_OBJECT

public:
    ManagerWidget();
    ~ManagerWidget(){}

    //界面拖动、界面绘制、接收信息
    void mousePressEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);    
    bool nativeEvent(const QByteArray &eventType, void *message, long *result);

private:
    QTabWidget* m_tab;

    CookTable*m_cook;
    WaiterTable*m_waiter;
    DishTable*m_dish;

private slots:
    void onExport();
};

/*
 * 抽象类, 由一个表格和一个导出Excel按钮构成
 * 厨师工作数据、服务员工作数据、菜品销售数据由Table类继承而来
 */
class Table : public QWidget
{
    Q_OBJECT
public:
    Table(ManagerWidget *parent=0):QWidget(), m_parent(parent)
    {

    }
    virtual ~Table(){}
    QVector<QStringList> m_content;
    QStringList m_HeaderLabel;
protected:
    ManagerWidget *m_parent;
    QTableWidget *m_table;      //数据表格
    QPushButton *m_ExportExcel; //导出按钮
    void setUp();
    virtual QVector<QStringList> getContent() = 0;  //表格内容
    virtual QStringList getHeaderLabel() = 0;       //表头内容
};

//厨师工作数据
class CookTable : public Table
{
    Q_OBJECT
public:
    CookTable(ManagerWidget *parent=0);
    ~CookTable(){}
protected:
    QVector<QStringList> getContent();
    QStringList getHeaderLabel();
};

//服务员工作数据
class WaiterTable : public Table
{
    Q_OBJECT
public:
    WaiterTable(ManagerWidget *parent=0);
    ~WaiterTable(){}
private:
    QVector<QStringList> getContent();
    QStringList getHeaderLabel();
};

//菜品销售数据
class DishTable: public Table
{
    Q_OBJECT
public:
    DishTable(ManagerWidget *parent=0);
    ~DishTable(){}
private:
    QVector<QStringList> getContent();
    QStringList getHeaderLabel();
};
#endif // MANAGERWIDGET_H
