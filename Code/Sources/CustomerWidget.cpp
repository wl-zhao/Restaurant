#include "CustomerWidget.h"

CustomerWidget::CustomerWidget(Customer *customer) : QWidget(), m_customer(customer)
{
    //设置主界面基本属性
    setWindowFlags(Qt::FramelessWindowHint | windowFlags());
    setAttribute(Qt::WA_TranslucentBackground);
    auto titleBar = new TitleBar(this);
    installEventFilter(titleBar);
    setWindowTitle("顾客界面");
    setWindowIcon(QIcon(":/icon.ico"));
    setFixedSize(750, 450);

    auto layout = new QVBoxLayout(this);
    layout->addWidget(titleBar);

    Dishes menu(0);//提取菜单
    if (menu.count() == 0)//如果菜单为空
    {
        //显示菜单为空信息, 顾客不能进行任何操作
        m_tab = NULL;
        auto label1 = new QLabel(this);
        auto label2 = new QLabel(this);
        QFont ft;
        ft.setPointSize(15);
        label1->setText("很抱歉, 本店菜单暂时为空");
        label1->setAlignment(Qt::AlignHCenter);
        label1->setFont(ft);
        label2->setText("请联系本店管理员更新菜单");
        label2->setAlignment(Qt::AlignHCenter);
        label2->setFont(ft);
        layout->addSpacing(50);
        layout->addWidget(label1);
        layout->addWidget(label2);
    }
    else//菜单不为空
    {
        //添加各子界面
        m_tab = new QTabWidget(this);
        m_tableSelect = new tableSelect(this);
        m_dishSelect = new dishSelect(this);
        m_eating = new eating(this);
        m_pay = new pay(this);
        m_score = new score(this);
        m_tab->addTab(m_tableSelect, "餐桌选择");
        m_tab->addTab(m_dishSelect, "开始点餐");
        m_tab->addTab(m_eating, "开始用餐");
        m_tab->addTab(m_pay, "结账");
        m_tab->addTab(m_score, "评价");
        m_tableId = m_customer->tableid();//获取顾客的餐桌编号
        if (m_tableId != 0)//如果餐桌编号不为0(表示该顾客在上次登录已经选桌完毕)
        {
            setTableId(m_customer->tableid());
            DiningTable table = DiningTable::extractTable(m_customer->tableid());
            //根据餐桌状态确定显示界面
            if (table.tableState() == "空闲")//正常操作一般不会出现，只有当该顾客选桌后管理员强制修改餐桌数量才会发生该情况
            {
                m_customer->setTableId(0);
                setFixedIndex(0);
            }
            else if (table.tableState() == "点菜中")
            {
                m_dishSelect->setUp();
                setFixedIndex(1);
            }
            else if (table.tableState() == "已下单")
            {
                m_eating->setUp();
                setFixedIndex(2);
            }
            else if (table.tableState() == "结账中")
            {
                m_pay->setUp();
                setFixedIndex(3);
            }
            else if (table.tableState() == "已结账")
            {
                m_score->setUp();
                setFixedIndex(4);
            }
        }//end of if

        layout->addWidget(m_tab);
    }

    layout->setContentsMargins(5, 5, 5, 5);
    wmsender = WMSender(this);
}

void CustomerWidget::mousePressEvent(QMouseEvent *event)
{
    dragEffect(this, event);//界面拖动效果
}

void CustomerWidget::paintEvent(QPaintEvent *event)
{
    paintEffect(this, event);//绘制圆角、阴影、背景
}

bool CustomerWidget::nativeEvent(const QByteArray &eventType, void *message, long *result)//接受消息
{
    QString msg = WMSender::msgProcess(eventType, message, result);
    if (msg == "update")//更新数据
    {
        if (m_tab != NULL)
        {
            switch (m_tab->currentIndex())
            {
            case 0:
                m_tableSelect->tableUpdate();   //更新餐桌数据
                break;
            case 2:
                m_eating->update();             //更新菜品状态
                break;
            case 3:
                m_pay->CheckPayState();         //检测支付状态
                break;
            default:
                break;
            }
        }
        else
        {
            auto customerWidget = new CustomerWidget(m_customer);
            this->close();
            customerWidget->show();
        }
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

void CustomerWidget::closeEvent(QCloseEvent *event)
{
    if (m_tab && (m_tab->currentIndex()==4) && (!m_score->m_scored))//如果退出时已经处于评价界面, 且尚未评价
    {
        m_score->onSubmit();//自动评价
    }
}

/*
 * 固定选项卡的下标
 * 用于保证顾客点餐流程的顺序
 */
void CustomerWidget::setFixedIndex(int Index)
{
    disconnect(m_tab, SIGNAL(currentChanged(int)), 0, 0);
    connect(m_tab, &QTabWidget::currentChanged, [=](){m_tab->setCurrentIndex(Index);});
    m_tab->setCurrentIndex(Index);
}

//获取已选择的餐桌
DiningTable CustomerWidget::getTable()
{
    DiningTables tables;
    return tables[m_tableId-1];
}

//选桌界面
tableSelect::tableSelect(CustomerWidget* parent):m_parent(parent)
{
    setUp();
    m_parent->setFixedIndex(0);
}

void tableSelect::setUp()
{
    auto oldLayout = this->layout();
    if (oldLayout != NULL)
    {
        delete oldLayout;
    }

    //界面控件添加和设置
    m_currentTableLabel = new QLabel("当前餐桌列表", this);
    m_myTable = new QLabel(this);
    m_tableList = new QTableWidget(this);
    m_submit = new QPushButton("提交", this);
    QStringList headerLabel;
    m_tableList->verticalHeader()->setVisible(false);
    m_tableList->setColumnCount(2);
    headerLabel<<"餐桌序号"<<"餐桌状态";
    m_tableList->setHorizontalHeaderLabels(headerLabel);
    m_tableList->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_tableList->setEditTriggers(QTableView::NoEditTriggers);
    m_tablesGroup = new QButtonGroup(this);

    //界面布局
    auto Layout = new QVBoxLayout(this);
    Layout->addWidget(m_currentTableLabel);
    Layout->addWidget(m_tableList);
    auto hLayout = new QHBoxLayout(this);
    hLayout->addWidget(m_myTable);
    hLayout->addStretch(1);
    Layout->addLayout(hLayout);
    Layout->addWidget(m_submit, 1, Qt::AlignRight);

    connect(m_tablesGroup, SIGNAL(buttonClicked(int)), this, SLOT(onTableChanged()));
    connect(m_submit, SIGNAL(clicked(bool)), this, SLOT(onSubmitTable()));

    tableUpdate();
}

void tableSelect::tableUpdate()
{
    m_tables = DiningTables();//提取所有餐桌信息

    m_tableList->setRowCount(m_tables.count());
    bool checkOne = false;      //是否选中了一个餐桌
    for (int i=0; i<m_tables.count(); ++i)
    {
        auto table = new QRadioButton(QString("%1号餐桌").arg(i+1), this);
        m_tableList->setCellWidget(i, 0, table);
        m_tablesGroup->addButton(table, i);
        if (m_tables[i].tableState() == "空闲")   //如果餐桌为空闲
        {
            table->setEnabled(true);             //设置为可用
            if (!checkOne)//如果仍未选中餐桌
            {
                //选中此餐桌
                table->setChecked(true);
                checkOne = true;
            }
        }
        else
        {
            table->setEnabled(false);           //否则禁用
        }
        m_tableList->setItem(i, 1, new QTableWidgetItem(m_tables[i].tableState()));
    }
    if (checkOne)//如果选中了餐桌
    {
        m_myTable->setText(QString("您已选择:%1").arg(m_tablesGroup->checkedButton()->text()));
        m_currentTableLabel->setText("当前餐桌列表");
        m_submit->setEnabled(true);
    }
    else//没有选中, 表示当前没有空闲餐桌
    {
        m_currentTableLabel->setText("很抱歉, 暂时没有可用餐桌");
        m_myTable->setText("请等待其他客人用餐结束");
        m_submit->setEnabled(false);
    }
}

//提交选定的餐桌
void tableSelect::onSubmitTable()
{
    //保存餐桌编号至顾客信息
    auto customer = m_parent->getCustomer();
    customer->setTableId(m_tablesGroup->checkedId()+1);
    customer->saveTableId();

    //主界面保存餐桌编号
    m_parent->setTableId(m_tablesGroup->checkedId()+1);

    //改变餐桌状态
    m_tables[m_tablesGroup->checkedId()].setState("点菜中");
    m_tables[m_tablesGroup->checkedId()].save('s');

    //进入点餐界面
    m_parent->m_dishSelect->setUp();
    m_parent->setFixedIndex(1);

    m_parent->wmsender.send("customer", "update");      //给顾客发消息, 更新餐桌列表
    m_parent->wmsender.send("waiter", "update");        //给服务员发消息, 更新待认领餐桌
}

//选择餐桌改变时, 改变提示信息
void tableSelect::onTableChanged()
{
    m_myTable->setText(QString("您已选择:%1").arg(m_tablesGroup->checkedButton()->text()));
}

//点菜界面
dishSelect::dishSelect(CustomerWidget* parent):m_parent(parent)
{

}

//点餐界面初始化
void dishSelect::setUp()
{
    m_Menu = Dishes(0);//提取菜品数据

    //菜单初始化
    auto label1 = new QLabel("菜单");
    label1->setAlignment(Qt::AlignBottom);
    label1->setMaximumHeight(30);
    m_menu = new QTableWidget(this);
    m_menu->setRowCount(m_Menu.count());
    m_menu->setColumnCount(5);
    m_menu->setEditTriggers(QTableView::NoEditTriggers);
    QStringList headerLabel1;
    headerLabel1<<""<<"菜名"<<"类别"<<"价格"<<"描述";
    m_menu->setHorizontalHeaderLabels(headerLabel1);

    int row = m_Menu.count();
    m_menu->setRowCount(row);
    m_menu->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_menu->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    for (int i=0; i<row; ++i)
    {
        auto dish = new QCheckBox(m_Menu[i].name(), this);
        m_dishes.append(dish);
        connect(dish, SIGNAL(clicked(bool)), this, SLOT(onTrayChange()));
        m_menu->verticalHeader()->setVisible(false);
        m_menu->setItem(i, 0, new QTableWidgetItem(QString::number(i+1)));
        m_menu->setCellWidget(i, 1, dish);
        m_menu->setItem(i, 2, new QTableWidgetItem(m_Menu[i].category()));
        m_menu->setItem(i, 3, new QTableWidgetItem(m_Menu[i].priceToString()));
        m_menu->setItem(i, 4, new QTableWidgetItem(m_Menu[i].description()));
    }

    m_menu->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_menu->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_menu->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

    //我的餐盘初始化
    auto label2 = new QLabel("我的餐盘");
    label2->setAlignment(Qt::AlignBottom);
    label2->setMaximumHeight(30);
    m_tray = new QTableWidget(this);
    m_tray->setEditTriggers(QTableView::NoEditTriggers);
    m_tray->setColumnCount(3);
    m_tray->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_tray->verticalHeader()->setVisible(false);
    QStringList headerLabel2;
    headerLabel2<<"菜名"<<"价格"<<"数量";
    m_tray->setHorizontalHeaderLabels(headerLabel2);

    m_submit = new QPushButton("提交菜单");
    m_submit->setEnabled(false);

    //界面布局
    auto Layout = new QGridLayout;
    Layout->addWidget(label1, 0, 0, 1, 4);
    Layout->addWidget(label2, 0, 4, 1, 3);
    Layout->addWidget(m_menu, 1, 0, 3, 4);
    Layout->addWidget(m_tray, 1, 4, 3, 3);
    Layout->addWidget(m_submit, 4, 6, 1, 1);
    this->setLayout(Layout);

    connect(m_submit, SIGNAL(clicked(bool)), this, SLOT(onSubmitDishes()));
    connect(m_tray, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onCountEdit()));
    connect(m_menu, &QTableWidget::cellClicked, [=] ()
    {
        int column = m_menu->currentColumn();
        int row = m_menu->currentRow();
        if (column >= 0 && column != 1)                 //如果列有效，且非复选框所在列
        {
            if (row >= 0 && row < m_dishes.count())     //如果行有效
            {
                m_dishes[row]->click();//选中对应菜品
            }
        }
    });

}

//所点菜品改变时更新餐盘
void dishSelect::onTrayChange()
{
    auto Sender = qobject_cast<QCheckBox*>(sender());
    if (Sender)
    {
        //获取用户点击的菜品
        int index = m_dishes.indexOf(Sender);
        if (index >= m_Menu.count() && index < 0)    //若下标不合法，立即返回
        {
            return;
        }

        Dish dish = m_Menu[index];
        if (m_dishes[index]->isChecked())//如果该菜品被选中
        {
            dish.setCount(1);       //默认数量为1
            m_Tray.add(dish);       //加入餐盘
        }
        else//如果该菜品未被选中
        {
            m_Tray.remove(dish);    //从餐盘中移除
        }
        int count = m_Tray.count();
        if (count != 0)             //如果餐盘不为空
        {
            //更新我的餐盘列表中的数据
            m_tray->setRowCount(count+1);
            for (int i=0; i<count; ++i)
            {
                m_tray->setItem(i, 0, new QTableWidgetItem(m_Tray[i].name()));
                m_tray->setItem(i, 1, new QTableWidgetItem(m_Tray[i].priceToString()));
                m_tray->setItem(i, 2, new QTableWidgetItem(QString::number(m_Tray[i].count())));
            }
            m_tray->setItem(count, 0, new QTableWidgetItem("合计"));
            m_tray->setItem(count, 1, new QTableWidgetItem(m_Tray.totalPrice()));
            m_tray->setItem(count, 2, new QTableWidgetItem(QString::number(m_Tray.totalCount())));
            m_submit->setEnabled(true);
        }//end of if
        else//餐盘为空
        {
            m_tray->setRowCount(0);         //设置行数为0;
            m_submit->setEnabled(false);    //提交按钮禁用
        }
    }//end of if
}

//提交菜品
void dishSelect::onSubmitDishes()
{
    for (int i=0; i<m_Tray.count(); ++i)
    {
        //更新并保存菜单中菜品的被点次数
        m_Menu[m_Tray[i].id()-1].setCount(m_Tray[i].count()+m_Menu[m_Tray[i].id()-1].count());
        m_Menu[m_Tray[i].id()-1].saveCount();

        //初始化菜品状态
        m_Tray[i].setState("等待中");
    }

    m_Tray.split();//将托盘中所点菜品拆分为以单个菜品储存的结构
    m_Tray.setTable(m_parent->tableId());   //为餐盘中的菜品设定餐桌
    m_Tray.save();

    //更新餐桌状态
    auto table = m_parent->getTable();
    table.setState("已下单");
    table.save('s');

    m_parent->wmsender.send("cook", "update");
    m_parent->m_eating->setUp();
    m_parent->setFixedIndex(2);
}

//编辑菜品数量
void dishSelect::onCountEdit()
{
    auto editDialog = new QDialog;
    editDialog->setWindowTitle("编辑数量");

    //界面控件的添加和设置
    editDialog->setFixedWidth(150);
    int row = m_tray->currentRow();
    if (row >= m_Tray.count())
    {
        return;
    }
    auto editCount = new QSpinBox(this);
    editCount->setValue(m_Tray[row].count());
    editCount->setRange(1, 1000);
    auto editSubmit = new QPushButton("确定");

    //界面布局
    auto editLayout = new QVBoxLayout(editDialog);
    editLayout->addWidget(editCount);
    editLayout->addWidget(editSubmit, Qt::AlignRight);

    connect(editSubmit, SIGNAL(clicked(bool)), editDialog, SLOT(accept()));

    if (editDialog->exec() == QDialog::Accepted)//如果修改成功
    {
        m_Tray[row].setCount(editCount->value());//修改餐盘中对应菜品的数量

        //修改列表中菜品数量、价格等信息
        m_tray->setItem(row, 2, new QTableWidgetItem(editCount->text()));
        m_tray->setItem(m_Tray.count(), 1, new QTableWidgetItem(m_Tray.totalPrice()));
        m_tray->setItem(m_Tray.count(), 2, new QTableWidgetItem(QString::number(m_Tray.totalCount())));
    }
}

//用餐界面
eating::eating(CustomerWidget *parent):m_parent(parent)
{

}

//用餐界面初始化
void eating::setUp()
{
    //界面基本控件的添加和设置
    auto label = new QLabel("已点菜品列表:", this);
    label->setMaximumHeight(30);
    label->setAlignment(Qt::AlignHCenter|Qt::AlignBottom);
    m_message = new QLineEdit;
    m_message->setPlaceholderText("向服务员发送消息");
    m_submit = new QPushButton("发送");
    m_submit->setEnabled(false);
    m_pay = new QPushButton("去结帐");

    //菜品列表
    m_dishesTable = new QTableWidget(this);
    m_dishesTable->setColumnCount(5);
    QStringList headerLabel;
    headerLabel<<"序号"<<"菜名"<<"类别"<<"价格"<<"状态";
    m_dishesTable->setHorizontalHeaderLabels(headerLabel);
    m_dishesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_dishesTable->verticalHeader()->setVisible(false);
    m_dishesTable->setEditTriggers(QTableView::NoEditTriggers);

    //界面布局
    auto Layout = new QVBoxLayout(this);
    auto hLayout = new QHBoxLayout;
    Layout->addWidget(label, Qt::AlignHCenter);
    hLayout = new QHBoxLayout;
    hLayout->addWidget(m_dishesTable, Qt::AlignHCenter);
    Layout->addLayout(hLayout);
    hLayout = new QHBoxLayout;
    hLayout->addWidget(m_message);
    m_message->setMaxLength(80);
    hLayout->addWidget(m_submit);
    Layout->addLayout(hLayout);
    hLayout = new QHBoxLayout;
    hLayout->addStretch(1);
    hLayout->addWidget(m_pay);
    Layout->addLayout(hLayout);

    connect(m_message, &QLineEdit::textChanged, [=]
    {
        if (m_message->text().count() == 0)
            m_submit->setEnabled(false);
        else
            m_submit->setEnabled(true);
    });
    connect(m_message, &QLineEdit::returnPressed, [=] ()
    {
        if (m_message->text().count() != 0)
            onSend();
        m_message->clear();
    });
    connect(m_submit, SIGNAL(clicked(bool)), this, SLOT(onSend()));
    connect(m_submit, SIGNAL(clicked(bool)), m_message, SLOT(clear()));
    connect(m_pay, &QPushButton::clicked, [=] ()
    {
        DiningTable table = DiningTable::extractTable(m_parent->tableId());
        table.setState("结账中");
        table.save('s');

        m_parent->m_pay->setUp();
        m_parent->setFixedIndex(3);
    });

    update();
}

//用餐时更新菜品状态
void eating::update()
{
    int tableId = m_parent->tableId();
    m_tray = Dishes(tableId);   //获取本餐桌所有菜品

    m_dishesTable->setRowCount(m_tray.count());
    for (int i=0; i<m_tray.count(); ++i)
    {
        m_dishesTable->setItem(i, 0, new QTableWidgetItem(QString::number(i+1)));
        m_dishesTable->setItem(i, 1, new QTableWidgetItem(m_tray[i].name()));
        m_dishesTable->setItem(i, 2, new QTableWidgetItem(m_tray[i].category()));
        m_dishesTable->setItem(i, 3, new QTableWidgetItem(m_tray[i].priceToString()));
        m_dishesTable->setItem(i, 4, new QTableWidgetItem(m_tray[i].dishState()));
    }
}

//给服务员发送消息
void eating::onSend()
{
    DiningTable table = m_parent->getTable();

    table.setMessage(m_message->text().trimmed());//去除消息前后的空格, 以便和厨师发送的菜品就绪消息区分
    table.save('m');//保存消息

    m_parent->wmsender.send("waiter", "update");//给服务员发送消息, 使其更新消息列表

    auto Msg = new QMessageBox(QMessageBox::Information, "发送成功", "您的信息已经成功发送给服务员, 请稍等", QMessageBox::Ok);
    Msg->setButtonText(QMessageBox::Ok, "确认");
    Msg->exec();
}

//支付界面
pay::pay(CustomerWidget *parent):m_parent(parent)
{

}

//支付界面初始化
void pay::setUp()
{

    DiningTable table = m_parent->getTable();//获取餐桌
    table.setState("结账中");
    table.save('s');
    Dishes dishes(table.id());//获取全部菜品(此时多个相同菜品以不同的对象储存)
    dishes.merge();//合并菜品(此时多个相同菜品用同一对象储存, 计入重数)

    //账单初始化
    m_dishesTable = new QTableWidget(this);
    m_dishesTable->setRowCount(dishes.count()+1);
    m_dishesTable->setColumnCount(3);
    m_dishesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    QStringList headerLabel;
    headerLabel<<"菜名"<<"价格"<<"数量";
    m_dishesTable->setHorizontalHeaderLabels(headerLabel);
    m_dishesTable->setEditTriggers(QTableView::NoEditTriggers);
    for (int i=0; i<dishes.count(); ++i)
    {
        m_dishesTable->setItem(i, 0, new QTableWidgetItem(dishes[i].name()));
        m_dishesTable->setItem(i, 1, new QTableWidgetItem(dishes[i].priceToString()));
        m_dishesTable->setItem(i, 2, new QTableWidgetItem(QString::number(dishes[i].count())));
    }
    m_dishesTable->setItem(dishes.count(), 0, new QTableWidgetItem("合计"));
    m_dishesTable->setItem(dishes.count(), 1, new QTableWidgetItem(dishes.totalPrice()));
    m_dishesTable->setItem(dishes.count(), 2, new QTableWidgetItem(QString::number(dishes.totalCount())));
    m_dishesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_dishesTable->verticalHeader()->setVisible(false);

    //账目信息
    auto label1 = new QLabel("您的账单如下");
    auto label2 = new QLabel(QString("您共需支付%1元").arg(dishes.totalPrice()));

    //支付方式
    auto byWechat = new QPushButton("微信支付");
    auto byCash = new QPushButton("现金支付");

    //界面布局
    auto Layout = new QVBoxLayout(this);
    Layout->addWidget(label1);
    Layout->addWidget(m_dishesTable);
    auto hLayout = new QHBoxLayout;
    hLayout->addStretch(1);
    hLayout->addWidget(label2);
    hLayout->addWidget(byWechat);
    hLayout->addWidget(byCash);
    Layout->addLayout(hLayout);
    connect(byWechat, SIGNAL(clicked(bool)), this, SLOT(onWechat()));
    connect(byCash, SIGNAL(clicked(bool)), this, SLOT(onCash()));
}

//微信支付
void pay::onWechat()
{
    auto wechat = new QDialog;
    wechat->setWindowTitle("微信支付");

    //设置二维码图片
    auto showQRcode = new QLabel(this);
    QImage *img = new QImage;
    img->load(":/qrcode");
    QImage *scaledImg = new QImage;
    *scaledImg = img->scaled(400, 400, Qt::KeepAspectRatio);
    showQRcode->setPixmap(QPixmap::fromImage(*scaledImg));

    auto finish = new QPushButton("支付完成");

    //界面布局
    auto Layout = new QVBoxLayout(wechat);
    Layout->addWidget(showQRcode);
    Layout->addWidget(finish, Qt::AlignRight);
    connect(finish, SIGNAL(clicked(bool)), wechat, SLOT(accept()));
    wechat->exec();

    //发送结账消息
    DiningTables tables;
    tables[m_parent->tableId()-1].setMessage("结账");
    tables[m_parent->tableId()-1].save('m');

    //向服务员发送消息, 使其更新消息列表
    m_parent->wmsender.send("waiter", "update");
}

//现金支付
void pay::onCash()
{
    auto Msg = new QMessageBox(QMessageBox::Information, "现金支付", "请等待服务员协助您完成支付", QMessageBox::Ok);
    Msg->setButtonText(QMessageBox::Ok, "支付完成");
    Msg->exec();

    //发送结账消息
    DiningTable table = m_parent->getTable();
    table.setMessage("结账");
    table.save('m');

    //向服务员发送消息, 使其更新消息列表
    m_parent->wmsender.send("waiter", "update");
}

/*
 * 检查餐桌状态, 当餐桌状态为"已结账"时自动跳转到评分界面
 * "已结账"会在服务员解决"结账"消息后产生
 */
void pay::CheckPayState()
{
    auto table = m_parent->getTable();
    if (table.tableState() == "已结账")//跳转到评分界面
    {
        m_parent->m_score->setUp();
        m_parent->setFixedIndex(4);
    }
}

//评分界面
score::score(CustomerWidget *parent):m_parent(parent)
{

}
//评分界面初始化
void score::setUp()
{
    m_scored = false;//默认为未评分

    //将此顾客的餐桌信息重置为0
    m_customer = m_parent->getCustomer();
    m_customer->setTableId(0);
    m_customer->saveTableId();

    m_dishes = Dishes(m_parent->tableId());     //提取菜品信息

    //基本控件添加
    auto label1 = new QLabel("请您对菜品做出评价:");
    auto label2 = new QLabel("请您对服务员做出评价");
    auto submit = new QPushButton("提交");

    //添加打分列表
    m_scoreTable = new QTableWidget(this);
    m_dishScores.clear();
    m_scoreTable->setRowCount(m_dishes.count());
    m_scoreTable->setColumnCount(5);
    m_scoreTable->setEditTriggers(QTableView::NoEditTriggers);
    QStringList headerLabel;
    headerLabel<<"序号"<<"菜名"<<"类别"<<"价格"<<"评分";
    m_scoreTable->setHorizontalHeaderLabels(headerLabel);
    for (int i=0; i<m_dishes.count(); ++i)
    {
        m_scoreTable->setItem(i, 0, new QTableWidgetItem(QString::number(i+1)));
        m_scoreTable->setItem(i, 1, new QTableWidgetItem(m_dishes[i].name()));
        m_scoreTable->setItem(i, 2, new QTableWidgetItem(m_dishes[i].category()));
        m_scoreTable->setItem(i, 3, new QTableWidgetItem(m_dishes[i].priceToString()));
        auto spinBox = new QSpinBox(this);
        spinBox->setStyleSheet("QSpinBox{border-radius:0;}");
        spinBox->setRange(0, 5);
        spinBox->setValue(5);
        m_dishScores.append(spinBox);
        m_scoreTable->setCellWidget(i, 4, spinBox);
    }
    m_scoreTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_scoreTable->verticalHeader()->setVisible(false);
    m_waiterScore = new QSpinBox(this);
    m_waiterScore->setFixedWidth(150);
    m_waiterScore->setRange(0, 5);
    m_waiterScore->setValue(5);

    //界面布局
    auto Layout = new QVBoxLayout(this);
    auto hLayout = new QHBoxLayout;
    Layout->addWidget(label1);
    Layout->addWidget(m_scoreTable);
    hLayout->addWidget(label2);
    hLayout->addWidget(m_waiterScore);
    hLayout->addStretch(1);
    Layout->addLayout(hLayout);
    hLayout = new QHBoxLayout;
    hLayout->addStretch(1);
    hLayout->addWidget(submit);
    Layout->addLayout(hLayout);

    connect(submit, SIGNAL(clicked(bool)), this, SLOT(onSubmit()));
}

//提交评分结果
void score::onSubmit()
{
    m_parent->wmsender.send("customer", "update");//给其他顾客发信息, 使其更新餐桌信息

    m_parent->close();

    //保存厨师评分
    auto cooks = Users(2);
    for (int i=0; i<m_dishes.count(); ++i)
    {
        if (m_dishes[i].cookId() != -1)
        {
            int cookId = m_dishes[i].cookId();
            int currentScore = m_dishScores[i]->value();
            Cook* cook = (Cook*)cooks[cookId-1];
            cook->setScoreCount(currentScore+cook->scoreCount());
            cook->saveCook();
        }//end of if
    }//end of for

    //保存服务员评分和工作数据
    auto waiters = Users(3);
    auto table = m_parent->getTable();
    int  waiterId = table.waiterId();
    if (waiterId != -1)
    {
        int currentScore = m_waiterScore->value();
        Waiter* waiter = (Waiter*)waiters[waiterId-1];
        waiter->setScoreCount(currentScore+waiter->scoreCount());
        waiter->setTableCount(waiter->tableCount()+1);
        waiter->saveWaiter();
    }
    table.setState("空闲");
    table.save('s');
    table.setWaiterId(-1);
    table.save('w');

    m_dishes.clearDataBase();//清空此顾客之前所选餐桌菜品数据

    m_scored = true;        //设置为已评分

    //告别界面
    auto bye = new QWidget;
    bye->setFixedSize(300, 150);
    bye->setWindowFlags(Qt::FramelessWindowHint);
    bye->setAttribute(Qt::WA_TranslucentBackground);
    bye->show();

    auto thanks = new QLabel("<font color = blue>欢迎下次光临</font>");
    thanks->setAttribute(Qt::WA_TranslucentBackground);
    thanks->setAlignment(Qt::AlignCenter);
    QFont ft;
    ft.setPointSize(20);
    thanks->setFont(ft);

    //定时3s后切换显示内容
    auto timer = new QTimer;
    timer->setSingleShot(true);
    timer->start(3000);

    auto Layout = new QVBoxLayout(bye);
    Layout->addWidget(thanks);
    connect(timer, &QTimer::timeout, [=] ()
    {
        bye->close();
    });
}
