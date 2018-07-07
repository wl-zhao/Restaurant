#include "SignWidget.h"
#include "user.h"
#include "AdministratorWidget.h"
#include "CustomerWidget.h"
#include "WaiterWidget.h"
#include "CookWidget.h"
#include "ManagerWidget.h"

bool SignWidget::useDataBase = true;

SignWidget::SignWidget(int type) : m_type(type)
{
    m_state = false;        //默认初始为登录界面
    copyRightClick = 0;     //将版权声明按钮点击次数置零

    m_Users = new Users[5];
    m_Users[m_type]=Users(m_type);      //提取用户信息(默认初始m_type = 0为顾客登录界面)

    setUp();
}

void SignWidget::setUp()
{
    //窗口基本属性
    setWindowFlags(Qt::FramelessWindowHint | windowFlags());
    this->setAttribute(Qt::WA_TranslucentBackground);
    TitleBar *titleBar = new TitleBar(this);
    installEventFilter(titleBar);
    setFixedSize(340, 380);//350, 350
    setWindowTitle("餐厅服务与管理系统");
    setWindowIcon(QIcon(":/icon.ico"));

    //用户类别选择相关控件添加与设置
    auto label1 = new QLabel("当前用户身份");
    label1->setFixedWidth(75);
    m_typeButton = new QPushButton(" 顾客");
    m_typeButton->setFlat(true);
    m_typeButton->setStyleSheet("QPushButton{height:25;text-align:left;}");
    m_typeButton->setFixedWidth(60);
    m_typeBox = new QComboBox(this);
    m_typeBox->setFixedWidth(60);
    QStringList userType;
    userType << "管理员" << "经理" << "厨师" << "服务员" <<"顾客";
    m_typeBox->addItems(userType);
    m_typeBox->setCurrentText(Identity[m_type]);
    m_typeBox->setVisible(false);
    m_typeBox->setStyleSheet("QComboBox{text-align:center;height:25;border:none;}");

    //用户类别选择布局
    m_userTypeLayout = new QHBoxLayout;
    m_userTypeLayout->addSpacing(20);
    m_userTypeLayout->addWidget(label1, Qt::AlignLeft);
    m_userTypeLayout->addWidget(m_typeButton, Qt::AlignLeft);
    m_userTypeLayout->addStretch(1);

    //其它控件的添加与设置
    m_phonenumberInput = new QLineEdit(this);
    QRegExp phonenumberExp("1[0-9]{10}");
    m_phonenumberInput->setValidator(new QRegExpValidator(phonenumberExp));
    m_phonenumberInput->setPlaceholderText("手机号");
    m_passwordInput = new QLineEdit(this);
    m_passwordInput->setEchoMode(QLineEdit::Password);
    m_passwordInput->setMaxLength(16);
    m_passwordInput->setPlaceholderText("密码");
    m_rePasswordInput = new QLineEdit(this);
    m_rePasswordInput->setEchoMode(QLineEdit::Password);
    m_rePasswordInput->setMaxLength(16);
    m_rePasswordInput->setPlaceholderText("请确认密码");
    m_rePasswordInput->setVisible(false);
    m_sign = new QPushButton("登 录");
    m_sign->setEnabled(false);
    m_errorMsg = new QLabel;
    m_signInOrUp = new QPushButton("新用户?点击注册");
    m_signInOrUp->setFlat(true);
    m_copyRight = new QPushButton("Copyright 2017 by John Williams", this);
    m_copyRight->setFlat(true);
    m_copyRight->setStyleSheet("QPushButton{background:rgb(235, 242, 249);color:rgb(0, 155, 188);}");

    //登录主界面布局
    auto Layout = new QVBoxLayout;
    Layout->addWidget(m_phonenumberInput);
    Layout->addWidget(m_passwordInput);
    Layout->addWidget(m_rePasswordInput);
    Layout->addWidget(m_errorMsg);
    Layout->addWidget(m_sign);
    Layout->setContentsMargins(20, 0, 20, 0);
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(titleBar);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(m_userTypeLayout);
    mainLayout->addSpacing(78);
    mainLayout->addLayout(Layout);
    Layout = new QVBoxLayout;
    Layout->setContentsMargins(20, 0, 20, 5);
    Layout->addWidget(m_signInOrUp);
    Layout->addSpacing(5);
    Layout->addWidget(m_copyRight);
    mainLayout->addLayout(Layout);
    mainLayout->setContentsMargins(5, 5, 5, 5);

    connect(m_typeButton, SIGNAL(clicked(bool)), this, SLOT(onChangeType()));
    connect(m_typeBox, SIGNAL(currentTextChanged(QString)), this, SLOT(onChangeType()));

    //每次输入检查有效性
    connect(m_phonenumberInput, &QLineEdit::textChanged, [&]()
    {
        m_phonenumber = m_phonenumberInput->text();
        checkValid();
    });
    connect(m_passwordInput, &QLineEdit::textChanged, [&]()
    {
        m_password = m_passwordInput->text();
        checkValid();
    });
    connect(m_rePasswordInput, &QLineEdit::textChanged, [&]()
    {
        m_rePassword = m_rePasswordInput->text();
        checkValid();
    });

    connect(m_signInOrUp, SIGNAL(clicked(bool)), this, SLOT(changeSignState()));
    connect(m_phonenumberInput, SIGNAL(returnPressed()), m_passwordInput, SLOT(setFocus()));
    connect(m_passwordInput, SIGNAL(textChanged(QString)), this, SLOT(checkValid()));

    connect(m_passwordInput, &QLineEdit::returnPressed, [=]()
    {
        if (m_rePasswordInput->isVisible())
        {
            m_rePasswordInput->setFocus();
        }
        else
        {
            m_sign->click();
        }
    });
    connect(m_rePasswordInput, &QLineEdit::returnPressed, [=] (){
        if (checkValid())
        {
            m_sign->click();
        }
    });
    connect(m_sign, SIGNAL(clicked(bool)), this, SLOT(onSign()));
    connect(m_copyRight, &QPushButton::clicked, [=] ()
    {
        copyRightClick++;
        if (copyRightClick == 5)
        {
            if (useDataBase)//如果当前为多App模式
            {
                QMessageBox*msg = new QMessageBox(QMessageBox::Information, "单App模式设置成功", "运行期间不使用数据库", QMessageBox::Ok, this);
                msg->setButtonText(QMessageBox::Ok, "确定");
                msg->exec();

                //提取数据库信息至内存
                for (int i=0; i<5; ++i)
                {
                    Users::UsersData[i] = Users(i);
                }
                Dishes::MenuData = Dishes(0);
                Dishes::MenuDataInCategory = Dishes::MenuData.categoryHash();
                DiningTables::TablesData = DiningTables();
                Dishes::DishesData.clear();
                for (int i=0; i<DiningTables::TablesData.count(); ++i)
                {
                    Dishes::DishesData.append(Dishes(i+1));
                }

                //设置不使用数据库
                useDataBase = false;

                m_phonenumberInput->setFocus();
            }//end of if
            else//如果当前为单App模式
            {
                useDataBase = true;
                QMessageBox*msg = new QMessageBox(QMessageBox::Information, "多App模式设置成功", "运行期间使用数据库", QMessageBox::Ok, this);
                msg->setButtonText(QMessageBox::Ok, "确定");
                msg->exec();
            }
            copyRightClick = 0;     //重置
        }//end of if
    });

    m_phonenumberInput->setFocus();
    wmsender = WMSender(this);
    wmsender.send("administrator", "new");   //通知管理员界面更新接收者信息
}

void SignWidget::mousePressEvent(QMouseEvent* event)
{
    dragEffect(this, event);//界面拖动效果
}

void SignWidget::paintEvent(QPaintEvent *event)
{
    paintEffect(this, event);//绘制圆角、阴影、背景
}

/*
 * 单App模式下若登录界面关闭
 * 将此时的运行数据保存到数据库
 */
void SignWidget::closeEvent(QCloseEvent *event)
{
    if (!useDataBase)//如果当前为单App模式
    {
        useDataBase = true;                 //改为使用数据库
        wmsender.updateReceivers();         //更新信息接收者
        wmsender.send("all", "close");       //全部关闭

        //保存数据到数据库
        Dishes::MenuData.save();
        Dishes::saveCategory();
        for (int i=0; i<5; ++i)
        {
            Users::UsersData[i].save();
        }
        DiningTables::TablesData.save();
        for (Dishes &i: Dishes::DishesData)
        {
            i.save();
        }
    }
}

bool SignWidget::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    QString msg = WMSender::msgProcess(eventType, message, result);
    if (msg == "update")
    {
        update();
    }
    return QWidget::nativeEvent(eventType, message, result);
}

//检查输入信息是否有效
bool SignWidget::checkValid()
{
    if (!m_phonenumberInput->text().count()
            && !m_passwordInput->text().count()
            && !m_rePasswordInput->text().count())//如果输入框均为空
    {
        m_sign->setEnabled(false);      //禁用登录(注册)按钮
        m_errorMsg->setText("");        //不显示错误信息
        return false;
    }
    if (!m_state)//如果处于登录状态
    {
        if (m_phonenumber.count()<11)
        {
            m_errorMsg->setText("请输入正确的手机号");
            m_sign->setEnabled(false);
            return false;
        }
        else if (!m_Users[m_type].contains(m_phonenumber))
        {
            m_errorMsg->setText("用户未注册");
            m_sign->setEnabled(false);
            return false;
        }
        else
        {
            m_errorMsg->setText("");
            m_sign->setEnabled(true);
            return true;
        }
    }//end of if
    else//如果处于注册界面
    {
        if (m_phonenumber.count()<11)
        {
            m_errorMsg->setText("请输入正确的手机号");
            m_sign->setEnabled(false);
            return false;
        }
        else if (m_Users[m_type].contains(m_phonenumber))
        {
            m_errorMsg->setText("该手机号已被注册");
            m_sign->setEnabled(false);
            return false;
        }
        else if (m_password.count()<6)
        {
            m_errorMsg->setText("请输入6-16位密码");
            m_sign->setEnabled(false);
            return false;
        }
        else if (m_rePassword != m_password)
        {
            m_errorMsg->setText("两次密码不一致");
            m_sign->setEnabled(false);
            return false;
        }
        else
        {
            m_errorMsg->setText("");
            m_sign->setEnabled(true);
            return true;
        }
    }//end of else
}

//登录或注册
void SignWidget::onSign()
{
    if (!m_state)//如果处于登录状态
    {
        auto user = m_Users[m_type].userValue(m_phonenumber);//获取对应用户的指针

        if(user && (user->password() == m_password))    //如果用户存在且密码正确
        {
            if (useDataBase)//如果为多App模式
            {
                this->close();
            }

            //根据用户类别进入不同的界面
            switch (m_type)
            {
            case 0:
            {
                Customer* customer = dynamic_cast<Customer *>(user);//new Customer(*((Customer*)Users(0)[userId-1]));
                auto c = new CustomerWidget(customer);
                c->show();
                break;
            }
            case 1:
            {
                auto a = new AdministratorWidget(this);
                a->show();
                break;
            }
            case 2:
            {
                Cook* cook = dynamic_cast<Cook *>(user);//new Cook(*((Cook*)Users(2)[userId-1]));
                auto c = new CookWidget(cook);
                c->show();
                break;
            }
            case 3:
            {
                Waiter* waiter = dynamic_cast<Waiter *>(user);//new Waiter(*((Waiter*)Users(3)[userId-1]));
                auto w = new WaiterWidget(waiter);
                w->show();
                break;
            }
            case 4:
            {
                auto manager = new ManagerWidget;
                manager->show();
                break;
            }
            }

            //更新接收者并发消息通知所有相关界面由新用户登录
            wmsender.updateReceivers();
            wmsender.send("all", "new");
        }//end of if
        else//否则登录失败
        {
            QMessageBox failSignIn(QMessageBox::Information, "登录失败!", "密码错误!");
            failSignIn.setButtonText(QMessageBox::Ok, "确定");
            failSignIn.exec();
            m_passwordInput->clear();
        }
        return;
    }//end of if
    else//处于注册状态
    {
        if (!m_Users[m_type].contains(m_phonenumber))//如果该号码未注册
        {
            //获取该用户的指针, 便于实现多态
            User *user = User::getPointer(User(0, m_phonenumber, m_password, m_type));
            user->insert();     //注册该用户

            int id = user->id();
            m_Users[m_type].add(user);          //添加将用户信息

            auto Msg = new QMessageBox(QMessageBox::Information, "注册成功",\
                                       QString("恭喜您成为第%1位%2").arg(id).arg(Identity[m_type]),\
                                       QMessageBox::Ok | QMessageBox::Cancel);
            Msg->setButtonText(QMessageBox::Ok, "立即登录");
            Msg->setButtonText(QMessageBox::Cancel, "取消");
            if (Msg->exec() == QMessageBox::Ok)
            {
                //将状态改为登录
                changeSignState();
                onSign();

                //清空输入框
                m_phonenumberInput->clear();
                m_passwordInput->clear();
                m_rePasswordInput->clear();
            }
            else
            {
                checkValid();
            }
        }
    }
}

//用户类别改变
void SignWidget::onChangeType()
{
    /*
     * 如果类别选择按钮可见
     * 即当前用户为顾客
     */
    if (m_typeButton->isVisible())
    {
        m_typeBox->setVisible(true);                                //类别选择下拉框设为可见
        m_userTypeLayout->replaceWidget(m_typeButton, m_typeBox);   //用下拉框替换按钮
        m_typeBox->setCurrentIndex(0);                              //默认当前下表为0(管理员)
        m_type = 1;                                                 //设定用户类型为管理员
        m_typeButton->setVisible(false);                            //类别选择按钮设为不可见
    }

    /*
     * 否则当前类别选择下拉框可见
     * 如果下拉框下标为4
     * 表示重新点击了顾客
     */
    else if (m_typeBox->currentIndex() == 4)
    {
        //重置用户类别为顾客, 并替换控件
        m_typeButton->setVisible(true);
        m_userTypeLayout->replaceWidget(m_typeBox, m_typeButton);
        m_typeBox->setVisible(false);
        m_type = 0;
    }
    else //否则获取当前选中类别的标号
    {
        for (int i=0; i<5; ++i)
        {
            if (Identity[i] == m_typeBox->currentText())//如果当前文本和某一身份匹配
            {
                m_type = i;//确认类别标号
                break;
            }
        }//end of for
    }

    m_phonenumberInput->setFocus();
    if (m_Users[m_type].count() == 0)//如果该类型用户为空, 重新提取
    {
        m_Users[m_type] = Users(m_type);
    }

    checkValid();
}

//改变登录或注册状态
void SignWidget::changeSignState()
{
    if (m_state == false)//如果处于登录状态
    {
        m_rePasswordInput->clear();
        m_rePasswordInput->setVisible(true);
        m_sign->setText("注 册");
        m_sign->setEnabled(false);
        m_signInOrUp->setText("已有账号?点击登录");
        m_phonenumberInput->setFocus();
        m_state = true;
    }
    else//如果处于注册状态
    {
        m_rePasswordInput->clear();
        m_rePasswordInput->setVisible(false);
        m_sign->setText("登 录");
        m_sign->setEnabled(false);
        m_signInOrUp->setText("新用户?点击注册");
        m_phonenumberInput->setFocus();
        m_state = false;
    }

    checkValid();
}

//更新用户信息
void SignWidget::update()
{
    for (int i=0; i<5; i++)
    {
        m_Users[i] = Users(i);
    }
    checkValid();
}
