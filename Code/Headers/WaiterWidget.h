#ifndef WAITERWIDGET_H
#define WAITERWIDGET_H
#include "stable.h"
#include "user.h"
#include "diningtable.h"
#include "beautify.h"
#include "WinMessage.h"

//消息类
class Message
{
public:
    Message(int tableId,QString msg) : m_tableId(tableId), m_msg(msg)
    {}
    Message(){}
    ~Message(){}
    int tableId() const
    { return m_tableId; }
    QString msg() const
    { return m_msg; }
    bool operator==(const Message &message) const
    { return (m_tableId==message.tableId())&&(m_msg==message.msg()); }

private:
    int m_tableId;
    QString m_msg;
};

class Waiting;
class Serving;
class Mytables;

//服务员主界面
class WaiterWidget : public QWidget
{
    Q_OBJECT

public:
    WaiterWidget(Waiter *waiter = 0);
    ~WaiterWidget(){}

    //界面拖动、界面绘制、接收消息
    void mousePressEvent(QMouseEvent*event);
    void paintEvent(QPaintEvent*event);
    bool nativeEvent(const QByteArray &eventType, void *message, long *result);

    Waiter* getWaiter() const         //返回服务员编号
    { return m_waiter; }
    Waiting *m_waiting;
    Serving *m_serving;
    Mytables *m_mytables;
private:
    int m_id;

    Waiter *m_waiter;
    QTabWidget *m_tab;
    WMSender mqsender;      //Windows消息发送器

    friend class Waiting;
    friend class Serving;
};

//认领餐桌界面
class Waiting : public QWidget
{
    Q_OBJECT

public:
    Waiting(WaiterWidget *parent);
    ~Waiting(){}

    DiningTables m_servingTables;       //本服务员服务的餐桌
private:
    int m_id;

    WaiterWidget *m_parent;             //主界面作为父对象
    QTableWidget *m_waitingTablesList;  //待认领餐桌列表
    QVector<QPushButton*> m_claim;
    DiningTables m_waitingTables;

public slots:
    void onClaim();                     //认领餐桌
    void update();                      //更新待认领餐桌列表
};

class Serving : public QWidget
{
    Q_OBJECT
public:
    Serving(WaiterWidget*parent);
    ~Serving(){}
private:
    int m_id;

    WaiterWidget *m_parent;             //主界面作为父对象
    QTableWidget *m_messageList;        //消息列表
    QVector<QPushButton*> m_solved;     //解决按钮
    DiningTables m_servingTables;       //本服务员服务的餐桌
    QVector<Message> m_messages;        //全部消息
    QHash<QString,QTime> m_time;        //记录处理消息的时间

public slots:
    void onSolve();                     //处理消息
    void update();                      //更新消息列表
};

/*
 * 我的餐桌界面
 * 显示本服务员服务的餐桌
 */
class Mytables : public QWidget
{
    Q_OBJECT

public:
    Mytables(WaiterWidget *parent);
    ~Mytables(){}
    void update();
private:
    int m_id;

    WaiterWidget *m_parent;
    QListWidget *m_servingTablesList;        //本服务员服务的餐桌列表
    DiningTables m_servingTables;           //本服务员服务的餐桌
};
#endif // WAITERWIDGET_H
