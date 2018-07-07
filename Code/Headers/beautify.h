#ifndef TITLE_BAR
#define TITLE_BAR
#ifdef Q_OS_WIN
#pragma comment(lib, "user32.lib")
#include <qt_windows.h>
#endif

#include "stable.h"

class TitleBar : public QWidget//自定义标题栏
{
    Q_OBJECT
public:
    explicit TitleBar(QWidget *parent = 0);
    ~TitleBar();
signals:
    void closeWidget();
protected:
    // 设置鼠标拖动界面
    virtual void mousePressEvent(QMouseEvent *event);
    // 设置界面标题与图标
    virtual bool eventFilter(QObject *obj, QEvent *event);
    //绘制界面
    void paintEvent(QPaintEvent *event);
private slots:
    // 进行最小化、关闭操作
    void onClicked();
private:
    QLabel *m_pIconLabel;
    QLabel *m_pTitleLabel;
    QPushButton *m_pMinimizeButton;
    QPushButton *m_pCloseButton;
};

//拖动效果
void dragEffect(QWidget *w, QMouseEvent*event);

//绘制圆角阴影以及界面背景
void paintEffect(QWidget* w,QPaintEvent*event,QColor color=QColor(235,242,249));

//设置样式表
void setStyle(const QString &style);
#endif // TITLE_BAR
