#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QColorDialog>
#include <QMainWindow>
#include <QMessageBox>

#include "mainwidget.h"
#include "stattable.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void LoadFromFile();

private slots:

    void on_drawGridY_clicked();

    void on_drawGridX_clicked();

    void on_drawCube_clicked();

    void on_functionLineEdit_returnPressed();

    void on_SingleStep_valueChanged(double arg1);

    void on_drawTypeComboBox_currentIndexChanged(int index);

    void on_CurrentColor_clicked();

    void on_Bounds_valueChanged(double arg1);

    void on_Intersections_clicked();

    void on_deleteSelected_clicked();

    void on_SaveToFile_clicked();

    void on_pushButton_clicked();

private:
    Ui::MainWindow* ui;
    mainwidget* _openGLWidget;
    QColor _color;
    StatTable* _table;
};

#endif // MAINWINDOW_H
