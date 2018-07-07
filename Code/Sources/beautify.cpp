#include "beautify.h"

#include <QApplication>
#include <QEvent>
#include <QMouseEvent>
#include <QStyle>
#include <QPainter>
#include <QtMath>


TitleBar::TitleBar(QWidget *parent)
    : QWidget(parent)
{
    setFixedHeight(30);
    this->setAttribute(Qt::WA_TranslucentBackground);//设置窗口背景透明

    //添加标题栏控件
    m_pIconLabel = new QLabel(this);
    m_pTitleLabel = new QLabel(this);
    m_pMinimizeButton = new QPushButton(this);
    m_pCloseButton = new QPushButton(this);

    //设置控件样式
    m_pIconLabel->setFixedSize(20, 20);
    m_pIconLabel->setScaledContents(true);

    m_pTitleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_pMinimizeButton->setFixedSize(27, 22);
    m_pCloseButton->setFixedSize(27, 22);

    m_pTitleLabel->setObjectName("whiteLabel");
    m_pMinimizeButton->setObjectName("minimizeButton");
    m_pCloseButton->setObjectName("closeButton");

    m_pMinimizeButton->setToolTip("最小化");
    m_pCloseButton->setToolTip("关闭");
    m_pMinimizeButton->setFlat(true);
    QString style="QPushButton{background: rgb(0,155,219);}"
                  "QPushButton:enabled:hover{background: rgb(100, 160, 220);}"
                  "QPushButton:enabled:pressed{background: rgb(0, 78, 161);}";
    m_pMinimizeButton->setStyleSheet(style);
    m_pCloseButton->setStyleSheet(style);

    //设置布局
    QHBoxLayout *pLayout = new QHBoxLayout(this);
    pLayout->addWidget(m_pIconLabel);
    pLayout->addSpacing(5);
    pLayout->addWidget(m_pTitleLabel);
    pLayout->addWidget(m_pMinimizeButton);
    pLayout->addSpacing(5);
    pLayout->addWidget(m_pCloseButton);
    pLayout->setSpacing(0);
    pLayout->setContentsMargins(5, 0, 5, 0);

    setLayout(pLayout);

    connect(m_pMinimizeButton, SIGNAL(clicked(bool)), this, SLOT(onClicked()));
    connect(m_pCloseButton, SIGNAL(clicked(bool)), this, SLOT(onClicked()));
}

TitleBar::~TitleBar()
{

}

void TitleBar::mousePressEvent(QMouseEvent *event)
{
    dragEffect(this,event);
}

bool TitleBar::eventFilter(QObject *obj, QEvent *event)//事件过滤器, 用于更改窗体图标和标题
{
    switch (event->type())
    {
    case QEvent::WindowTitleChange:
    {
        QWidget *pWidget = qobject_cast<QWidget *>(obj);
        if (pWidget)
        {
            m_pTitleLabel->setText(pWidget->windowTitle());
            return true;
        }
    }
    case QEvent::WindowIconChange:
    {
        QWidget *pWidget = qobject_cast<QWidget *>(obj);
        if (pWidget)
        {
            QIcon icon = pWidget->windowIcon();
            m_pIconLabel->setPixmap(icon.pixmap(m_pIconLabel->size()));
            return true;
        }
    }
        return true;
    }
    return QWidget::eventFilter(obj, event);
}

void TitleBar::paintEvent(QPaintEvent *event)
{
    paintEffect(this,event,QColor(0,155,219));//绘制圆角、阴影、背景
}

//窗口最小化和关闭
void TitleBar::onClicked()
{
    QPushButton *pButton = qobject_cast<QPushButton *>(sender());
    QWidget *pWindow = this->window();
    if (pWindow->isTopLevel())
    {
        if (pButton == m_pMinimizeButton)
        {
            pWindow->showMinimized();
        }
        else if (pButton == m_pCloseButton)
        {
            pWindow->close();
        }
    }
}

//设置界面w可拖动
void dragEffect(QWidget *w, QMouseEvent *event)
{
#ifdef Q_OS_WIN
    if (ReleaseCapture())
    {
        QWidget *pWindow = w->window();
        if (pWindow->isTopLevel())
        {
            SendMessage(HWND(pWindow->winId()), WM_SYSCOMMAND, SC_MOVE + HTCAPTION, 0);
        }
    }
    event->ignore();
#else
#endif
}

//绘制圆角、阴影、背景
void paintEffect(QWidget *w, QPaintEvent *event, QColor color)
{
    Q_UNUSED(event)
    QPainter painter(w);
    painter.setRenderHint(QPainter::Antialiasing,true);
    painter.setBrush(QBrush(color));
    painter.setPen(Qt::transparent);

    if (color != QColor(0, 155, 219))//如果不是绘制标题栏
    {
        painter.drawRoundedRect(5, 5, w->width()-10, w->height()-10, 5, 5);//绘制背景
        QColor Color(0, 0, 0);
        painter.setPen(Color);
        for (int i=0; i<5; ++i)//绘制阴影
        {
            painter.setBrush(QBrush(Qt::transparent));
            QPainterPath path;
            path.setFillRule(Qt::WindingFill);
            path.addRoundedRect(5-i, 5-i, w->width()-(5-i)*2, w->height()-(5-i)*2, 5, 5);
            Color.setAlpha((100-qSqrt(i)*50)*2);
            painter.setPen(Color);
            painter.drawPath(path);
        }
    }
    else//绘制标题栏, 实现上边有圆角而下边没有
    {
        painter.drawRoundedRect(0,0,w->width()-1,w->height(),5,5);//绘制圆角矩形
        painter.drawRect(0,15,w->width()-1,15);//向下偏移绘制矩形
    }
}

//设置QSS
void setStyle(const QString &style)
{
    QFile qss(style);
    qss.open(QFile::ReadOnly);
    qApp->setStyleSheet(qss.readAll());
    qss.close();
}
