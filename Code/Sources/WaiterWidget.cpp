#include "WaiterWidget.h"

WaiterWidget::WaiterWidget(Waiter *waiter) : QWidget(), m_waiter(waiter)
{
    //界面基本属性
    setWindowFlags(Qt::FramelessWindowHint | windowFlags());
    setAttribute(Qt::WA_TranslucentBackground);
    auto titleBar = new TitleBar(this);
    installEventFilter(titleBar);
    setWindowTitle("服务员界面");
    setWindowIcon(QIcon(":/icon.ico"));
    setFixedSize(500, 500);

    //添加子界面
    m_tab = new QTabWidget(this);
    m_waiting = new Waiting(this);
    m_serving = new Serving(this);
    m_mytables = new Mytables(this);
    m_tab->addTab(m_waiting, "认领餐桌");
    m_tab->addTab(m_serving, "顾客服务");
    m_tab->addTab(m_mytables, "我的餐桌");

    //界面布局
    auto layout = new QVBoxLayout(this);
    layout->addWidget(titleBar);
    layout->addWidget(m_tab);
    layout->setContentsMargins(5, 5, 5, 5);

    mqsender = WMSender(this);
}

void WaiterWidget::mousePressEvent(QMouseEvent *event)
{
    dragEffect(this, event);//界面拖动效果
}

void WaiterWidget::paintEvent(QPaintEvent *event)
{
    paintEffect(this, event);//绘制圆角、阴影、背景
}

bool WaiterWidget::nativeEvent(const QByteArray &eventType, void *message, long *result)//接收消息
{
    QString msg = WMSender::msgProcess(eventType, message, result);
    if (msg == "update")//更新
    {
        m_waiting->update();
        m_serving->update();
        m_mytables->update();
    }
    else if (msg == "new")//新用户登录, 更新接收者列表
    {
        mqsender.updateReceivers();
    }
    else if (msg == "close")//关闭界面
    {
        close();
    }
    return QWidget::nativeEvent(eventType, message, result);
}

Waiting::Waiting(WaiterWidget *parent) : m_parent(parent)
{
    m_id = m_parent->getWaiter()->id();

    //界面控件的添加和设置
    auto label1 = new QLabel("待认领餐桌列表");
    m_waitingTablesList = new QTableWidget;
    m_waitingTablesList->setColumnCount(2);
    m_waitingTablesList->horizontalHeader()->setVisible(false);
    m_waitingTablesList->verticalHeader()->setVisible(false);
    m_waitingTablesList->setEditTriggers(QHeaderView::NoEditTriggers);
    m_waitingTablesList->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    //界面布局
    auto Layout = new QVBoxLayout(this);
    Layout->addWidget(label1);
    Layout->addWidget(m_waitingTablesList);

    update();
}

//更新待认领餐桌
void Waiting::update()
{
    m_waitingTables = DiningTables::extractWaitingTables();     //提取待认领餐桌

    m_claim.clear();
    m_waitingTablesList->setRowCount(m_waitingTables.count());

    for (int i=0; i<m_waitingTables.count(); ++i)
    {
        m_waitingTablesList->setItem(i, 0, new QTableWidgetItem(QString("%1号餐桌").arg(m_waitingTables[i].id())));
        auto pushButton = new QPushButton("认领餐桌");
        pushButton->setStyleSheet("QPushButton{border-radius:0;}");
        m_claim.append(pushButton);
        m_waitingTablesList->setCellWidget(i, 1, pushButton);
        connect(pushButton, SIGNAL(clicked(bool)), this, SLOT(onClaim()));
    }
}

//认领餐桌
void Waiting::onClaim()
{
    auto Sender = qobject_cast<QPushButton*>(sender());
    if (Sender)
    {
        //对应餐桌保存服务员编号
        int index = m_claim.indexOf(Sender);
        auto table = m_waitingTables[index];
        table.setWaiterId(m_id);
        table.save('w');

        m_servingTables.add(table);

        update();
        m_parent->m_serving->update();
        m_parent->m_mytables->update();
    }
    m_parent->mqsender.send("waiter", "update");        //给其它服务员发送消息以更新餐桌数据
}

//服务界面
Serving::Serving(WaiterWidget *parent):m_parent(parent)
{
    m_id = m_parent->getWaiter()->id();

    //界面控件的添加和设置
    auto label1 = new QLabel("请处理顾客信息");
    m_messageList = new QTableWidget;
    m_messageList->setColumnCount(3);
    m_messageList->horizontalHeader()->setVisible(false);
    m_messageList->verticalHeader()->setVisible(false);
    m_messageList->setEditTriggers(QHeaderView::NoEditTriggers);
    m_messageList->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_messageList->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);

    //界面布局
    auto Layout = new QVBoxLayout(this);
    Layout->addWidget(label1);
    Layout->addWidget(m_messageList);

    update();
}

//服务界面更新
void Serving::update()
{
    m_solved.clear();
    m_servingTables = DiningTables::extractServingTables(m_id);     //获取本服务员服务的所有餐桌

    for (int i=0; i<m_servingTables.count(); ++i)                   //遍历更新消息
    {
        QString temp = m_servingTables[i].message();
        Message message(m_servingTables[i].id(), temp);
        if ((temp.count()!=0) && (!m_messages.contains(message)))   //舍去重复消息
        {
            m_messages.append(message);
        }
    }

    m_messageList->setRowCount(0);
    m_messageList->setRowCount(m_messages.count());

    for (int i = 0;i<m_messages.count();++i)                        //将消息显示在列表中
    {
        m_messageList->setItem(i, 0, new QTableWidgetItem(QString("%1号餐桌       ").arg(m_messages[i].tableId())));
        int length = m_messages[i].msg().length();
        if (m_messages[i].msg()[length-1] == ' ')//如果最后一位为空格(代表由厨师发来的菜品就绪的消息)
        {
            QString content = m_messages[i].msg().mid(0, m_messages[i].msg().indexOf("  "))+"已完成, 请上菜";//获取显示内容
            //将内容用红色显示以区分顾客消息
            auto label = new QLabel(QString("%1%2%3")\
                                    .arg("<font color = red>")\
                                    .arg(content).arg("</font>"), this);
            label->setAlignment(Qt::AlignVCenter|Qt::AlignLeft);
            label->setContentsMargins(3, 8, 5, 5);
            m_messageList->setCellWidget(i, 1, label);
        }//end of if
        else//消息来自顾客
        {
            m_messageList->setItem(i, 1, new QTableWidgetItem(m_messages[i].msg()));
        }

        auto pushButton = new QPushButton("已解决");
        pushButton->setStyleSheet("QPushButton{border-radius:0;}");
        m_messageList->setCellWidget(i, 2, pushButton);
        m_solved.append(pushButton);
        connect(pushButton, SIGNAL(clicked(bool)), this, SLOT(onSolve()));

        //唯一标识消息的键, 由餐桌号和消息内容构成
        QString Key = QString("%1, %2")\
                .arg(m_messages[i].tableId()).arg(m_messages[i].msg());
        //为该任务绑定计时器
        if (!m_time.contains(Key))
        {
            m_time[Key] = QTime::currentTime();
        }
    }//end of for
}

//解决消息
void Serving::onSolve()
{
    Waiter *waiter = m_parent->m_waiter;
    auto Sender = qobject_cast<QPushButton*>(sender());
    if (Sender)
    {
        int index = m_solved.indexOf(Sender);
        if (index >= 0)
        {
            //获取消息
            QString msg = m_messages[index].msg();

            //对应餐桌消息置空
            auto table = DiningTable::extractTable(m_messages[index].tableId());
            table.setMessage("");
            table.save('m');

            if ((msg == "结账") && (table.tableState() == "结账中"))//如果是结账消息
            {
                //保存结账状态, 并发消息给顾客使其更新状态
                table.setState("已结账");
                table.save('s');
                m_parent->mqsender.send("customer", "update");
            }
            else if (msg[msg.count()-1] == ' ')//如果最后一位为空格(厨师发来的消息)
            {
                //提取餐桌菜品
                Dishes dishes(table.id());
                //解码厨师消息获得菜品名称和编号
                int index = msg.indexOf("  ");
                QString name = msg.mid(0, index);
                int id = msg.mid(index+2, msg.length()).toInt();//

                for (int i=0; i<dishes.count(); ++i)
                {
                    if ((dishes[i].name() == name)\
                            && (dishes[i].dishState()== "已完成")\
                            && (dishes[i].id() == id))              //如果找到对应菜品
                    {
                        //更新菜品状态, 并发消息给顾客使其更新状态
                        dishes[i].setState("已上菜");
                        dishes[i].save('s');
                        m_parent->mqsender.send("customer", "update");
                        break;
                    }//end of if
                }//end of for
            }

            //获取//唯一标识消息的键, 由餐桌号和消息内容构成
            QString Key = QString("%1, %2").arg(m_messages[index].tableId()).arg(m_messages[index].msg());

            //更新服务员工作数据
            if (m_time.contains(Key))
            {
                int interval = m_time[Key].secsTo(QTime::currentTime());
                waiter->setTimeCount(waiter->timeCount()+interval);
                m_time.remove(Key);
            }
            waiter->setTaskCount(waiter->taskCount()+1);
            waiter->saveWaiter();

            m_messages.removeAt(index);
            if (m_messages.count() == 0)
            {
                m_messageList->setRowCount(0);
            }

            update();
        }//end of if
    }//end of if
}

//我的餐桌界面
Mytables::Mytables(WaiterWidget *parent):m_parent(parent)
{
    m_id = m_parent->getWaiter()->id();

    //界面控件与布局
    m_servingTablesList = new QListWidget(this);
    auto label = new QLabel("已认领餐桌列表", this);
    auto Layout = new QVBoxLayout(this);
    Layout->addWidget(label);
    Layout->addWidget(m_servingTablesList);

    update();
}

//更新本服务员服务的餐桌列表
void Mytables::update()
{
    m_servingTables = DiningTables::extractServingTables(m_id);
    m_servingTablesList->clear();
    for (int i=0; i<m_servingTables.count(); ++i)
    {
        m_servingTablesList->addItem(QString("%1号餐桌").arg(m_servingTables[i].id()));
    }
}
