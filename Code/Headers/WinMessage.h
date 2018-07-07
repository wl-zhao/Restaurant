#ifndef WINWM_H
#define WINWM_H
#ifdef Q_OS_WIN
#pragma comment(lib, "user32.lib")
#include <qt_windows.h>
#endif
#include <QWidget>
#include <windows.h>
#include <QString>
#include <QHash>
#include <QVector>
#include <QObject>
const ULONG_PTR CUSTOM_TYPE = 10000;

//Windows消息发送器
class WMSender:public QObject
{
    Q_OBJECT

public:
    WMSender(QWidget *parent=0);
    ~WMSender(){}
    WMSender& operator=(const WMSender&);
    void send(QString receiverName,QString message="");
    void setMessage(QString message)
    { msg = message; }
    static QString msgProcess(const QByteArray &eventType, void *message, long *result);    //处理信息

    //存储各界面对应的窗体句柄
    static QVector<HWND> signHwnds;
    static QVector<HWND> customerHwnds;
    static QVector<HWND> administratorHwnds;
    static QVector<HWND> cookHwnds;
    static QVector<HWND> waiterHwnds;
    static QVector<HWND> managerHwnds;

private:

    QWidget *m_parent;
    QHash<QString,QVector<HWND>> hwnds;
    static WINBOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);//列举窗体
    QString msg;

public slots:
    void updateReceivers();
};
#endif // WINWM_H
