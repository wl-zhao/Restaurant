#include <QApplication>
#include "beautify.h"
#include "SignWidget.h"

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);

    setStyle(":/white.qss");//加载样式表
    SignWidget sign;
    sign.show();

    return a.exec();
}
