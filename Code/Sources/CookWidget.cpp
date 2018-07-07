#include "CookWidget.h"

CookWidget::CookWidget(Cook *cook):QWidget(), m_cook(cook)
{
    //设置界面基本属性
    auto titleBar = new TitleBar(this);
    installEventFilter(titleBar);
    setWindowFlags(Qt::FramelessWindowHint | windowFlags());
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowTitle("厨师界面");
    setFixedSize(400, 500);
    setWindowIcon(QIcon(":/icon.ico"));

    //界面控件的添加和设置
    auto label = new QLabel("请认领做菜任务");
    m_dishesTable = new QTableWidget;
    m_dishesTable->setColumnCount(3);
    m_dishesTable->horizontalHeader()->setVisible(false);
    m_dishesTable->verticalHeader()->setVisible(false);
    m_dishesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    //界面布局
    auto Layout = new QVBoxLayout(this);
    auto hLayout = new QHBoxLayout;
    Layout->addWidget(titleBar);
    hLayout->setContentsMargins(10, 10, 10, 10);
    hLayout->addWidget(label);
    Layout->addLayout(hLayout);
    hLayout = new QHBoxLayout;
    hLayout->addWidget(m_dishesTable);
    Layout->addLayout(hLayout);
    Layout->setContentsMargins(5, 5, 5, 5);

    extract();                  //提取未完成菜品
    update();                   //更新任务列表
    wmsender = WMSender(this);
}

void CookWidget::mousePressEvent(QMouseEvent *event)
{
    dragEffect(this, event);//界面拖动效果
}

bool CookWidget::nativeEvent(const QByteArray &eventType, void *message, long *result)//接收消息
{
    QString msg = WMSender::msgProcess(eventType, message, result);
    if (msg == "update")//更新
    {
        extract();
        update();
    }
    else if (msg == "new")//新用户登录, 更新接收者列表
    {
        wmsender.updateReceivers();
    }
    else if (msg == "close")//关闭界面
    {
        close();
    }
    return QWidget::nativeEvent(eventType, message, result);
}

void CookWidget::paintEvent(QPaintEvent *event)
{
    paintEffect(this, event);//绘制圆角、阴影、背景
}

/* 提取未完成的菜品
 * 包括未认领的菜品和自己已认领但未完成的菜品
 */
void CookWidget::extract()
{
    m_unfinishedDishes = Dishes::extractUnfinishedDishes(m_cook->id());
}

//任务列表更新
void CookWidget::update()
{
    m_cookButtons.clear();
    m_dishesTable->setRowCount(m_unfinishedDishes.count());
    for (int i=0; i<m_unfinishedDishes.count(); ++i)
    {
        m_dishesTable->setItem(i, 0, new QTableWidgetItem(QString("%1号餐桌").arg(m_unfinishedDishes[i].table())));
        m_dishesTable->setItem(i, 1, new QTableWidgetItem(m_unfinishedDishes[i].name()));
        auto pushButton = new QPushButton("认领任务");
        pushButton->setStyleSheet("QPushButton{border-radius:0;}");
        if (m_unfinishedDishes[i].dishState() == "制作中")
        {
            pushButton->setText("已完成");
        }
        m_dishesTable->setCellWidget(i, 2, pushButton);
        m_cookButtons.append(pushButton);

        connect(pushButton, SIGNAL(clicked(bool)), this, SLOT(onClicked()));
    }
}

//认领或完成任务
void CookWidget::onClicked()
{
    auto Sender = qobject_cast<QPushButton*>(sender());
    if (Sender)
    {
        int index = m_cookButtons.indexOf(Sender);
        if (index >= 0)
        {
            //唯一标识菜品的键, 由餐桌号和菜品序号构成
            QString Key = QString("%1, %2").arg(m_unfinishedDishes[index].table())\
                    .arg(m_unfinishedDishes[index].id());

            if (Sender->text() == "认领任务")
            {
                m_unfinishedDishes[index].setState("制作中");
                m_unfinishedDishes[index].save('s');        //保存菜品状态
                m_unfinishedDishes[index].setCookId(m_cook->id());
                m_unfinishedDishes[index].save('c');        //保存厨师编号
                m_time[Key] = QTime::currentTime();         //为此任务绑定计时器
                update();
                wmsender.send("cook", "update");            //发送消息使其它厨师更新任务列表
            }
            else if (Sender->text() == "已完成")
            {
                m_unfinishedDishes[index].setState("已完成");
                Dish &dish = m_unfinishedDishes[index];
                m_unfinishedDishes[index].save('s');        //保存餐桌状态

                /* 发送上菜信息
                 * 将菜品名称和序号写入餐桌消息
                 * 结尾添加空格以便和顾客的消息区分
                 */
                DiningTable table = DiningTable::extractTable(m_unfinishedDishes[index].table());
                table.setMessage(QString("%1  %2 ").\
                                 arg(dish.name()).arg(dish.id()));
                table.save('m');

                //更新厨师工作数据
                m_unfinishedDishes.remove(index);
                if (m_time.contains(Key))
                {
                    int interval = m_time[Key].secsTo(QTime::currentTime());
                    m_cook->setTimeCount(interval+m_cook->timeCount());
                    m_time.remove(Key);
                }
                m_cook->setTaskCount(1+m_cook->taskCount());
                m_cook->saveCook();
                update();
                wmsender.send("waiter", "update");      //发送消息给服务员, 更新消息列表, 接收菜品已就绪的信息
            }
        }//end of if
    }//end of if
    wmsender.send("customer", "update");                //发送消息给顾客, 更新菜品状态信息
}
