#ifndef ADMINISTRATORWIDGET_H
#define ADMINISTRATORWIDGET_H
#include "stable.h"
#include "user.h"
#include "dish.h"
#include "diningtable.h"
#include "beautify.h"
#include "WinMessage.h"

class menuManage;
class usersManage;
class tableManage;

class AdministratorWidget : public QWidget    //管理员主界面
{
    Q_OBJECT

public:
    AdministratorWidget(SignWidget *parent);
    ~AdministratorWidget(){}
    static void showList(QTableWidget *table, QVector<QStringList> &content); //将content中的内容写入表格table中

    //实现全选框的三态效果,在菜品和用户类别选择时使用
    static void onCheckChange(QVector<QCheckBox *> checkBoxes, QCheckBox *selectAll, QCheckBox *sender);

    static bool userChanged;    //记录用户信息是否改动
    static bool menuChanged;    //记录菜品信息是否改动
    static bool tableChanged;   //记录餐桌信息是否改动

private:

    SignWidget *m_parent;

    //菜单、用户、餐桌管理界面
    menuManage *m_menuMode;
    usersManage *m_userMode;
    tableManage *m_tableMode;

    WMSender wmsender;         //Windows消息发送器

    //界面拖动、界面绘制、接收信息、界面关闭
    void mousePressEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);
    bool nativeEvent(const QByteArray &eventType, void *message, long *result);
    void closeEvent(QCloseEvent *event);
};

class menuManage : public QWidget     //菜单管理界面
{
    Q_OBJECT

public:
    menuManage();
    ~menuManage(){}

private:
    QLabel *m_vaccant;      //显示菜单为空信息
    QLabel *m_errorMsg;     //添加和编辑菜品时显示错误信息
    QTableWidget *m_menuTable;

    //菜品类别管理
    QComboBox *m_category;                  //菜品类别下拉框
    QLineEdit *m_categoryMsg;               //菜品类别提示信息
    QListWidget *m_categoryList;            //菜品类别列表
    QVector<QCheckBox*> m_checkCategory;    //菜品类别选择
    QCheckBox *m_allCategory;               //菜品类别全选
    QVector<Qt::CheckState> m_checkStates;  //保存菜品类别的选择状态
    QPushButton *m_categoryManage;

    QDialog *m_categoryManageDialog;
    QTableWidget *m_categoryTable;
    QPushButton *m_addCategory;             //添加菜品类别
    QPushButton *m_editCategory;            //编辑菜品类别
    QPushButton *m_removeCategory;          //移除菜品类别
    QPushButton *m_moveCategory;            //移动菜品类别
    QPushButton *m_clearCategory;           //清空菜品类别

    //菜品增、删、改、查、清空控件
    QPushButton *m_add;
    QPushButton *m_delete;
    QPushButton *m_edit;
    QLineEdit *m_search;
    QLabel *m_searchMsg;
    QPushButton *m_clear;

    //添加或编辑菜品的类别选择、输入框、提交按钮
    QComboBox *m_categoryBox;
    QLineEdit *m_nameInput;
    QLineEdit *m_priceInput;
    QPushButton *m_submit;

    Dishes m_Dishes;                            //全部菜品
    Dishes m_chosenDishes;                      //选中类别的菜品
    Dishes m_searchDishes;                      //菜品搜索结果
    QHash<QString,Dishes> m_DishesInCategory;   //全部菜品(按类别以Hash表形式存储)

private slots:
    //菜品类别管理
    void onCategoryManage();
    void onCategoryEdit();
    void onCategoryAdd();
    void onCategoryRemove();
    void onCategoryMove();
    void onCategoryClear();

    void categoryUpdate(bool initialize);   //更新菜品类别,initialize控制是否初次更新

    //菜品增、删、改、查操作
    void onMenuAdd();
    void onMenuDelete();
    void onMenuClear();
    void onMenuEdit();
    void onMenuSearch(QString search);

    void checkMenuValid(bool editing = false, const Dish &dish= Dish());    //检查菜品信息是否有效(增加和编辑时调用)
    void menuUpdate();

    friend class AdministratorWidget;
};

class usersManage : public QWidget    //用户管理界面
{
    Q_OBJECT

public:
    usersManage();
    ~usersManage(){}

private:
    QLabel *m_vaccant;      //显示用户列表为空信息
    QLabel *m_errorMsg;     //添加和编辑用户时显示错误信息
    QTableWidget *m_usersTable;

    QVector<QCheckBox*> m_checkType;    //用户类别选择
    QCheckBox *m_allType;               //用户类别全选
    QGroupBox *m_typeGroupBox;          //用户类别分组框
    QVector<int> m_chosenTypes;         //已选择的类别

    //用户增、删、改、查、清空控件
    QPushButton *m_add;
    QPushButton *m_delete;
    QPushButton *m_edit;
    QLabel *m_searchMsg;
    QPushButton *m_clear;

    //添加或编辑用户的类别选择、输入框、提交按钮
    QComboBox *m_typeBox;
    QLineEdit *m_phonenumber;
    QLineEdit *m_password;
    QLineEdit *m_search;
    QPushButton *m_submit;

    Users m_Users[5];           //全部用户
    Users m_chosenUsers;        //选中类别的用户
    Users m_searchUsers;        //用户搜索结果
private slots:
    //用户增、删、改、查操作
    void onUsersAdd();
    void onUsersDelete();
    void onUsersClear();
    void onUsersEdit();
    void onUsersSearch(QString search);

    void checkUserValid(bool editing=false,User *user=NULL);    //检查用户信息是否有效(添加和编辑时调用)
    void usersUpdate();

    friend class AdministratorWidget;
};

class tableManage : public QWidget    //餐桌管理界面
{
    Q_OBJECT
public:
    tableManage();
    ~tableManage(){}
private:
    QVector<QCheckBox*> m_checkTables;   //用于编辑餐桌状态的复选框
    QLabel *m_tableNumber;               //显示餐桌数目
    QTableWidget *m_table;               //餐桌列表
    DiningTables m_diningTables;         //全部餐桌
private slots:
    //编辑餐桌数目与状态
    void onEdit();
    void onStateChange();

    int tableListUpdate();

    friend class AdministratorWidget;
};
#endif // ADMINISTRATORWIDGET_H
