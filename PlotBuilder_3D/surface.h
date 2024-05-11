#ifndef SURFACE_H
#define SURFACE_H

#include <QFile>
#include <QObject>
#include <QWidget>

#include "OpenGL/OpenGL.h"
#include "QtOpenGL/qopenglshaderprogram.h"
#include "parser.h"

struct Vertex
{
    std::vector<GLfloat> vertices;
    std::vector<GLuint> indices;
};

struct Color
{
    GLfloat red;
    GLfloat green;
    GLfloat blue;
};

const Color NO_COLOR = {0.0f, 0.0f, 0.0f};

class Surface
{

public:

    /// Constructors
    Surface();
    ~Surface();

    /// Methods
    void draw(float(*func)(float x, float y,float z));
    void draw();
    void drawSection();
    void drawSection(std::vector<int> sections1 , std::vector<int> sections2);
    void drawSection(std::vector<int> sections);
    void addSurface(std::string expr);
    void hide(int index);

    /// Getters
    Color getColor();
    float getStep();

    /// Setters
    void setColor(Color color);
    void setColor(QColor color);
    void setStep(float step);
    void setBounds(float bound);
    void setShader(std::shared_ptr<QOpenGLShaderProgram> shader);
    void setLayout(int index);
    void removeSurface(int index);
    QVector<QString>& getExpressions();
    void saveToFile();
    void loadFromFile();
    void clearFile();

private:

    std::shared_ptr<QOpenGLShaderProgram> program;
    Color _color = NO_COLOR;
    float _step = 0.05f;
    float _bound = 1.0;
    QVector<Vertex> _vertex;
    QVector<Color> _colors;
    QVector<QString> _expressions;
    int _currentIndex = 0;
    std::vector<bool> _hidden;

    std::vector<GLfloat> _sections;
    const QString FILE_NAME = "surfaces.txt";
};

#endif // SURFACE_H
