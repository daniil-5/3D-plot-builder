#ifndef STATTABLE_H
#define STATTABLE_H

#include <QCheckBox>
#include <QObject>
#include <QTableWidget>
#include <QWidget>

#include "mainwidget.h"

class StatTable : public QTableWidget
{
    Q_OBJECT
public:
    StatTable();
    void AddRow(QString expr);
public slots:
    void cellModified(int row, int column);
    void checkboxStateChanged(int state);
    void setWidget(mainwidget* widget);
    void RemoveRow(int index);
    std::vector<int> getActiveSections();
private:
    QVector<QCheckBox*> m_boxes;
    QVector<QString> m_expressions;
    mainwidget* _widget;
};

#endif // STATTABLE_H
