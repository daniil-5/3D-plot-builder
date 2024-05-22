#include "mainwindow.h"
#include "QtGui/qsurfaceformat.h"
#include "surface.h"
#include "ui_mainwindow.h"

#define DEPTH_SIZE 24

bool CheckFunction(QString& expr)
{
    int openParentheses = expr.count(QChar('('));
    int closeParentheses = expr.count(QChar(')'));
    if(openParentheses != closeParentheses or expr.isEmpty())
    {
        QMessageBox::warning(nullptr, "Invalid Input", "The number of ( and ) is not equal");
        return false;
    }
    QRegularExpression re("^([-+/*^ - ^ + * / ]|\\b(sin|cos|tan|log)\\b\\(.*\\)|\\b(x|y)\\b|\\d+(\\.\\d+)?|\\(.*\\))*$");
    QRegularExpressionMatch match = re.match(expr);
    if(!match.hasMatch())
    {
       QMessageBox::warning(nullptr, "Invalid Input", "The mathematic equatation inlude : \n sin , cos , tan , log , - , + , * , / , ^ .\nAnd use ( and ) to set the parameters correct(DON'T miss the operators such as 5x use 5*x instead)");
       return false;
    }
    return true;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QSurfaceFormat format;
    format.setDepthBufferSize(DEPTH_SIZE);
    format.setAlphaBufferSize(8);
    QSurfaceFormat::setDefaultFormat(format);
    ui->CurrentColor->setStyleSheet("QPushButton { background-color: qlineargradient(x1: 1, y1: 1, x2: 0, y2: 1,  stop: 0 #f6d365, stop: 0.5 #7cc576, stop: 1 #3498db); }");
    _openGLWidget = new mainwidget(nullptr);
    _table = new StatTable();
    _table->setWidget(_openGLWidget);
    LoadFromFile();
    _openGLWidget->getSurface().setColor(QColor(0.0f,0.0F,0.0F));
    ui->Table->addWidget(_table);
    ui->gridLayout->addWidget(_openGLWidget);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::LoadFromFile()
{
    QVector<QString> expr = _openGLWidget->getSurface().getExpressions();
    for (int i = 0; i < expr.size() && expr.at(0) != ""; ++i)
    {
        _table->AddRow(expr.at(i));
    }
}

void MainWindow::on_drawGridY_clicked()
{
    _openGLWidget->setYGrid();
}

void MainWindow::on_drawGridX_clicked()
{
    _openGLWidget->setXGrid();
}

void MainWindow::on_drawCube_clicked()
{
    _openGLWidget->setCube();
}

void MainWindow::on_functionLineEdit_returnPressed()
{
    if(_table->rowCount() <= 7)
    {
        std::string func;
        QString str = ui->functionLineEdit->text();
        ui->functionLineEdit->clear();
        func = str.toStdString();
        if (CheckFunction(str))
        {
            try
            {
                _openGLWidget->getSurface().addSurface(func);
                _openGLWidget->update();
                _table->AddRow(QString("z = " + str));
            }
            catch (...)
            {
                QMessageBox::warning(nullptr, "Invalid Input", "The mathematic equatation inlude : \n sin , cos , tan , log , - , + , * , / , ^ .");
                ui->functionLineEdit->clear();
            }

        }
    }
    else
    {
        QMessageBox::warning(nullptr, "Invalid Input", "The number of mathematal equatations is too big");
        ui->functionLineEdit->clear();
    }
}

void MainWindow::on_SingleStep_valueChanged(double arg1)
{
    _openGLWidget->getSurface().setStep(ui->SingleStep_2->value());
    _openGLWidget->update();
}

void MainWindow::on_drawTypeComboBox_currentIndexChanged(int index)
{
    _openGLWidget->getSurface().setLayout(ui->drawTypeComboBox->currentIndex());
    _openGLWidget->update();
}

void MainWindow::on_CurrentColor_clicked()
{
    _color = QColorDialog::getColor();
    if(_color.isValid())
    {
        QString qss = QString("background-color: %1").arg(_color.name());
        ui->CurrentColor->setStyleSheet(qss);
    }
    _openGLWidget->getSurface().setColor(_color);
    _openGLWidget->update();
}


void MainWindow::on_Bounds_valueChanged(double arg1)
{
    _openGLWidget->getSurface().setBounds(ui->Bounds->value());
}

void MainWindow::on_Intersections_clicked()
{
         std::vector<int> sections = _table->getActiveSections();
         if(!sections.empty())
         {
        _openGLWidget->setSections();
        _openGLWidget->setSections(sections);
        _openGLWidget->update();
         }
        else
        {
             QMessageBox::warning(nullptr, "Invalid Index", "The number of selected expressions is empty!");
             _openGLWidget->setSections();
             _openGLWidget->update();
             return;
        }
}

void MainWindow::on_deleteSelected_clicked()
{
    std::vector<int> selected = _table->getActiveSections();
    for (int i = selected.size() - 1; i >= 0; --i)
    {
        _openGLWidget->getSurface().removeSurface(selected.at(i));
        _table->RemoveRow(selected.at(i));
        _openGLWidget->update();
    }
}

void MainWindow::on_SaveToFile_clicked()
{
    _openGLWidget->getSurface().saveToFile();
}

void MainWindow::on_pushButton_clicked()
{
    close();
}

