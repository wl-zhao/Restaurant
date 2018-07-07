#include "ExportExcel.h"
#include <qt_windows.h>
#include <QDir>

//初始化
ExportExcel::ExportExcel(QString filepath,QString sheetName) : m_filepath(filepath)
{
    CoInitialize(0);
    m_excel = new QAxObject("Excel.Application");
    m_excel->setProperty("Visible", false);
    QAxObject *work_books = m_excel->querySubObject("WorkBooks");
    work_books->dynamicCall("Add");
    m_workbook = m_excel->querySubObject("ActiveWorkBook");
    m_worksheets = m_workbook->querySubObject("Sheets");
    m_worksheet=m_worksheets->querySubObject("Item(int)",1);
    m_worksheet->setProperty("Name",sheetName);
}

//插入一个工作表
QAxObject* ExportExcel::insertSheet(QString sheetName)
{
    int sheet_count = m_worksheets->property("Count").toInt();  //获取工作表数目
    QAxObject *last_sheet = m_worksheets->querySubObject("Item(int)", sheet_count);
    m_worksheet = m_worksheets->querySubObject("Add(QVariant)", last_sheet->asVariant());
    last_sheet->dynamicCall("Move(QVariant)", m_worksheet->asVariant());

    m_worksheet->setProperty("Name", sheetName);
    return m_worksheet;
}

//插入一个单元格
QAxObject* ExportExcel::insertCell(int row, int colomn, QString content)
{
    QAxObject *cell = m_worksheet->querySubObject("Cells(int,int)", row, colomn);
    cell->setProperty("Value", content);  //设置单元格值
    cell->setProperty("HorizontalAlignment", -4108);
    cell->setProperty("VerticalAlignment", -4108);
    return cell;
}

//设置当前单元格
void ExportExcel::setCurrentSheet(QAxObject *worksheet)
{
    m_worksheet = worksheet;
}

//合并单元格
void ExportExcel::cellMerge(int row_begin, int row_end, int column_begin, int column_end)
{
    QString merge_cell;
    merge_cell.append(QChar(column_begin - 1 + 'A'));   //初始列
    merge_cell.append(QString::number(row_begin));      //初始行
    merge_cell.append(":");
    merge_cell.append(QChar(column_end - 1 + 'A'));     //终止列
    merge_cell.append(QString::number(row_end));        //终止行
    QAxObject *merge_range =m_worksheet->querySubObject("Range(const QString&)", merge_cell);
    merge_range->setProperty("HorizontalAlignment", -4108);
    merge_range->setProperty("VerticalAlignment", -4108);
    merge_range->setProperty("MergeCells", true);       //合并单元格
}

//按路径保存
void ExportExcel::save()
{
    m_workbook->dynamicCall("SaveAs(const QString&)",QDir::toNativeSeparators(m_filepath));
}

//退出Excel
void ExportExcel::quit()
{
    m_workbook->dynamicCall("Close(Boolean)", false);
    m_excel->dynamicCall("Quit(void)");
}
