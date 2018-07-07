#ifndef SIGNWIDGET_H
#define SIGNWIDGET_H
#include "stable.h"
#include "user.h"
#include "WinMessage.h"

class Users;

/*
 * 开始界面
 * 完成登录和注册
 */
class SignWidget:public QWidget
{
    Q_OBJECT

public:
    SignWidget(int type=0);
    ~SignWidget(){}
    void setUp();
    void update();      //更新用户数据

    //界面拖动、界面绘制、界面关闭、接受消息
    void mousePressEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);
    void closeEvent(QCloseEvent *event);
    bool nativeEvent(const QByteArray &eventType, void *message, long *result);

    static bool useDataBase;        //标识当前模式, 是否使用数据库

protected slots:  
    bool checkValid();              //检查输入有效性
    void onSign();                  //注册或登录
    void onChangeType();            //改变用户类别
    void changeSignState();         //切换登录和注册

private:
    int m_type;                     //用户类型 : 从0至4依次为 : 顾客、管理员、厨师、服务员、经理
    bool m_state;                   //当前界面状态,false为默认值登录界面,true为注册界面
    QHBoxLayout *m_userTypeLayout;  //用户类型布局
    QString m_phonenumber;
    QString m_password;
    QString m_rePassword;
    QLabel *m_errorMsg;
    QPushButton *m_sign;
    QPushButton *m_typeButton;       //更改用户类别按钮
    QComboBox *m_typeBox;            //更改用户类别下拉框
    Users *m_Users;                //存储所有用户
    QPushButton *m_signInOrUp;       //切换登录状态
    QLineEdit *m_phonenumberInput;   //手机号输入框
    QLineEdit *m_passwordInput;      //密码输入框
    QLineEdit *m_rePasswordInput;    //重输密码框
    QPushButton *m_copyRight;        //版权声明按钮

    /*
     * 记录版权声明被点击的次数
     * 点击版权声明5次可以在多App和单App模式间切换
     * 同时更改useDataBase的值
     */
    int copyRightClick;

    WMSender wmsender;               //Windows 消息发送器
};
#endif // SIGNWIDGET_H
