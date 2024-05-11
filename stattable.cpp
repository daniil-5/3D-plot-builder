#include "stattable.h"
#include "QtGui/qpen.h"

StatTable::StatTable():QTableWidget()
{
    this->setColumnCount(2);
    QStringList labels;
    labels << "Select" << "Equation";
    this->setHorizontalHeaderLabels(labels);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    this->setGridStyle(Qt::SolidLine);
    //this->setStyleSheet("gridline-color: #FFFFFF");
    this->setColumnWidth(0,50);
    this->setColumnWidth(1,400);
    this->setAlternatingRowColors(true);

    connect(this, &QTableWidget::cellChanged, this, &StatTable::cellModified);
}
void StatTable::AddRow(QString expr)
{
    // Disconnect the cellChanged signal
    disconnect(this, &QTableWidget::cellChanged, this, &StatTable::cellModified);

    QCheckBox* tempbox = new QCheckBox();
    tempbox->setChecked(true);
    int row  = this->rowCount();
    this->insertRow(row);
    this->setCellWidget(row,0,tempbox);
    this->setItem(row,1,new QTableWidgetItem(expr));

    m_boxes.push_back(tempbox);
    m_expressions.push_back(expr);

    // Connect the stateChanged signal of the checkbox to the checkboxStateChanged slot
    connect(tempbox, &QCheckBox::stateChanged, this, &StatTable::checkboxStateChanged);

    // Reconnect the cellChanged signal
    connect(this, &QTableWidget::cellChanged, this, &StatTable::cellModified);
}

void StatTable::cellModified(int row, int column)
{
    QWidget *widget = this->cellWidget(row, 0);

    // Try to cast the widget to a QCheckBox
    auto *checkbox = qobject_cast<QCheckBox *>(widget);

    // If the cast is successful
    if (checkbox)
    {
        // Check if the checkbox is checked
        if (checkbox->isChecked())
        {
            qDebug() << "Checkbox at row" << row << "and column 0 is checked.";
        } else
        {
            qDebug() << "Checkbox at row" << row << "and column 0 is unchecked.";
        }
    } else
    {
        qDebug() << "No checkbox at row" << row << "and column 0.";
    }
}

void StatTable::checkboxStateChanged(int state)
{
    // Get the checkbox that emitted the signal
    QCheckBox *checkbox = qobject_cast<QCheckBox *>(sender());

    // If the cast is successful
    if (checkbox)
    {
        // Find the row of the checkbox
        int row = this->indexAt(checkbox->pos()).row();
        _widget->getSurface().hide(row);
        _widget->update();
        // Check the new state of the checkbox
        if (state == Qt::Checked)
        {
            qDebug() << "Checkbox at row" << row << "is checked.";
        }
        else
        {
            qDebug() << "Checkbox at row" << row << "is unchecked.";
        }
    }
}

void StatTable::setWidget(mainwidget *widget)
{
    _widget = widget;
}

void StatTable::RemoveRow(int index)
{
    this->removeRow(index);
    m_boxes.removeAt(index);
    m_expressions.removeAt(index);
    for (int i = 0; i < m_boxes.size(); ++i)
    {
        if(!m_boxes.at(i)->checkState())
            m_boxes.at(i)->setChecked(true);
    }
}

std::vector<int> StatTable::getActiveSections()
{
    std::vector<int> s;
    for (int i = 0; i < m_boxes.size(); ++i)
    {
        if(m_boxes.at(i)->checkState())
        {
            s.push_back(i);
        }
    }
    return s;
}
