#include "AdministratorWidget.h"

//默认用户、菜品、餐桌未改动
bool AdministratorWidget::userChanged = false;
bool AdministratorWidget::menuChanged = false;
bool AdministratorWidget::tableChanged = false;

AdministratorWidget::AdministratorWidget(SignWidget *parent) : m_parent(parent)
{
    //设置主界面基本属性
    setWindowFlags(Qt::FramelessWindowHint | windowFlags());
    setAttribute(Qt::WA_TranslucentBackground);
    TitleBar *titleBar = new TitleBar(this);
    installEventFilter(titleBar);
    setFixedSize(650, 550);
    setWindowTitle("管理员界面");
    setWindowIcon(QIcon(":/icon.ico"));

    //加入子界面
    auto tabWidget = new QTabWidget(this);
    m_menuMode = new menuManage;
    m_userMode = new usersManage;
    m_tableMode = new tableManage;
    tabWidget->addTab(m_menuMode, "菜单管理");
    tabWidget->addTab(m_userMode, "用户管理");
    tabWidget->addTab(m_tableMode, "餐桌管理");

    //界面布局
    auto layout = new QVBoxLayout(this);
    layout->addWidget(titleBar);
    layout->addWidget(tabWidget);
    layout->setContentsMargins(5, 5, 5, 5);

    wmsender = WMSender(this);
}

void AdministratorWidget::mousePressEvent(QMouseEvent *event)
{
    dragEffect(this, event);//界面拖动效果
}

void AdministratorWidget::paintEvent(QPaintEvent *event)
{
    paintEffect(this, event);//绘制圆角、阴影、背景
}

bool AdministratorWidget::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    QString msg = WMSender::msgProcess(eventType, message, result);
    if (msg == "new")
    {
        wmsender.updateReceivers();
    }
    if(msg == "close")
    {
        close();
    }
    return QWidget::nativeEvent(eventType, message, result);
}

//重写closeEvent, 界面关闭之前提示保存数据
void AdministratorWidget::closeEvent(QCloseEvent *event)
{
    if (menuChanged || userChanged || tableChanged)  //如果数据有改动
    {
        auto Msg = new QMessageBox(QMessageBox::Information, "保存修改", "是否保存修改?", \
                                   QMessageBox::Save|QMessageBox::Discard);
        Msg->setButtonText(QMessageBox::Save, "保存");
        Msg->setButtonText(QMessageBox::Discard, "不保存");
        int result = Msg->exec();
        if (result == QMessageBox::Save)
        {
            if (userChanged)     //保存用户数据
            {
                for (int i=0; i<5; ++i)
                {
                    m_userMode->m_Users[i].save();
                }

                wmsender.send("sign", "update");      //发送消息，使开始界面更新用户数据
            }
            if (menuChanged)     //保存菜单数据
            {
                m_menuMode->m_Dishes = m_menuMode->m_DishesInCategory;
                m_menuMode->m_Dishes.save();    //保存菜品数据
                Dishes::saveCategory();         //保存类别数据
                wmsender.send("customer", "update"); //给顾客发送消息, 处理原来菜品为空导致顾客无法操作得问题
            }
            if (tableChanged)    //保存餐桌数据
            {
                m_tableMode->m_diningTables.save();

                //发送消息更新餐桌信息
                wmsender.send("customer", "update");
                wmsender.send("waiter", "update");
            }
        }
        userChanged = menuChanged = tableChanged = false;   //重置数据更改信息
    }
}

/*
 * 用于实现全选框的三态效果
 * 当checkBoxes全部选中时, selectAll为Checked
 * 当checkBoxes全部未选中时, selectAll为Unchecked
 * 当checkBoxes部分选中时, selectAll为PartiallyChecked
 * 点击全选框selectAll时, checkBoxes中的所有复选框随之改变到对应状态
 * 用于菜单管理和用户管理的类别选择
 */
void AdministratorWidget::onCheckChange(QVector<QCheckBox*> checkBoxes, QCheckBox *selectAll, QCheckBox *sender)
{
    if (sender != selectAll)//当信号由checkBoxes中某一个对象发出
    {
        bool allCheck = true;
        bool allUncheck = true;
        for (QCheckBox *&i : checkBoxes)
        {
            if (i->checkState() == Qt::Checked)
            {
                allUncheck = false;
            }
            else
            {
                allCheck = false;
            }
        }//end of for

        //依据checkBoxes的选择状态调整selectAll的状态
        if (allCheck)
        {
            selectAll->setTristate(false);
            selectAll->setCheckState(Qt::Checked);
        }
        else if (allUncheck)
        {
            selectAll->setTristate(false);
            selectAll->setCheckState(Qt::Unchecked);
        }
        else
        {
            selectAll->setCheckState(Qt::PartiallyChecked);
        }
    }//end of if
    else//信号由selectAll发出, 调整checkBoxes的选择状态
    {
        auto state = selectAll->checkState();
        if (state != Qt::PartiallyChecked)
        {
            for (auto &i : checkBoxes)
                i->setCheckState(state);
        }
    }
}

/*
 * 将content中的内容写入表格table中
 * 用于用户管理和菜单管理的表格
 */
void AdministratorWidget::showList(QTableWidget *table, QVector<QStringList> &content)
{
    if (content.count() > 0)
    {
        table->setRowCount(content[0].count());
        table->setColumnCount(content.count());
        for (int i=0; i<content[0].count(); ++i)
        {
            for (int j=0; j<content.count(); j++)
            {
                table->setItem(i, j, new QTableWidgetItem(content[j][i]));
            }//end of for
        }//end of for
    }
}

//菜单管理界面
menuManage::menuManage()
{
    //提取全部菜品信息并分类存储
    m_Dishes = Dishes(0);
    m_DishesInCategory = m_Dishes.categoryHash();

    //类别管理相关控件
    m_category = new QComboBox(this);
    m_categoryList = new QListWidget(this);
    m_categoryList->setResizeMode(QListView::Adjust);
    m_categoryMsg = new QLineEdit(this);
    m_categoryMsg->setStyleSheet("QLineEdit{border:0;}");
    m_category->setEditable(false);
    m_category->setModel(m_categoryList->model());
    m_category->setView(m_categoryList);
    m_category->setLineEdit(m_categoryMsg);
    m_categoryMsg->setReadOnly(true);
    m_categoryBox = NULL;

    //菜单管理相关控件
    m_add = new QPushButton("添加", this);
    m_delete = new QPushButton("删除", this);
    m_edit = new QPushButton("编辑", this);
    m_clear = new QPushButton("清空", this);
    m_search = new QLineEdit;
    m_search->setClearButtonEnabled(true);
    m_search->setPlaceholderText("搜索");
    m_searchMsg = new QLabel;
    m_menuTable = new QTableWidget;
    m_menuTable->setEditTriggers(QTableWidget::NoEditTriggers);
    m_vaccant = new QLabel("菜单为空");
    QFont ft;
    ft.setPointSize(20);
    m_vaccant->setFont(ft);
    m_vaccant->setAlignment(Qt::AlignCenter);

    m_nameInput = NULL;

    //界面布局
    auto menugLayout = new QGridLayout(this);
    menugLayout->addWidget(m_category, 0, 0, 1, 1);
    menugLayout->addWidget(m_search, 0, 1, 1, 3);
    menugLayout->addWidget(m_searchMsg, 0, 4, 1, 1);
    menugLayout->addWidget(m_add, 1, 4, 1, 1);
    menugLayout->addWidget(m_delete, 2, 4, 1, 1);
    menugLayout->addWidget(m_edit, 3, 4, 1, 1);
    menugLayout->addWidget(m_clear, 4, 4, 1, 1);
    menugLayout->addWidget(m_menuTable, 1, 0, 4, 4);
    menugLayout->addWidget(m_vaccant, 1, 0, 4, 4);
    for (int i=0; i<5; ++i)
    {
        menugLayout->setRowMinimumHeight(i, 100);
    }

    connect(m_add, SIGNAL(clicked(bool)), this, SLOT(onMenuAdd()));
    connect(m_clear, SIGNAL(clicked(bool)), this, SLOT(onMenuClear()));
    connect(m_delete, SIGNAL(clicked(bool)), this, SLOT(onMenuDelete()));
    connect(m_edit, SIGNAL(clicked(bool)), this, SLOT(onMenuEdit()));
    connect(m_search, SIGNAL(textChanged(QString)), this, SLOT(onMenuSearch(QString)));
    connect(m_add, SIGNAL(clicked(bool)), m_search, SLOT(clear()));
    connect(m_clear, SIGNAL(clicked(bool)), m_search, SLOT(clear()));
    connect(m_menuTable, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(onMenuEdit()));
    connect(m_menuTable, &QTableWidget::cellClicked, [=] ()
    {
        m_delete->setEnabled(true);
        m_edit->setEnabled(true);
    });

    //初始化类别和菜品列表
    categoryUpdate(true);
    menuUpdate();
}

//菜品类别管理
void menuManage::onCategoryManage()
{
    m_categoryManageDialog = new QDialog(this);
    m_categoryManageDialog->setWindowTitle("菜品类别管理");

    //界面控件的添加与设置
    m_categoryTable = new QTableWidget(this);
    m_categoryTable->setColumnCount(3);
    m_categoryTable->setRowCount(Dishes::categoryList.count());
    QStringList headerLabel;
    headerLabel << "序号" << "类别名称" << "菜品数量";
    m_categoryTable->setHorizontalHeaderLabels(headerLabel);
    m_categoryTable->verticalHeader()->setVisible(false);
    m_categoryTable->setEditTriggers(QTableView::NoEditTriggers);
    m_categoryTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    for (int i=0; i<Dishes::categoryList.count(); ++i)
    {
        m_categoryTable->setItem(i, 0, new QTableWidgetItem(QString::number(i+1)));
        m_categoryTable->setItem(i, 1, new QTableWidgetItem(Dishes::categoryList[i]));
        m_categoryTable->setItem(i, 2,\
                                 new QTableWidgetItem(\
                                     QString::number(m_DishesInCategory.value(Dishes::categoryList[i]).count())));
    }

    m_addCategory = new QPushButton("添加", m_categoryManageDialog);
    m_editCategory = new QPushButton("编辑", m_categoryManageDialog);
    m_removeCategory = new QPushButton("移除", m_categoryManageDialog);
    m_moveCategory = new QPushButton("移动", m_categoryManageDialog);
    m_clearCategory = new QPushButton("清空", m_categoryManageDialog);

    if (Dishes::categoryList.count() == 0)
    {
        m_editCategory->setEnabled(false);
        m_moveCategory->setEnabled(false);
        m_removeCategory->setEnabled(false);
        m_clearCategory->setEnabled(false);
    }

    if (m_categoryTable->currentRow() < 0)
    {
        m_editCategory->setEnabled(false);
    }
    //界面布局
    auto Layout = new QGridLayout(m_categoryManageDialog);
    Layout->addWidget(m_categoryTable, 0, 0, 5, 3);
    Layout->addWidget(m_addCategory, 0, 3, 1, 1);
    Layout->addWidget(m_editCategory, 1, 3, 1, 1);
    Layout->addWidget(m_removeCategory, 2 , 3, 1, 1);
    Layout->addWidget(m_moveCategory, 3, 3, 1, 1);
    Layout->addWidget(m_clearCategory, 4, 3, 1, 1);

    connect(m_addCategory, SIGNAL(clicked(bool)), this, SLOT(onCategoryAdd()));
    connect(m_editCategory, SIGNAL(clicked(bool)), this,SLOT(onCategoryEdit()));
    connect(m_removeCategory, SIGNAL(clicked(bool)), this, SLOT(onCategoryRemove()));
    connect(m_moveCategory, SIGNAL(clicked(bool)), this, SLOT(onCategoryMove()));
    connect(m_clearCategory, SIGNAL(clicked(bool)), this, SLOT(onCategoryClear()));
    connect(m_categoryTable, &QTableWidget::cellClicked, [=] ()
    {
       m_editCategory->setEnabled(true);
       m_removeCategory->setEnabled(true);
    });

    m_categoryManageDialog->exec();
}

//添加菜品类别
void menuManage::onCategoryAdd()
{
    auto addDialog = new QDialog(this);
    addDialog->setWindowTitle("添加菜品类别");
    auto categoryInput = new QLineEdit(this);
    categoryInput->setPlaceholderText("类别名称");
    auto submit = new QPushButton("提交", this);
    submit->setEnabled(false);
    auto cancel = new QPushButton("取消", this);
    auto errorMsg = new QLabel(this);

    auto layout = new QGridLayout(addDialog);
    layout->addWidget(categoryInput, 0, 0, 1, 2);
    layout->addWidget(errorMsg, 1, 0, 1, 2);
    layout->addWidget(submit, 2, 0, 1, 1);
    layout->addWidget(cancel, 2, 1, 1, 1);

    connect(submit, SIGNAL(clicked(bool)), addDialog, SLOT(accept()));
    connect(cancel, SIGNAL(clicked(bool)), addDialog, SLOT(close()));
    //判断类别输入是否有效
    connect(categoryInput, &QLineEdit::textChanged, [=] ()
    {
        if (categoryInput->text().trimmed() == "")
        {
            submit->setEnabled(false);
            errorMsg->setText("类别名称不能为空");
        }
        else if (Dishes::categoryList.contains(categoryInput->text().trimmed()))
        {
            submit->setEnabled(false);
            errorMsg->setText("该类别已存在");
        }
        else
        {
            errorMsg->setText("");
            submit->setEnabled(true);
        }
    });

    categoryInput->setFocus();

    if (addDialog->exec() == QDialog::Accepted)
    {
        Dishes::categoryList.append(categoryInput->text().trimmed());
        m_checkStates.append(Qt::Checked);//默认新增选中类别

        categoryUpdate(false);
        menuUpdate();
        AdministratorWidget::menuChanged = true;

        if (m_nameInput)//添加和编辑菜品状态下设定焦点
        {
            m_nameInput->setFocus();
        }
        if (m_categoryBox)//添加和编辑菜品状态下更新类别显示框
        {
            m_categoryBox->clear();
            m_categoryBox->addItems(Dishes::categoryList);
            m_categoryBox->setCurrentText(categoryInput->text().trimmed());
        }
    }

    if (m_categoryManageDialog)
    {
        m_categoryManageDialog->close();
    }
}

//编辑菜品类别
void menuManage::onCategoryEdit()
{
    auto editDialog = new QDialog(this);
    editDialog->setWindowTitle("编辑菜品类别");

    //界面控件添加和设置
    auto categoryInput = new QLineEdit(editDialog);

    auto index = m_categoryTable->currentRow();
    auto editingCategory = Dishes::categoryList[index];
    categoryInput->setText(Dishes::categoryList[index]);
    auto submit = new QPushButton("确认",editDialog);
    auto cancel = new QPushButton("取消",editDialog);

    //界面布局
    auto Layout = new QVBoxLayout(editDialog);
    auto hLayout = new QHBoxLayout;
    Layout->addWidget(categoryInput);
    hLayout->addWidget(submit);
    hLayout->addWidget(cancel);
    Layout->addLayout(hLayout);

    connect(categoryInput, &QLineEdit::textChanged, [=] ()
    {
        if (categoryInput->text().trimmed().count() == 0)//如果类别名称为空
        {
            submit->setEnabled(false);
        }
        else
        {
            submit->setEnabled(true);
        }
    });
    connect(submit, SIGNAL(clicked(bool)), editDialog, SLOT(accept()));
    connect(cancel, SIGNAL(clicked(bool)), editDialog, SLOT(close()));

    if (editDialog->exec() == QDialog::Accepted)
    {
        auto newCategory = categoryInput->text().trimmed();
        if (newCategory == Dishes::categoryList[index] )//如果没有改动
        {
            editDialog->close();
        }
        else if (Dishes::categoryList.contains(newCategory))//如果改动为已有类别, 询问是否合并
        {
            QMessageBox *msg = new QMessageBox(QMessageBox::Information,\
                                               "该类别已存在", "是否合并?" ,\
                                               QMessageBox::Ok | QMessageBox::Cancel);
            msg->setButtonText(QMessageBox::Ok, "合并");
            msg->setButtonText(QMessageBox::Cancel, "取消");

            if (msg->exec() == QMessageBox::Ok)
            {
                //合并类别
                Dishes::categoryList.removeAt(index);
                m_DishesInCategory[editingCategory].setCategory(newCategory);
                m_DishesInCategory[newCategory].add(m_DishesInCategory.value(editingCategory));
                m_DishesInCategory.remove(editingCategory);
            }//end of if
        }// end of else if
        else//改动为新类别
        {
            Dishes::categoryList[index] = newCategory;
            m_DishesInCategory[newCategory] = m_DishesInCategory[editingCategory];
            m_DishesInCategory[newCategory].setCategory(newCategory);
            m_DishesInCategory.remove(editingCategory);
        }

        categoryUpdate(false);
        menuUpdate();

    }//end of if

    if (m_categoryManageDialog)
    {
        m_categoryManageDialog->close();
    }
}


//移除菜品类别
void menuManage::onCategoryRemove()
{
    auto removeDialog = new QDialog(this);
    auto label = new QLabel("请选择移除类别:", this);
    auto removeCategoryBox = new QComboBox(this);
    removeCategoryBox->addItems(Dishes::categoryList);
    removeCategoryBox->setCurrentIndex(0);
    auto remove = new QPushButton("移除", this);
    auto cancel = new QPushButton("取消", this);

    auto layout = new QVBoxLayout(removeDialog);
    auto hLayout = new QHBoxLayout;
    hLayout->addWidget(label);
    hLayout->addWidget(removeCategoryBox);
    layout->addLayout(hLayout);
    hLayout = new QHBoxLayout;
    hLayout->addWidget(remove);
    hLayout->addWidget(cancel);
    layout->addLayout(hLayout);

    connect(remove, &QPushButton::clicked, [=] ()
    {
        auto msgBox = new QMessageBox(QMessageBox::Warning, "移除类别", \
                                      QString("移除:%1?").arg(removeCategoryBox->currentText()), \
                                      QMessageBox::Ok | QMessageBox::Cancel);
        msgBox->setButtonText(QMessageBox::Ok, "确认");
        msgBox->setButtonText(QMessageBox::Cancel, "取消");
        if (msgBox->exec() == QMessageBox::Ok)
        {
            m_checkStates.removeAt(Dishes::categoryList.indexOf(removeCategoryBox->currentText()));
            Dishes::categoryList.removeOne(removeCategoryBox->currentText());
            m_DishesInCategory.remove(removeCategoryBox->currentText());

            categoryUpdate(false);
            menuUpdate();
            AdministratorWidget::menuChanged = true;
            removeDialog->close();
        }
    });
    connect(cancel, SIGNAL(clicked(bool)), removeDialog, SLOT(close()));
    removeDialog->exec();

    if (m_categoryManageDialog)
    {
        m_categoryManageDialog->close();
    }
}

//将某一类别的全部菜品移动到另一类别
void menuManage::onCategoryMove()
{
    auto moveDialog = new QDialog(this);;
    moveDialog->setWindowTitle("移动菜品");

    //界面控件添加和设置
    auto fromLabel = new QLabel("将菜品从", moveDialog);
    auto toLabel = new QLabel("移动到", moveDialog);
    auto categoryFrom = new QComboBox;
    auto categoryTo = new QComboBox;
    categoryFrom->addItems(Dishes::categoryList);
    categoryTo->addItems(Dishes::categoryList);
    auto submit = new QPushButton("确认移动", moveDialog);
    auto cancel = new QPushButton("取消", moveDialog);

    //界面布局
    auto Layout = new QVBoxLayout(moveDialog);
    auto hLayout = new QHBoxLayout;
    hLayout->addWidget(fromLabel);
    hLayout->addWidget(categoryFrom);
    Layout->addLayout(hLayout);
    hLayout = new QHBoxLayout;
    hLayout->addWidget(toLabel);
    hLayout->addWidget(categoryTo);
    Layout->addLayout(hLayout);
    hLayout = new QHBoxLayout;
    hLayout->addWidget(submit);
    hLayout->addWidget(cancel);
    Layout->addLayout(hLayout);

    connect(submit, SIGNAL(clicked(bool)), moveDialog, SLOT(accept()));
    connect(cancel, SIGNAL(clicked(bool)), moveDialog, SLOT(close()));

    if (moveDialog->exec() == QDialog::Accepted)//如果确认, 将所有菜品从原类别移动到目标类别
    {
        m_DishesInCategory[categoryFrom->currentText()].setCategory(categoryTo->currentText());
        m_DishesInCategory[categoryTo->currentText()].\
                add(m_DishesInCategory.value(categoryFrom->currentText()));
        m_DishesInCategory[categoryFrom->currentText()].clear();

        menuUpdate();
    }

    m_categoryManageDialog->close();
}

void menuManage::onCategoryClear()
{
    Dishes::categoryList.clear();
    m_DishesInCategory.clear();

    categoryUpdate(false);
    menuUpdate();
    m_categoryManageDialog->close();
}

/*
 * 更新菜品类别列表
 * initialize用于标识是否为首次更新
 * 首次默认选中全部类别
 * 以后每次更新保持更新前的选择状态
 */
void menuManage::categoryUpdate(bool initialize)
{
    m_categoryList->clear();
    m_checkCategory.clear();
    if (!Dishes::categoryList.isEmpty())//如果类别列表不为空
    {
        auto item = new QListWidgetItem(m_categoryList);
        m_categoryList->addItem(item);
        m_allCategory = new QCheckBox("全选", this);
        connect(m_allCategory, &QCheckBox::stateChanged, [=] ()
        {
            AdministratorWidget::onCheckChange(m_checkCategory, m_allCategory, m_allCategory);//实现全选框的三态效果
        });
        m_categoryList->setItemWidget(item, m_allCategory);
        for (int i=0; i<Dishes::categoryList.count(); ++i)
        {
            item = new QListWidgetItem(m_categoryList);
            m_categoryList->addItem(item);
            auto checkBox = new QCheckBox(Dishes::categoryList[i], this);
            m_checkCategory.append(checkBox);
            m_categoryList->setItemWidget(item, checkBox);
            connect(checkBox, &QCheckBox::stateChanged, [=] ()
            {
                AdministratorWidget::onCheckChange(m_checkCategory, m_allCategory, checkBox);//实现全选框的三态效果
            });

            if (initialize)
            {
                m_checkStates.append(Qt::Checked);
                checkBox->setChecked(true);
            }
            else
            {
                checkBox->setCheckState(m_checkStates[i]);
            }

            connect(checkBox, &QCheckBox::stateChanged, [=] ()
            {
                m_checkStates[m_checkCategory.indexOf(checkBox)] = checkBox->checkState();
            });
            connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(menuUpdate()));
        }//end of for
    }

    //设置菜品类别管理按钮
    auto item = new QListWidgetItem(m_categoryList);
    item->setTextAlignment(Qt::AlignCenter);
    m_categoryList->addItem(item);
    m_categoryManage = new QPushButton("类别管理", this);
    m_categoryManage->setStyleSheet("QPushButton{border-radius:0px;}");
    m_categoryList->setItemWidget(item, m_categoryManage);
    connect(m_categoryManage, SIGNAL(clicked(bool)), this, SLOT(onCategoryManage()));

    if (Dishes::categoryList.isEmpty())//类别为空时显示信息
    {
        //防止m_categoryMsg的内容改动
        disconnect(m_categoryMsg, SIGNAL(textChanged(QString)), 0, 0);
        connect(m_categoryMsg, &QLineEdit::textChanged, [=] ()
        {
            m_categoryMsg->setText("菜品类别为空");
        });
        m_categoryMsg->setText("菜品类别为空");
    }
    else//类别不为空时显示信息
    {
        //防止m_categoryMsg的内容改动
        disconnect(m_categoryMsg, SIGNAL(textChanged(QString)) , 0 , 0);
        connect(m_categoryMsg, &QLineEdit::textChanged, [this] ()
        {
            m_categoryMsg->setText("选择菜品类别");
        });
        m_categoryMsg->setText("选择菜品类别");
    }
}

//菜单更新
void menuManage::menuUpdate()
{
    //获取选中类别的所有菜品
    m_chosenDishes.clear();
    for (int i=0; i<Dishes::categoryList.count(); ++i)
    {
        if (m_checkCategory[i]->isChecked())
        {
            m_chosenDishes.add(m_DishesInCategory[Dishes::categoryList[i]]);
        }
    }

    if (m_search->text().count() != 0)  //如果在搜索状态下，显示搜索结果
    {
        onMenuSearch(m_search->text());
        return;
    }

    if (m_chosenDishes.count() == 0)//如果选中菜品为空, 重置部分控件
    {
        m_menuTable->hide();
        m_vaccant->show();
        m_edit->setEnabled(false);
        m_clear->setEnabled(false);
        m_search->setEnabled(false);
        m_delete->setEnabled(false);
        m_add->setEnabled(true);
    }
    else//如果选中菜品不为空, 重置部分控件
    {
        m_menuTable->setColumnCount(5);
        m_search->setEnabled(true);
        if ((m_menuTable->currentRow()>=0) && (m_menuTable->currentColumn()>=0))//如果有选中的菜品, 启用编辑和删除
        {
            m_edit->setEnabled(true);
            m_delete->setEnabled(true);
        }
        else//否则禁用编辑和删除
        {
            m_edit->setEnabled(false);
            m_delete->setEnabled(false);
        }
        m_clear->setEnabled(true);
        QStringList label;
        label << "" << "类别" << "菜名" << "价格" << "描述";
        m_menuTable->setHorizontalHeaderLabels(label);
        m_menuTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        m_menuTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        m_menuTable->verticalHeader()->setVisible(false);
        m_vaccant->hide();
        m_menuTable->show();

        //获取选中菜品信息并显示在表格m_menuTable中
        auto contents = m_chosenDishes.toTableContents();
        AdministratorWidget::showList(m_menuTable, contents);
    }//end of else
}

//添加菜品
void menuManage::onMenuAdd()
{
    auto addDialog = new QDialog(this);
    addDialog->setWindowTitle("添加菜品");

    //界面控件添加与设置
    auto categoryLabel = new QLabel("选择菜品类别", this);
    m_categoryBox = new QComboBox(this);
    m_categoryBox->addItems(Dishes::categoryList);
    auto addCategory = new QPushButton("新建类别", this);
    m_nameInput = new QLineEdit(this);
    m_nameInput->setPlaceholderText("请输入菜品名称");
    m_nameInput->setMaxLength(15);
    m_priceInput = new QLineEdit(this);
    m_priceInput->setPlaceholderText("请输入菜品价格");
    m_priceInput->setMaxLength(15);
    QRegExp priceExp("^(([1-9]+)|([0-9]+\\.[0-9]{1,2}))$");
    m_priceInput->setValidator(new QRegExpValidator(priceExp));
    auto descriptionInput = new QTextEdit(this);
    descriptionInput->setPlaceholderText("请输入菜品描述");
    m_errorMsg = new QLabel(this);
    m_submit = new QPushButton("提交", this);
    m_submit->setEnabled(false);
    auto cancel = new QPushButton("取消", this);

    //界面布局
    auto Layout = new QVBoxLayout(addDialog);
    auto hLayout = new QHBoxLayout;
    hLayout->addWidget(categoryLabel);
    hLayout->addWidget(m_categoryBox);
    Layout->addLayout(hLayout);
    Layout->addWidget(addCategory);
    Layout->addWidget(m_nameInput);
    Layout->addWidget(m_priceInput);
    Layout->addWidget(descriptionInput);
    Layout->addWidget(m_errorMsg);
    hLayout = new QHBoxLayout;
    hLayout->addWidget(m_submit);
    hLayout->addWidget(cancel);
    Layout->addLayout(hLayout);

    m_nameInput->setFocus();

    connect(addCategory, SIGNAL(clicked(bool)), this, SLOT(onCategoryAdd()));
    connect(m_nameInput, SIGNAL(returnPressed()), m_priceInput, SLOT(setFocus()));
    connect(m_priceInput, SIGNAL(returnPressed()), descriptionInput, SLOT(setFocus()));
    connect(m_nameInput, SIGNAL(textChanged(QString)), this, SLOT(checkMenuValid()));
    connect(m_priceInput, SIGNAL(textChanged(QString)), this, SLOT(checkMenuValid()));
    connect(m_submit, SIGNAL(clicked(bool)), addDialog, SLOT(accept()));
    connect(cancel, SIGNAL(clicked(bool)), addDialog, SLOT(close()));

    if (addDialog->exec() == QDialog::Accepted)
    {
        QString category;
        if (m_categoryBox->currentIndex() == -1)//如果当前类别列表为空, 且没有添加类别
        {
            //添加新类别"默认类别"
            category = "默认类别";
            Dishes::categoryList.append(category);
            auto checkBox = new QCheckBox("默认类别");
            m_checkCategory.append(checkBox);
            m_checkStates.append(Qt::Checked);
            checkBox->setChecked(true);
            categoryUpdate(false);
            connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(menuUpdate()));
            connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(onCheckChange()));
        }
        else//否则获取当前选中类别
        {
            category = m_categoryBox->currentText();
        }
        if (!Dishes::categoryList.contains(category))
            Dishes::categoryList.append(category);

        QString name = m_nameInput->text();
        QString price = m_priceInput->text();
        QString description = descriptionInput->toPlainText();
        if (description.count() == 0)
        {
            description = "暂无描述";
        }

        Dish dish(category, name, price.toDouble(), description, "", 0, 0, 0);
        m_DishesInCategory[category].add(dish);
        AdministratorWidget::menuChanged = true;
        menuUpdate();
    }
}

/*
 * 删除菜品
 * 实现了普通状态下删除和搜索状态下删除
 */
void menuManage::onMenuDelete()
{
    if (m_search->text().count() == 0)//如果不处于搜索状态
    {
        //获取所选类别的全部菜品中对应菜品并删除
        Dish &dish = m_chosenDishes[m_menuTable->currentRow()];
        m_DishesInCategory[dish.category()].remove(dish);
    }
    else//如果处于搜索状态
    {
        //获取搜索列表中对应菜品并删除
        Dish &dish = m_searchDishes[m_menuTable->currentRow()];
        m_DishesInCategory[dish.category()].remove(dish);
    }

    AdministratorWidget::menuChanged = true;
    menuUpdate();

    if ((m_chosenDishes.count() != 0) && (m_search->text().count() != 0))//如果所选菜品不为空, 并且处于搜索状态
    {
        onMenuSearch(m_search->text());//重新搜索
    }
}

//清空所选菜品列表
void menuManage::onMenuClear()
{
    auto Msg = new QMessageBox(QMessageBox::Warning, "清空菜单", "您确认要清空菜单吗?", QMessageBox::Ok | QMessageBox::Cancel);
    Msg->setButtonText(QMessageBox::Ok, "确认");
    Msg->setButtonText(QMessageBox::Cancel, "取消");
    if (Msg->exec() == QMessageBox::Ok)
    {
        for (QCheckBox *&i : m_checkCategory)
        {
            if (i->isChecked())
                m_DishesInCategory[i->text()].clear();
        }
        AdministratorWidget::menuChanged = true;
    }
    menuUpdate();
}

/*
 * 编辑菜品
 * 实现了普通状态下删除和搜索状态下编辑
 */
void menuManage::onMenuEdit()
{
    int index;
    if (m_search->text().count() != 0)//如果处于搜索状态
    {
        index = m_chosenDishes.indexOf(m_searchDishes[m_menuTable->currentRow()]);//通过搜索列表获取待编辑菜品的下标
    }
    else//如果不处于搜索状态
    {
        index = m_menuTable->currentRow();//直接获取下标
    }
    Dish &dish  = m_chosenDishes[index];//获取对应菜品

    auto editDialog = new QDialog(this);
    editDialog->setWindowTitle("编辑菜品");

    //界面控件添加与设置
    auto categoryLabel = new QLabel("菜品类别", this);
    categoryLabel->setMaximumWidth(50);
    m_categoryBox = new QComboBox(this);
    m_categoryBox->addItems(Dishes::categoryList);
    auto addCategory = new QPushButton("新建类别", this);
    auto name = new QLabel("菜品名称", this);
    m_nameInput = new QLineEdit(this);
    m_nameInput->setMaxLength(15);
    auto price = new QLabel("菜品价格", this);
    m_priceInput = new QLineEdit(this);
    m_priceInput->setMaxLength(15);
    QRegExp priceExp("^(([1-9]+)|([0-9]+\\.[0-9]{1,2}))$");
    m_priceInput->setValidator(new QRegExpValidator(priceExp));
    auto descriptionInput = new QTextEdit(this);
    auto description = new QLabel("菜品描述");
    m_errorMsg = new QLabel(this);
    m_submit = new QPushButton("提交", this);
    m_submit->setEnabled(false);
    auto cancel = new QPushButton("取消", this);

    //根据选中菜品设定控件内容
    m_categoryBox->setCurrentText(m_chosenDishes[index].category());
    m_nameInput->setText(m_chosenDishes[index].name());
    m_priceInput->setText(m_chosenDishes[index].priceToString());
    descriptionInput->setText(m_chosenDishes[index].description());

    //界面布局
    auto Layout = new QVBoxLayout(editDialog);
    auto hLayout = new QHBoxLayout;
    hLayout->addWidget(categoryLabel);
    hLayout->addWidget(m_categoryBox);
    hLayout->addWidget(addCategory);
    Layout->addLayout(hLayout);
    hLayout = new QHBoxLayout;
    hLayout->addWidget(name);
    hLayout->addWidget(m_nameInput);
    Layout->addLayout(hLayout);
    hLayout = new QHBoxLayout;
    hLayout->addWidget(price);
    hLayout->addWidget(m_priceInput);
    Layout->addLayout(hLayout);
    hLayout = new QHBoxLayout;
    hLayout->addWidget(description);
    hLayout->addWidget(descriptionInput);
    Layout->addLayout(hLayout);
    Layout->addWidget(m_errorMsg);
    hLayout = new QHBoxLayout;
    hLayout->addWidget(m_submit);
    hLayout->addWidget(cancel);
    Layout->addLayout(hLayout);

    m_nameInput->setFocus();

    connect(addCategory, SIGNAL(clicked(bool)), this, SLOT(onCategoryAdd()));
    connect(m_nameInput, SIGNAL(returnPressed()), m_priceInput, SLOT(setFocus()));
    connect(m_priceInput, SIGNAL(returnPressed()), descriptionInput, SLOT(setFocus()));
    //编辑信息合法性检查
    connect(m_categoryBox, &QComboBox::currentTextChanged, [=] ()
    {
        checkMenuValid(true, dish);
    });
    connect(m_nameInput, &QLineEdit::textChanged, [=] ()
    {
        checkMenuValid(true, dish);
    });
    connect(m_priceInput, &QLineEdit::textChanged, [=] ()
    {
        checkMenuValid(true, dish);
    });
    connect(descriptionInput, &QTextEdit::textChanged, [=] ()
    {
        checkMenuValid(true, dish);
    });
    connect(m_submit, SIGNAL(clicked(bool)), editDialog, SLOT(accept()));
    connect(cancel, SIGNAL(clicked(bool)), editDialog, SLOT(reject()));
    int re = editDialog->exec();
    if (re == QDialog::Accepted)
    {
        QString category = m_categoryBox->currentText();
        QString name = m_nameInput->text();
        QString price = m_priceInput->text();
        QString description = descriptionInput->toPlainText();
        if (description.count() == 0)
            description = "暂无描述";
        if (m_chosenDishes[index].category() != category)//如果类别改变
        {
            //将菜品从原有类别移除并添加到新类别
            m_DishesInCategory[m_chosenDishes[index].category()].remove(m_chosenDishes[index]);
            m_chosenDishes.edit(index, category, name, price.toDouble(), description);
            m_DishesInCategory[category].add(m_chosenDishes[index]);
        }
        else
        {
            int i = m_DishesInCategory[category].indexOf(m_chosenDishes[index]);
            m_DishesInCategory[category].edit(i, category, name, price.toDouble(), description);
        }

        AdministratorWidget::menuChanged = true;
        menuUpdate();
        if (m_search->text().count() != 0)//如果处于搜索状态
        {
            onMenuSearch(m_search->text());//重新搜索
        }
    }
}

/*
 * 菜单动态搜索
 * 搜索框内容变化时更新搜索列表
 * 同时可以实现搜索过程中的删除和编辑
 */
void menuManage::onMenuSearch(QString search)
{
    if (search == "")
    {
        m_searchMsg->setText("");
        menuUpdate();
        return;
    }

    m_searchDishes = m_chosenDishes.search(m_search->text());
    auto contents = m_searchDishes.toTableContents();
    if (m_searchDishes.count() == 0)
        m_searchMsg->setText("查询失败");
    else if (m_search->text().count() != 0)
    {
        m_searchMsg->setText("查询成功");
    }
    AdministratorWidget::showList(m_menuTable, contents);
}

/*
 * 检查菜品信息有效性
 * 分为添加状态和编辑状态
 * 添加状态下, 如果全部菜品中存在某个菜品, 该菜品的名称与名称输入框内容相同, 则禁止添加, 给出错误信息
 *
 * 编辑状态下, 如果全部菜品中存在某个菜品, 该菜品的名称与名称输入框内容相同,
 * 则需要判断名称输入框中内容是否和正在编辑菜品的名称相同
 * 如果相同, 则表示未更改菜品名称, 不应报错
 * 如果不同, 则表示试图将菜品名称改为另一个已添加的菜品的名称, 应该报错
 *
 * 添加时调用该函数时, 使用默认参数
 * 编辑时调用该函数时, 需指定editing = true, 并将当前正在编辑的菜品作为dish参数传入
 */
void menuManage::checkMenuValid(bool editing, const Dish &dish)
{
    bool editingCheck = true;//编辑状态下的检查值
    if (editing)//如果正在编辑, 判断正在编辑的菜品的名称和名称输入框的内容是否一致
    {
        editingCheck = (dish.name() != m_nameInput->text());
    }
    m_Dishes = m_DishesInCategory;//获取全部菜品

    /*添加状态下, 判断输入的名称是否已经存在
     * 编辑状态下, 如果输入的名称不是正在编辑菜品的名称, 判断修改后的名称是否已经存在
     */
    if (m_Dishes.contains(m_nameInput->text()) && editingCheck)
    {
        m_errorMsg->setText("菜品已存在");
        m_submit->setEnabled(false);
    }
    else if (m_nameInput->text().trimmed().count() == 0)
    {
        m_errorMsg->setText("名称不能为空");
    }
    else if (m_priceInput->text().count() == 0)
    {
        m_errorMsg->setText("价格不能为空");
        m_submit->setEnabled(false);
    }
    else
    {
        m_errorMsg->setText("");
        m_submit->setEnabled(true);
    }
}

//用户管理界面
usersManage::usersManage()
{
    for (int i=0; i<5; ++i)//提取所有用户信息
    {
        m_Users[i] = Users(i);
    }

    //用户管理相关控件
    m_add = new QPushButton("添加");
    m_delete = new QPushButton("删除");
    m_edit = new QPushButton("编辑");
    m_clear = new QPushButton("清空");
    m_search = new QLineEdit;
    m_search->setClearButtonEnabled(true);
    m_search->setPlaceholderText("搜索");
    m_searchMsg = new QLabel;
    m_usersTable = new QTableWidget;
    m_usersTable->setEditTriggers(QTableWidget::NoEditTriggers);
    m_vaccant = new QLabel("用户列表为空");
    QFont ft;
    ft.setPointSize(20);
    m_vaccant->setFont(ft);
    m_vaccant->setAlignment(Qt::AlignCenter);
    m_typeGroupBox = new QGroupBox("用户身份选择");
    m_typeGroupBox->setAlignment(Qt::AlignLeft);
    for (int i=0; i<5; ++i)
    {
        m_checkType.append(new QCheckBox(Identity[i], this));
    }
    m_allType = new QCheckBox("全选", this);

    //界面布局
    auto usergLayout = new QGridLayout;
    auto Layout = new QVBoxLayout(this);
    auto chooseLayout = new QHBoxLayout(m_typeGroupBox);
    for (int i=0; i<5; ++i)
    {
        chooseLayout->addWidget(m_checkType[i]);
    }
    chooseLayout->addWidget(m_allType);
    Layout->addWidget(m_typeGroupBox);
    usergLayout->addWidget(m_search, 0, 0, 1, 4);
    usergLayout->addWidget(m_searchMsg, 0, 4, 1, 1);
    usergLayout->addWidget(m_add, 1, 4, 1, 1);
    usergLayout->addWidget(m_delete, 2, 4, 1, 1);
    usergLayout->addWidget(m_edit, 3, 4, 1, 1);
    usergLayout->addWidget(m_clear, 4, 4, 1, 1);
    usergLayout->addWidget(m_vaccant, 1, 0, 4, 4);
    usergLayout->addWidget(m_usersTable, 1, 0, 4, 4);
    for (int i=0; i<5; ++i)
    {
        usergLayout->setRowMinimumHeight(i, 100);
    }
    Layout->addLayout(usergLayout);

    for (int i=0; i<5; ++i)//实现全选框的三态效果
    {
        connect(m_checkType[i], SIGNAL(stateChanged(int)), this, SLOT(usersUpdate()));
        connect(m_checkType[i], &QCheckBox::stateChanged, [=] ()
        {
            AdministratorWidget::onCheckChange(m_checkType, m_allType, m_checkType[i]);
        });
    }
    connect(m_allType, &QCheckBox::stateChanged, [=] ()
    {
        AdministratorWidget::onCheckChange(m_checkType, m_allType, m_allType);//实现全选框的三态效果
    });
    connect(m_add, SIGNAL(clicked(bool)), this, SLOT(onUsersAdd()));
    connect(m_clear, SIGNAL(clicked(bool)), this, SLOT(onUsersClear()));
    connect(m_delete, SIGNAL(clicked(bool)), this, SLOT(onUsersDelete()));
    connect(m_edit, SIGNAL(clicked(bool)), this, SLOT(onUsersEdit()));
    connect(m_search, SIGNAL(textChanged(QString)), this, SLOT(onUsersSearch(QString)));
    connect(m_add, SIGNAL(clicked(bool)), m_search, SLOT(clear()));
    connect(m_usersTable, &QTableWidget::cellClicked, [=] ()
    {
        m_delete->setEnabled(true);
        m_edit->setEnabled(true);
    });
    connect(m_usersTable, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(onUsersEdit()));

    m_checkType[0]->setChecked(true);
    usersUpdate();
}

//用户列表更新
void usersManage::usersUpdate()
{
    if (m_usersTable->currentRow()>=0 && m_usersTable->currentColumn()>=0)//如果有被选中的条目, 启用编辑和删除
    {
        m_edit->setEnabled(true);
        m_delete->setEnabled(true);
    }
    else//否则禁用编辑和删除
    {
        m_edit->setEnabled(false);
        m_delete->setEnabled(false);
    }
    m_chosenTypes.clear();
    m_chosenUsers.clear();

    for (int i=0; i<5; ++i)//获取选中的类别和对应的用户
    {
        if (m_checkType.at(i)->isChecked())
        {
            m_chosenTypes.append(i);
            m_chosenUsers.add(m_Users[i]);
        }
    }

    if (m_search->text().count() != 0)
    {
        onUsersSearch(m_search->text());
        return;
    }

    if (m_chosenUsers.count() == 0)
    {
        m_edit->setEnabled(false);
        m_clear->setEnabled(false);
        m_search->setEnabled(false);
        m_delete->setEnabled(false);
        m_add->setEnabled(true);
        m_vaccant->show();
        m_usersTable->hide();
    }
    else
    {
        m_usersTable->setColumnCount(4);
        m_search->setEnabled(true);
        m_clear->setEnabled(true);
        m_vaccant->hide();
        m_usersTable->show();
        QStringList label;
        label << "" << "身份" << "手机号" << "密码";
        m_usersTable->setHorizontalHeaderLabels(label);
        m_usersTable->verticalHeader()->setVisible(false);
        m_usersTable->setEditTriggers(QTableView::NoEditTriggers);
        m_usersTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        m_usersTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        m_usersTable->setRowCount(m_chosenUsers.count());
        m_usersTable->verticalHeader()->setVisible(false);

        QVector<QStringList> showContent = m_chosenUsers.toTableContents();
        AdministratorWidget::showList(m_usersTable, showContent);
    }
}

//添加用户
void usersManage::onUsersAdd()
{
    auto addDialog = new QDialog(this);
    addDialog->setFixedSize(250, 250);
    addDialog->setWindowTitle("添加用户");

    //相关控件的添加与设置
    auto label = new QLabel("添加用户身份为:");
    m_typeBox = new QComboBox(this);
    QStringList UsersType;
    UsersType << "顾客" << "管理员" << "厨师" << "服务员" << "经理";
    m_typeBox->addItems(UsersType);
    m_phonenumber = new QLineEdit(this);
    m_phonenumber->setPlaceholderText("请输入手机号");
    m_phonenumber->setMaxLength(11);
    QRegExp phoneNumberExp("1[0-9]{10}");
    m_phonenumber->setValidator(new QRegExpValidator(phoneNumberExp));
    m_password = new QLineEdit(this);
    m_password->setPlaceholderText("请输入密码");
    m_errorMsg = new QLabel;
    m_submit = new QPushButton("提交", this);
    m_submit->setEnabled(false);
    auto cancel = new QPushButton("取消", this);

    //界面布局
    auto Layout = new QVBoxLayout(addDialog);
    auto hLayout = new QHBoxLayout;
    hLayout->addWidget(label);
    hLayout->addWidget(m_typeBox);
    Layout->addLayout(hLayout);
    Layout->addStretch(1);
    Layout->addWidget(m_phonenumber);
    Layout->addWidget(m_password);
    Layout->addWidget(m_errorMsg);
    Layout->addStretch(1);
    hLayout = new QHBoxLayout;
    hLayout->addWidget(m_submit);
    hLayout->addWidget(cancel);
    Layout->addLayout(hLayout);

    m_phonenumber->setFocus();

    connect(m_typeBox, &QComboBox::currentTextChanged, [=] ()
    {
        m_phonenumber->setFocus();
        checkUserValid();
    });
    connect(m_phonenumber, SIGNAL(returnPressed()), m_password, SLOT(setFocus()));
    connect(m_phonenumber, SIGNAL(textChanged(QString)), this, SLOT(checkUserValid()));
    connect(m_password, SIGNAL(textChanged(QString)), this, SLOT(checkUserValid()));
    connect(m_submit, SIGNAL(clicked(bool)), addDialog, SLOT(accept()));
    connect(cancel, SIGNAL(clicked(bool)), addDialog, SLOT(reject()));

    if (addDialog->exec() == QDialog::Accepted)
    {
        QString phonenumber = m_phonenumber->text();
        QString password = m_password->text();
        int type = m_typeBox->currentIndex();
        int id = m_Users[type].maxId()+1;
        m_Users[type].add(User::getPointer(User(id, phonenumber, password, type)));

        AdministratorWidget::userChanged = true;
        usersUpdate();
    }
}

/*
 * 删除用户
 * 实现了普通状态下删除和搜索状态下删除
 */
void usersManage::onUsersDelete()
{
    User* deletedUser;
    if (m_search->text().count() != 0)//如果处于搜索状态
    {
        deletedUser = m_searchUsers[m_usersTable->currentRow()];//从搜索列表里获取待删除用户
    }
    else//如果处于普通状态
    {
        deletedUser = m_chosenUsers[m_usersTable->currentRow()]; //从已选类别的用户列表里获取待删除用户
    }
    m_Users[deletedUser->type()].remove(deletedUser);
    AdministratorWidget::userChanged = true;
    usersUpdate();
    if (m_search->text() != 0)//如果处于搜索状态
    {
        onUsersSearch(m_search->text());//重新搜索
    }
}

//清空已选类别的用户列表
void usersManage::onUsersClear()
{
    auto Msg = new QMessageBox(QMessageBox::Warning, \
                               "清空用户列表", "您确认要清空用户列表?",\
                               QMessageBox::Ok | QMessageBox::Cancel);
    Msg->setButtonText(QMessageBox::Ok, "确认");
    Msg->setButtonText(QMessageBox::Cancel, "取消");

    if (Msg->exec() == QMessageBox::Ok)
    {
        for (int i=0;i<m_chosenTypes.count();++i)
        {
            m_Users[m_chosenTypes[i]].clear();
        }
        AdministratorWidget::userChanged = true;
    }
    usersUpdate();
}

/*
 * 编辑菜品
 * 实现了普通状态下删除和搜索状态下编辑
 */
void usersManage::onUsersEdit()
{
    auto editDialog = new QDialog(this);
    editDialog->setWindowTitle("编辑用户");

    //获取待编辑用户
    int index;
    int type;
    if (m_search->text().count() != 0)//如果处于搜索状态, 从搜索列表中获取
    {
        type = m_searchUsers[m_usersTable->currentRow()]->type();
        index = m_Users[type].indexOf(m_searchUsers[m_usersTable->currentRow()]);
    }
    else//如果处于普通状态, 从已选类别的用户列表中获取
    {
        type = m_chosenUsers[m_usersTable->currentRow()]->type();
        index = m_Users[type].indexOf(m_chosenUsers[m_usersTable->currentRow()]);
    }
    User *user = m_Users[type][index];

    //相关控件添加与初始化
    m_submit = new QPushButton("提交");
    m_submit->setEnabled(false);
    auto cancel = new QPushButton("取消");
    auto phonenumber = new QLabel("手机号");
    auto code = new QLabel("密  码");
    m_phonenumber = new QLineEdit(this);
    m_phonenumber->setMaxLength(11);
    QRegExp phoneNumberExp("1[0-9]{10}");
    m_phonenumber->setValidator(new QRegExpValidator(phoneNumberExp));
    m_password = new QLineEdit(this);
    m_password->setMaxLength(16);
    m_errorMsg = new QLabel(this);

    //手机号、密码输入框设定初始值
    m_phonenumber->setText(user->phonenumber());
    m_password->setText(user->password());

    //界面布局
    auto Layout = new QVBoxLayout(editDialog);
    auto hLayout = new QHBoxLayout;
    hLayout->addWidget(phonenumber);
    hLayout->addWidget(m_phonenumber);
    Layout->addLayout(hLayout);
    hLayout = new QHBoxLayout;
    hLayout->addWidget(code);
    hLayout->addWidget(m_password);
    Layout->addLayout(hLayout);
    Layout->addWidget(m_errorMsg);
    hLayout = new QHBoxLayout;
    hLayout->addWidget(m_submit);
    hLayout->addWidget(cancel);
    Layout->addLayout(hLayout);

    m_phonenumber->setFocus();

    connect(m_phonenumber, SIGNAL(returnPressed()), m_password, SLOT(setFocus()));
    //检查编辑是否有效
    connect(m_phonenumber, &QLineEdit::textChanged, [=] ()
    {
        checkUserValid(true, user);
    });
    connect(m_password, &QLineEdit::textChanged, [=] ()
    {
        checkUserValid(true, user);
    });

    connect(m_submit, SIGNAL(clicked(bool)), editDialog, SLOT(accept()));
    connect(cancel, SIGNAL(clicked(bool)), editDialog, SLOT(close()));

    if (editDialog->exec() == QDialog::Accepted)
    {
        m_Users[type].edit(index, m_phonenumber->text(), m_password->text());
        AdministratorWidget::userChanged = true;
        usersUpdate();

        if (m_search->text().count() != 0)//如果处于搜索状态中
        {
            onUsersSearch(m_search->text());//重新搜索
        }
    }
}

//搜索用户
void usersManage::onUsersSearch(QString search)
{
    if (search == "")
    {
        m_searchMsg->setText("");
        usersUpdate();
        return;
    }
    m_searchUsers = m_chosenUsers.search(search);
    auto showSearch = m_searchUsers.toTableContents();
    AdministratorWidget::showList(m_usersTable, showSearch);

    if (m_searchUsers.count() == 0&&m_search->text().count() != 0)
        m_searchMsg->setText("查询失败");
    else if (m_search->text().count() != 0)
        m_searchMsg->setText("查询成功");
}

/*
 * 检查用户信息有效性
 * 分为添加状态和编辑状态
 * 添加状态下, 如果该类别用户中存在某个用户, 该用户的手机号与手机号输入框内容相同, 则禁止添加, 给出错误信息
 *
 * 编辑状态下, 如果该类别用户中存在某个用户, 该用户的手机号与手机号输入框内容相同,
 * 则需要判断手机号输入框中内容是否和正在编辑用户的手机号相同
 * 如果相同, 则表示未更改用户手机号, 不应报错
 * 如果不同, 则表示试图将手机号修改为另一个相同类别的已注册过的手机号, 应该报错
 *
 * 添加时调用该函数时, 使用默认参数
 * 编辑时调用该函数时, 需指定editing = true, 并将当前正在编辑的用户指针作为user参数传入
 */
void usersManage::checkUserValid(bool editing, User *user)
{
    bool editingCheck = true;//编辑状态下的检查值
    int type;
    if (user)
    {
        type = user->type();
    }
    else
    {
        type = m_typeBox->currentIndex();
    }
    if (editing)//如果正在编辑, 判断正在编辑的用户的手机号和手机号输入框的内容是否一致
    {
        editingCheck = (user->phonenumber() != m_phonenumber->text());
    }
    if (m_phonenumber->text().count() != 11)
    {
        m_submit->setEnabled(false);
        m_errorMsg->setText("请输入正确的手机号");
    }
    /*添加状态下, 判断在该类别用户中输入的手机号是否已经存在
     * 编辑状态下, 如果输入的手机号不是正在编辑用户的手机号, 判断修改后的手机号在同类别用户中是否已经存在
     */
    else if (m_Users[type].contains(m_phonenumber->text()) && editingCheck)
    {
        m_submit->setEnabled(false);
        m_errorMsg->setText("用户已注册");
    }
    else if (m_password->text().count() < 6)
    {
        m_submit->setEnabled(false);
        m_errorMsg->setText("请输入6-16位密码");
    }
    else
    {
        m_submit->setEnabled(true);
        m_errorMsg->setText("");
    }
}

//餐桌管理界面
tableManage::tableManage()
{
    m_diningTables = DiningTables();//提取所有餐桌信息
    //添加并设置界面控件
    auto label1 = new QLabel("当前餐桌总数:");
    m_table = new QTableWidget;
    m_table->setColumnCount(2);
    m_table->verticalHeader()->setVisible(false);
    QStringList headerLabel;
    headerLabel<<"餐桌序号"<<"餐桌状态";
    m_table->setHorizontalHeaderLabels(headerLabel);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setEditTriggers(QTableView::NoEditTriggers);
    m_tableNumber = new QLabel(this);
    auto edit = new QPushButton("更改");
    auto label2 = new QLabel("管理餐桌状态:");
    auto able = new QCheckBox("空闲");
    auto unable = new QCheckBox("禁用");
    auto occupied = new QCheckBox("顾客占用");
    able->setCheckState(Qt::Checked);
    able->setEnabled(false);
    unable->setCheckState(Qt::Unchecked);
    unable->setEnabled(false);
    occupied->setCheckState(Qt::PartiallyChecked);
    occupied->setEnabled(false);

    //界面布局
    auto hLayout = new QHBoxLayout;
    auto Layout = new QVBoxLayout(this);
    hLayout->addWidget(label1);
    hLayout->addWidget(m_tableNumber);
    hLayout->addWidget(edit);
    hLayout->addStretch(1);
    Layout->addLayout(hLayout);
    hLayout = new QHBoxLayout;
    hLayout->addWidget(label2);
    hLayout->addWidget(able);
    hLayout->addWidget(unable);
    hLayout->addWidget(occupied);
    hLayout->stretch(1);
    Layout->addLayout(hLayout);
    Layout->addWidget(m_table);
    connect(edit, SIGNAL(clicked(bool)), this, SLOT(onEdit()));

    tableListUpdate();
}

//编辑餐桌数量
void tableManage::onEdit()
{
    auto editDialog = new QDialog(this);
    editDialog->setWindowTitle("修改餐桌数");
    //相关控件的添加与设置
    auto label = new QLabel("餐桌数量:");
    auto tableNumber = new QLineEdit(m_tableNumber);
    tableNumber->setFocus();
    QRegExp numExp("([1-9]{1}[0-9]{0,3})|[0]");
    tableNumber->setValidator(new QRegExpValidator(numExp));
    auto submit = new QPushButton("确定");
    submit->setEnabled(false);
    auto cancel = new QPushButton("取消");

    //界面布局
    auto editLayout = new QVBoxLayout(editDialog);
    auto editHLayout = new QHBoxLayout;
    editHLayout->addWidget(label);
    editHLayout->addWidget(tableNumber);
    editLayout->addLayout(editHLayout);
    editHLayout = new QHBoxLayout;
    editHLayout->addStretch(1);
    editHLayout->addWidget(submit);
    editHLayout->addWidget(cancel);
    editLayout->addLayout(editHLayout);

    connect(tableNumber, &QLineEdit::textChanged, [=] ()
    {
        if (tableNumber->text() != "")
            submit->setEnabled(true);
        else
            submit->setEnabled(false);
    });
    connect(tableNumber, SIGNAL(returnPressed()), submit, SLOT(setFocus()));
    connect(submit, SIGNAL(clicked(bool)), editDialog, SLOT(accept()));
    connect(cancel, SIGNAL(clicked(bool)), editDialog, SLOT(reject()));
    int re = editDialog->exec();
    if (re == QDialog::Accepted)
    {
        /*
         * 设置餐桌数量
         * 如果减少餐桌数量, 且减少的餐桌恰好有顾客占用, 则设置餐桌数量失败
         */
        if (!m_diningTables.setTableNumber(tableNumber->text().toInt()))
        {
            /*
             * 如果设置失败, 可以选择强制修改或取消修改
             */
            auto ErrorMsg = new QMessageBox(QMessageBox::Warning,\
                                            "编辑失败", "有顾客正在用餐, 不能更改餐桌数量!",\
                                            QMessageBox::Ok|QMessageBox::Cancel);
            ErrorMsg->setButtonText(QMessageBox::Ok, "强制修改");
            ErrorMsg->setButtonText(QMessageBox::Cancel, "取消修改");
            if (ErrorMsg->exec() == QMessageBox::Ok)
            {
                m_diningTables.setTableNumber(tableNumber->text().toInt(), true);
                m_tableNumber->setText(tableNumber->text());
            }
            else
            {
                ErrorMsg->close();
            }
        }//end of if
        else
        {
            m_tableNumber->setText(tableNumber->text());
        }
        AdministratorWidget::tableChanged = true;
    }
    tableListUpdate();
}

//改变餐桌状态(空闲或禁用)
void tableManage::onStateChange()
{
    QCheckBox*Sender = qobject_cast<QCheckBox*>(sender());
    if (Sender)
    {
        QString state;
        if (Sender->checkState() == Qt::Checked)
            state = "空闲";
        else if (Sender->checkState() == Qt::Unchecked)
            state = "禁用";
        m_diningTables[m_checkTables.indexOf(Sender)].setState(state);
        AdministratorWidget::tableChanged = true;
    }
    tableListUpdate();
}

//餐桌列表更新
int tableManage::tableListUpdate()
{
    m_checkTables.clear();
    m_tableNumber->setText(QString::number(m_diningTables.count()));
    m_table->setRowCount(m_diningTables.count());

    for (int i=0; i<m_diningTables.count(); ++i)
    {
        m_checkTables.append(new QCheckBox(QString("%1号餐桌").arg(i+1)));
        m_checkTables[i]->setTristate(false);

        if (m_diningTables[i].tableState() == "空闲")
        {
            m_checkTables[i]->setCheckState(Qt::Checked);
        }
        else if (m_diningTables[i].tableState() == "禁用")
        {
            m_checkTables[i]->setCheckState(Qt::Unchecked);
        }
        else//顾客占用的餐桌不可编辑状态
        {
            m_checkTables[i]->setTristate(true);
            m_checkTables[i]->setCheckState(Qt::PartiallyChecked);
            m_checkTables[i]->setEnabled(false);
        }

        m_table->setCellWidget(i, 0, m_checkTables[i]);
        m_table->setItem(i, 1, new QTableWidgetItem(m_diningTables[i].tableState()));
        connect(m_checkTables[i], SIGNAL(stateChanged(int)), this, SLOT(onStateChange()));
    }
    return m_diningTables.count();
}
