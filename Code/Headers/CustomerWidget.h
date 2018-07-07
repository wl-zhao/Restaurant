#ifndef CUSTOMERWIDGET_H
#define CUSTOMERWIDGET_H
#include "dish.h"
#include "diningtable.h"
#include "user.h"
#include "WinMessage.h"
#include "beautify.h"

class tableSelect;
class dishSelect;
class eating;
class pay;
class score;

//顾客主界面
class CustomerWidget : public QWidget
{
    Q_OBJECT
public:
    CustomerWidget(Customer *customer = 0);
    ~CustomerWidget(){}

    //界面拖动、界面绘制、接收消息、界面关闭
    void mousePressEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);
    bool nativeEvent(const QByteArray &eventType, void *message, long *result);
    void closeEvent(QCloseEvent *event);

    void setFixedIndex(int Index);              //设置固定的选项卡序号
    void setTableId(int tableId)
    { m_tableId=tableId; }
    const int& tableId() const
    { return m_tableId; }
    Customer* getCustomer() const
    { return m_customer; }
    DiningTable getTable();
    WMSender wmsender;                          //Windows消息发送器

private:
    QTabWidget *m_tab;                          //控制用餐流程的选项卡界面
    Customer *m_customer;                       //该顾客的指针

    //用餐流程的子界面
    tableSelect *m_tableSelect;
    dishSelect *m_dishSelect;
    eating *m_eating;
    pay *m_pay;
    score *m_score;

    int m_tableId;

    friend class tableSelect;
    friend class dishSelect;
    friend class eating;
    friend class pay;
};

//选桌界面
class tableSelect : public QWidget
{
    Q_OBJECT

public:
    tableSelect(CustomerWidget* parent = 0);
    ~tableSelect(){}
    void setUp();

private:
    CustomerWidget *m_parent;

    QTableWidget *m_tableList;          //餐桌列表
    QButtonGroup *m_tablesGroup;        //选桌按钮
    QLabel *m_myTable;                  //显示顾客已选餐桌信息
    QLabel *m_currentTableLabel;        //显示当前餐桌提示信息
    QPushButton *m_submit;
    DiningTables m_tables;

private slots:
    void onSubmitTable();               //提交选定餐桌
    void onTableChanged();              //选定餐桌变化时触发

public slots:
    void tableUpdate();                 //更新餐桌数据
};

//点餐界面
class dishSelect : public QWidget
{
    Q_OBJECT

public:
    dishSelect(CustomerWidget *parent = 0);
    ~dishSelect(){}
    void setUp();

private:
    CustomerWidget *m_parent;
    Dishes m_Menu;                  //菜单
    Dishes m_Tray;                  //我的托盘

    QTableWidget *m_menu;           //菜单(表格)
    QTableWidget *m_tray;           //我的托盘(表格)
    QVector<QCheckBox*> m_dishes;   //点菜复选框
    QPushButton *m_submit;

protected slots:
    void onTrayChange();            //托盘改变
    void onSubmitDishes();          //提交菜品
    void onCountEdit();             //编辑数量
    friend class CustomerWidget;
};

//用餐界面
class eating : public QWidget
{
    Q_OBJECT

public:
    eating(CustomerWidget *parent = 0);
    ~eating(){}
    void setUp();

private:
    CustomerWidget* m_parent;

    QTableWidget* m_dishesTable;    //已点菜品列表
    QLineEdit* m_message;           //消息框
    QPushButton *m_submit;          //提交按钮
    QPushButton *m_pay;             //支付按钮, 跳转到支付界面
    Dishes m_tray;                  //我的托盘

public slots:
    void update();

protected slots:
    void onSend();                  //发送消息
};

//支付界面
class pay : public QWidget
{
    Q_OBJECT

public:
    pay(CustomerWidget *parent = 0);
    ~pay(){}
    void setUp();
private:
    CustomerWidget *m_parent;

    QTableWidget *m_dishesTable;     //菜品列表

protected slots:
    void onWechat();
    void onCash();

public slots:
    void CheckPayState();       //检查支付状态
};

//评分界面
class score : public QWidget
{
    Q_OBJECT

public:
    score(CustomerWidget *parent = 0);
    ~score(){}
    void setUp();

    bool m_scored;

private:
    CustomerWidget *m_parent;
    Customer *m_customer;

    QTableWidget *m_scoreTable;         //评分表格
    QVector<QSpinBox*> m_dishScores;    //菜品评分
    QSpinBox *m_waiterScore;            //服务员评分
    Dishes m_dishes;                    //已点菜品

public slots:
    void onSubmit();
};

#endif // CUSTOMERWIDGET_H
