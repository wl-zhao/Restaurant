#include "WinMessage.h"

QVector<HWND> WMSender::signHwnds = QVector<HWND>();
QVector<HWND> WMSender::customerHwnds = QVector<HWND>();
QVector<HWND> WMSender::administratorHwnds = QVector<HWND>();
QVector<HWND> WMSender::waiterHwnds = QVector<HWND>();
QVector<HWND> WMSender::cookHwnds = QVector<HWND>();
QVector<HWND> WMSender::managerHwnds = QVector<HWND>();

WMSender::WMSender(QWidget *parent) : m_parent(parent),msg("")
{
    updateReceivers();
}

WMSender &WMSender::WMSender::operator=(const WMSender &sender)
{
    m_parent = sender.m_parent;
    updateReceivers();
}
void WMSender::send(QString receiverName, QString message)
{
    setMessage(message);
    if (hwnds.contains(receiverName))
    {
        QByteArray data = msg.toUtf8();

        COPYDATASTRUCT copydata;
        copydata.dwData = CUSTOM_TYPE;  // 用户定义数据
        copydata.lpData = data.data();  //数据大小
        copydata.cbData = data.size();  // 指向数据的指针

        HWND sender = (HWND)(m_parent->effectiveWinId());
        for (int i=0; i<hwnds[receiverName].count(); ++i)
        {
            if ((HWND)(sender) != hwnds[receiverName][i])
                ::SendMessage(hwnds[receiverName][i],WM_COPYDATA,\
                              reinterpret_cast<WPARAM>(sender),reinterpret_cast<LPARAM>(&copydata));
        }
        return;
    }
    else if (receiverName == "all")
    {
        for (QString &Key : hwnds.keys())
        {
            send(Key, message);
        }
    }
}

void WMSender::updateReceivers()
{
    customerHwnds.clear();
    administratorHwnds.clear();
    waiterHwnds.clear();
    cookHwnds.clear();
    managerHwnds.clear();
    EnumWindows(&WMSender::EnumWindowsProc, NULL);

    hwnds["sign"] = signHwnds;
    hwnds["customer"] = customerHwnds;
    hwnds["administrator"] = administratorHwnds;
    hwnds["waiter"] = waiterHwnds;
    hwnds["cook"] = cookHwnds;
    hwnds["manager"] = managerHwnds;
}

WINBOOL WMSender::EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    wchar_t title[80];
    ::GetWindowText(hwnd,title,sizeof(title));
    QString qTitle = QString::fromStdWString(title);

    //获取各窗体的句柄
    if (qTitle == "餐厅服务与管理系统")
    {
        signHwnds.append(hwnd);
    }
    else if (qTitle == "顾客界面")
    {
        customerHwnds.append(hwnd);
    }
    else if (qTitle == "管理员界面")
    {
        administratorHwnds.append(hwnd);
    }
    else if (qTitle == "服务员界面")
    {
        waiterHwnds.append(hwnd);
    }
    else if (qTitle == "厨师界面")
    {
        cookHwnds.append(hwnd);
    }
    else if (qTitle == "经理界面")
    {
        managerHwnds.append(hwnd);
    }

    return TRUE;
}

//消息处理, 解码后返回
QString WMSender::msgProcess(const QByteArray &eventType, void *message, long *result)
{
    MSG *param = static_cast<MSG *>(message);

    switch (param->message)
    {
    case WM_COPYDATA:
    {
        COPYDATASTRUCT *cds = reinterpret_cast<COPYDATASTRUCT*>(param->lParam);
        if (cds->dwData == CUSTOM_TYPE)
        {
            QString strMessage = QString::fromUtf8(reinterpret_cast<char*>(cds->lpData), cds->cbData);
            *result = 1;
            return strMessage;
        }
    }
    }
    return "";  //默认返回空字符串
}
