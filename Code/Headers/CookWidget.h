#ifndef COOKWIDGET_H
#define COOKWIDGET_H
#include "stable.h"
#include "dish.h"
#include "user.h"
#include "diningtable.h"
#include "beautify.h"
#include "WinMessage.h"

class CookWidget : public QWidget
{
    Q_OBJECT

public:
    CookWidget(Cook *cook = 0);
    ~CookWidget(){}

    //界面拖动、界面绘制、接收消息
    void mousePressEvent(QMouseEvent *event);//界面拖动
    bool nativeEvent(const QByteArray &eventType, void *message, long *result);
    void paintEvent(QPaintEvent *event);//界面绘制

private:
    Cook *m_cook;

    QTableWidget *m_dishesTable;            //显示菜品的表格
    QVector<QPushButton*> m_cookButtons;    //用于认领任务和完成任务的按钮
    QHash<QString,QTime> m_time;            //记录完成任务的时间
    Dishes m_unfinishedDishes;              //未完成的菜品
    void extract();                         //提取未完成菜品

    WMSender wmsender;                      //Windows消息发送器

private slots:
    void update();                          //更新任务列表
    void onClicked();                       //认领或完成任务
};

#endif  //COOKWIDGET_H
