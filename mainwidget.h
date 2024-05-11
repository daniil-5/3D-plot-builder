#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include "surface.h"

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QMatrix4x4>
#include <QQuaternion>
#include <QVector2D>
#include <QBasicTimer>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>


class GeometryEngine;

class mainwidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    using QOpenGLWidget::QOpenGLWidget;
    ~mainwidget();

    void drawCube();
    void drawAxes();
    void drawGridX();
    void drawGridY();

    void setXGrid();
    void setYGrid();
    void setCube();
    void setSections(std::vector<int> s);
    void setSections();
    bool getSection();
    Surface& getSurface();

protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;
    void timerEvent(QTimerEvent *e) override;

    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void initShaders();

private:
    bool isXGrid = false;
    bool isYGrid = false;
    bool isCube = false;
    bool isSections = false;
    QBasicTimer timer;
    QOpenGLShaderProgram program;
    std::vector<int> _sections;
    QMatrix4x4 projection;

    QVector2D mousePressPosition;
    QVector3D rotationAxis;
    qreal angularSpeed = 0;
    qreal scale = 4;
    QQuaternion rotation;
    Surface surface;
};

#endif // MAINWIDGET_H
