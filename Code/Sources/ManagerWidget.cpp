#include "ManagerWidget.h"
#include "AdministratorWidget.h"

//厨师界面
CookTable::CookTable(ManagerWidget *parent) : Table(parent)
{
    m_content = getContent();
    m_HeaderLabel = getHeaderLabel();
    setUp();
}

//获取厨师工作数据
QVector<QStringList> CookTable::getContent()
{
    Users Cooks = Users(2);

    QStringList ids;
    QStringList phonenumbers;
    QStringList dishct;
    QStringList timect;
    QStringList scorect;
    QStringList avgtime;
    QStringList avgscore;

    for(int i=0; i<Cooks.count(); ++i)//遍历所有厨师
    {
        Cook *cook = dynamic_cast<Cook*>(Cooks[i]);
        ids.append(QString::number(cook->id()));
        phonenumbers.append(cook->phonenumber());
        dishct.append(QString::number(cook->taskCount()));
        timect.append(QString::number(cook->timeCount()));
        scorect.append(QString::number(cook->scoreCount()));
        if(dishct[i].toInt() == 0)//如果做菜数量为零
        {
            avgtime.append("\\");
            avgscore.append("\\");
        }
        else
        {
            avgtime.append(QString::number(timect[i].toDouble()/dishct[i].toDouble(), 'f', 2));
            avgscore.append(QString::number(scorect[i].toDouble()/dishct[i].toDouble(), 'f', 2));
        }
    }//end of for

    QVector<QStringList> content;
    content.append(ids);
    content.append(phonenumbers);
    content.append(dishct);
    content.append(timect);
    content.append(avgtime);
    content.append(scorect);
    content.append(avgscore);
    return content;
}

QStringList CookTable::getHeaderLabel()
{
    QStringList headerLabel;
    headerLabel << "厨师编号" << "手机号" << "做菜数量" << "做菜总时间/s" << "平均时间/s" << "顾客评分" << "平均得分";
    return headerLabel;
}

//服务员界面
WaiterTable::WaiterTable(ManagerWidget *parent):Table(parent)
{
    m_content = getContent();
    m_HeaderLabel = getHeaderLabel();
    setUp();
}

//获取服务员工作数据
QVector<QStringList> WaiterTable::getContent()
{
    Users Waiters = Users(3);

    QStringList ids;
    QStringList phonenumbers;
    QStringList taskct;
    QStringList timect;
    QStringList tablect;
    QStringList scorect;
    QStringList avgtime;
    QStringList avgscore;

    for(int i=0; i<Waiters.count(); ++i)//遍历所有服务员
    {
        Waiter *waiter = dynamic_cast<Waiter*>(Waiters[i]);
        ids.append(QString::number(waiter->id()));
        phonenumbers.append(waiter->phonenumber());
        taskct.append(QString::number(waiter->taskCount()));
        timect.append(QString::number(waiter->timeCount()));
        tablect.append(QString::number(waiter->tableCount()));
        scorect.append(QString::number(waiter->scoreCount()));

        if(taskct[i].toInt() == 0)//如果服务次数为零
        {
            avgtime.append("\\");
        }
        else
        {
            avgtime.append(QString::number(timect[i].toDouble()/taskct[i].toDouble(), 'f', 2));
        }
        if(tablect[i].toInt() == 0)//如果服务桌数为零
        {
            avgscore.append("\\");
        }
        else
        {
            avgscore.append(QString::number(scorect[i].toDouble()/tablect[i].toDouble(), 'f', 2));
        }
    }//end of for

    QVector<QStringList> content;
    content.append(ids);
    content.append(phonenumbers);
    content.append(taskct);
    content.append(timect);
    content.append(avgtime);
    content.append(tablect);
    content.append(scorect);
    content.append(avgscore);
    return content;
}

QStringList WaiterTable::getHeaderLabel()
{
    QStringList headerLabel;
    headerLabel << "服务员编号" << "手机号" << "服务总次数" << "服务总时间/s" << "平均服务时间/s" << "服务桌数" << "顾客评分" << "平均得分";
    return headerLabel;
}
DishTable::DishTable(ManagerWidget*parent):Table(parent)
{
    m_content = getContent();
    m_HeaderLabel = getHeaderLabel();
    setUp();

    //合并单元格
    m_table->setSpan(m_content[0].count()-1, 0, 1, m_HeaderLabel.count()-1);
    QTableWidgetItem* item = new QTableWidgetItem;
    m_table->setItem(m_content[0].count()-1, 0, item);
    item->setText("营业额");
    item->setTextAlignment(Qt::AlignCenter);
}

QVector<QStringList> DishTable::getContent()
{
    Dishes menu = Dishes(0);
    QStringList ids;
    QStringList categorys;
    QStringList name;
    QStringList price;
    QStringList description;
    QStringList purchasect;
    QStringList sale;

    for(int i=0; i<menu.count(); ++i)
    {
        ids.append(QString::number(menu[i].id()));
        categorys.append(menu[i].category());
        name.append(menu[i].name());
        price.append(menu[i].priceToString());
        description.append(menu[i].description());
        purchasect.append(QString::number(menu[i].count()));
        sale.append(QString::number(price[i].toDouble()*purchasect[i].toInt(), 'f', 2));
    }

    QVector<QStringList> content;

    //在末尾添加""占位, 以便与sale保持行数相等
    ids.append("");
    categorys.append("");
    name.append("");
    price.append("");
    description.append("");
    purchasect.append("");

    sale.append(menu.totalPrice());

    content.append(ids);
    content.append(categorys);
    content.append(name);
    content.append(price);
    content.append(description);
    content.append(purchasect);
    content.append(sale);
    return content;
}

QStringList DishTable::getHeaderLabel()
{
    QStringList headerLabel;
    headerLabel << "菜品编号" << "菜品类别" << "菜品名称" << "单价/元" << "菜品描述" << "售出总数" << "销售额/元";
    return headerLabel;
}
void Table::setUp()
{
    //表格与按钮的添加与设置
    m_table = new QTableWidget(this);
    m_table->setEditTriggers(QTableView::NoEditTriggers);
    m_table->setColumnCount(m_content.count());
    m_table->setHorizontalHeaderLabels(m_HeaderLabel);
    m_table->verticalHeader()->setVisible(false);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    AdministratorWidget::showList(m_table, m_content);
    m_ExportExcel = new QPushButton("导出为Excel");

    //界面布局
    auto Layout = new QVBoxLayout(this);
    auto hLayout = new QHBoxLayout;
    Layout->addWidget(m_table);
    hLayout->addStretch(1);
    hLayout->addWidget(m_ExportExcel);
    Layout->addLayout(hLayout);

    connect(m_ExportExcel, SIGNAL(clicked(bool)), m_parent, SLOT(onExport()));
}

//将表格导出为Excel
void ManagerWidget::onExport()
{
    //获取保存路径
    QString filepath = QFileDialog::getSaveFileName(this, "保存路径", ".", "Microsoft Office 2007 (*.xlsx)");

    if(!filepath.isEmpty())
    {
        ExportExcel expxlsx(filepath, "厨师工作数据");
        for(int i=0; i<m_cook->m_HeaderLabel.count(); ++i)//写入表头
        {
            auto cell = expxlsx.insertCell(1, i+1, m_cook->m_HeaderLabel[i]);
            cell->setProperty("RowHeight", 20);
            cell->setProperty("ColumnWidth", 13);
            QAxObject* interior = cell->querySubObject("Interior");
            interior->setProperty("Color", QColor(91, 155, 213));
            QAxObject* border = cell->querySubObject("Borders");
            border->setProperty("Color", QColor(155, 194, 230));
        }
        for(int i=0; i<m_cook->m_content.count(); ++i)//写入厨师工作数据
        {
            for(int j=0;j<m_cook->m_content[i].count(); ++j)
            {
                auto cell = expxlsx.insertCell(j+2, i+1, m_cook->m_content[i][j]);
                cell->setProperty("RowHeight", 20);
                cell->setProperty("ColumnWidth", 13);
                QAxObject* interior = cell->querySubObject("Interior");
                interior->setProperty("Color", QColor(255-44*(j%2), 255-20*(j%2), 255-8*(j%2)));
                QAxObject* border = cell->querySubObject("Borders");
                border->setProperty("Color", QColor(155, 194, 230));
            }
        }
        expxlsx.insertSheet("服务员工作数据");
        for(int i=0; i<m_waiter->m_HeaderLabel.count(); ++i)//写入表头
        {
            auto cell = expxlsx.insertCell(1, i+1, m_waiter->m_HeaderLabel[i]);
            cell->setProperty("RowHeight", 20);
            cell->setProperty("ColumnWidth", 13);
            QAxObject* interior = cell->querySubObject("Interior");
            interior->setProperty("Color", QColor(91, 155, 213));
            QAxObject* border = cell->querySubObject("Borders");
            border->setProperty("Color", QColor(155, 194, 230));
        }
        for(int i=0; i<m_waiter->m_content.count(); ++i)//写入服务员工作数据
        {
            for(int j= 0; j<m_waiter->m_content[i].count(); ++j)
            {
                auto cell = expxlsx.insertCell(j+2, i+1, m_waiter->m_content[i][j]);
                cell->setProperty("RowHeight", 20);
                cell->setProperty("ColumnWidth", 13);
                QAxObject* interior = cell->querySubObject("Interior");
                interior->setProperty("Color", QColor(255-44*(j%2), 255-20*(j%2), 255-8*(j%2)));
                QAxObject* border = cell->querySubObject("Borders");
                border->setProperty("Color", QColor(155, 194, 230));
            }
        }
        expxlsx.insertSheet("销售数据");
        for(int i= 0; i<m_dish->m_HeaderLabel.count(); ++i)//写入表头
        {
            auto cell = expxlsx.insertCell(1, i+1, m_dish->m_HeaderLabel[i]);
            cell->setProperty("RowHeight", 20);
            cell->setProperty("ColumnWidth", 13);
            QAxObject* interior = cell->querySubObject("Interior");
            interior->setProperty("Color", QColor(91, 155, 213));
            QAxObject* border = cell->querySubObject("Borders");
            border->setProperty("Color", QColor(155, 194, 230));
        }
        for(int i=0; i<m_dish->m_content.count(); ++i)//写入菜品销售数据
        {
            for(int j=0; j<m_dish->m_content[i].count(); ++j)
            {
                auto cell = expxlsx.insertCell(j+2, i+1, m_dish->m_content[i][j]);
                cell->setProperty("RowHeight", 20);
                cell->setProperty("ColumnWidth", 13);
                QAxObject* interior = cell->querySubObject("Interior");
                interior->setProperty("Color", QColor(255-44*(j%2), 255-20*(j%2), 255-8*(j%2)));
                QAxObject* border = cell->querySubObject("Borders");
                border->setProperty("Color", QColor(155, 194, 230));
            }
        }

        //写入营业额, 并合并单元格
        int row = m_dish->m_content[0].count()+1;
        auto cell = expxlsx.insertCell(row, 1, "营业额");
        expxlsx.cellMerge(m_dish->m_content[0].count()+1, m_dish->m_content[0].count()+1, 1, 6);
        QAxObject* interior = cell->querySubObject("Interior");
        interior->setProperty("Color", QColor(255, 230, 153));
        QAxObject* border = cell->querySubObject("Borders");
        border->setProperty("Color", QColor(155, 194, 230));

        //保存并关闭
        expxlsx.save();
        expxlsx.quit();

        auto msg = new QMessageBox(QMessageBox::Information,\
                                   "导出成功", "是否打开?", QMessageBox::Ok | QMessageBox::Cancel);
        msg->setButtonText(QMessageBox::Ok, "打开");
        msg->setButtonText(QMessageBox::Cancel, "取消");
        if(msg->exec() == QMessageBox::Ok)//打开Excel
        {
            QAxObject excel("Excel.Application");
            excel.setProperty("Visible", true);
            QAxObject *work_books = excel.querySubObject("WorkBooks");
            work_books->dynamicCall("Open (const QString&)", filepath);
        }
        else
        {
            msg->close();
        }

    }
}

//经理界面
ManagerWidget::ManagerWidget() : QWidget()
{
    //界面基本属性
    setWindowFlags(Qt::FramelessWindowHint | windowFlags());
    setAttribute(Qt::WA_TranslucentBackground);
    auto titleBar = new TitleBar(this);
    installEventFilter(titleBar);
    setWindowTitle("经理界面");
    setFixedSize(800, 500);
    setWindowIcon(QIcon(":/icon.ico"));

    m_cook = new CookTable(this);
    m_waiter = new WaiterTable(this);
    m_dish = new DishTable(this);
    m_tab = new QTabWidget(this);
    m_tab->addTab(m_cook, "厨师工作数据");
    m_tab->addTab(m_waiter, "服务员工作数据");
    m_tab->addTab(m_dish, "销售数据");

    auto layout = new QVBoxLayout(this);
    layout->addWidget(titleBar);
    layout->addWidget(m_tab);
    layout->setContentsMargins(5, 5, 5, 5);
}

void ManagerWidget::mousePressEvent(QMouseEvent *event)
{
    dragEffect(this, event);//界面拖动效果
}

void ManagerWidget::paintEvent(QPaintEvent *event)
{
    paintEffect(this, event);//绘制圆角、阴影、背景
}

//接收界面关闭的信息
bool ManagerWidget::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    QString msg = WMSender::msgProcess(eventType, message, result);
    if (msg == "close")
    {
        close();
    }

    return QWidget::nativeEvent(eventType, message, result);
}
