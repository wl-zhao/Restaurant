#ifndef ExportExcel_H
#define ExportExcel_H
#include <QString>
#include <QAxObject>

//导出Excel
class ExportExcel
{
public:
    ExportExcel(QString filepath,QString sheetName);
    QAxObject* insertSheet(QString sheetName);
    QAxObject* insertCell(int row, int colomn, QString content);
    void  setCurrentSheet(QAxObject *worksheet);
    void cellMerge(int row_begin, int row_end, int column_begin,  int column_end);
    void save();
    void quit();

private:
    QString m_filepath;         //文件路径
    QAxObject *m_worksheet;     //工作表
    QAxObject *m_worksheets;    //所有工作表
    QAxObject *m_workbook;      //工作簿
    QAxObject *m_excel;         //Excel文件
};

#endif // ExportExcel_H
