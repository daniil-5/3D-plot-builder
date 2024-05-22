#include "mainwidget.h"

#include "mainwidget.h"

#include <QMouseEvent>
#include <QWheelEvent>

const float MIN_SCALE = 15.0;
const float MAX_SCALE = 0.1;

const float SPEED_DECREASE = 0.998;
const float MIN_SPEED = 0.01;

const int TIMER_SPEED = 12;

const float SCALE_FACTOR = 1.1;

mainwidget::~mainwidget()
{
    makeCurrent();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    doneCurrent();
}

void mainwidget::mousePressEvent(QMouseEvent *e)
{
    // Save mouse press position
    mousePressPosition = QVector2D(e->position());
    angularSpeed = 0;
}

void mainwidget::mouseReleaseEvent(QMouseEvent *e)
{
    // Mouse release position - mouse press position
    QVector2D diff = QVector2D(e->position()) - mousePressPosition;

    // Rotation axis is perpendicular to the mouse position difference
    // vector
    QVector3D n = QVector3D(diff.y(), diff.x(), 0.0).normalized();

    // Accelerate angular speed relative to the length of the mouse sweep
    qreal acc = diff.length() / 100.0;

    // Calculate new rotation axis as weighted sum
    rotationAxis = (rotationAxis * angularSpeed + n * acc).normalized();
    // Increase angular speed
    angularSpeed += acc;

}

void mainwidget::wheelEvent(QWheelEvent *e)
{
    if (e->angleDelta().y() < 0)
    {
        // Mouse wheel scrolled up, increase size or zoom in
        if (scale <= MIN_SCALE)
        {
            scale *= SCALE_FACTOR;
        }
        qDebug()<<"Wheel is scrolling up";
    }
    else
    {
        if (scale >= MAX_SCALE)
        {
            scale /= SCALE_FACTOR;
        }
        qDebug()<<"Wheel is scrolling down";
        // Mouse wheel scrolled down, decrease size or zoom out
    }
    update();
}

void mainwidget::timerEvent(QTimerEvent *timer)
{
    Q_UNUSED(timer);
    // Decrease angular speed (friction)
    angularSpeed *= SPEED_DECREASE;

    // Stop rotation when speed goes below threshold
    if (angularSpeed < MIN_SPEED)
    {
        angularSpeed = 0.0;
    }
    else
    {
        // Update rotation
        rotation = QQuaternion::fromAxisAndAngle(rotationAxis, angularSpeed) * rotation;

        // Request an update
        update();
    }
}

void mainwidget::initializeGL()
{
    initializeOpenGLFunctions();

    glClearColor(1, 1, 1, 1);

    initShaders();
    // Use QBasicTimer because its faster than QTimer
    timer.start(TIMER_SPEED, this);
}

void mainwidget::initShaders()
{
    // Compile vertex shader
    if (!program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/Shaders/vshader.glsl"))
    {
        close();
    }

    // Compile fragment shader
    if (!program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/Shaders/fshader.glsl"))
    {
        close();
    }

    // Link shader pipeline
    if (!program.link())
    {
        close();
    }

    // Bind shader pipeline for use
    if (!program.bind())
    {
        close();
    }
}

void mainwidget::resizeGL(int w, int h)
{
    // Calculate aspect ratio
    qreal aspect = static_cast<qreal>(w) / qreal(h ? h : 1);

    // Set near plane to 3.0, far plane to 7.0, field of view 45 degrees
    const qreal zNear = 0.10, zFar = 20.0, fov = 45.0;

    // Reset projection
    projection.setToIdentity();

    // Set perspective projection
    projection.perspective(fov, aspect, zNear, zFar);
}

void mainwidget::paintGL()
{
    // Clear color and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Enable depth buffer
    glEnable(GL_DEPTH_TEST);

    //texture->bind();
    program.bind();

    // Calculate model view transformation
    QMatrix4x4 matrix;
    matrix.translate(0.0, 0.0, -scale);
    matrix.rotate(rotation);

    // Set modelview-projection matrix
    program.setUniformValue("mvp_matrix", projection * matrix);
    program.bind();


    drawAxes();
    if(isSections)
    {
        surface.drawSection();
    }

    if(isCube)
    {
        drawCube();
    }

    if (isXGrid)
    {
        drawGridX();
    }

    if (isYGrid)
    {
        drawGridY();
    }
    surface.draw();
}

void mainwidget::drawCube()
{
    // Define the vertices and colors of the cube
    GLfloat vertices[] =
        {
            // positions          // colors
            -0.5f, -0.5f, -0.5f,  0.10f, 0.10f, 0.10f,  // bottom-left
            0.5f, -0.5f, -0.5f,  0.10f, 0.10f, 0.10f,  // bottom-right
            0.5f,  0.5f, -0.5f,  0.10f, 0.10f, 0.10f,  // top-right
            -0.5f,  0.5f, -0.5f,  0.10f, 0.10f, 0.10f,  // top-left
            -0.5f, -0.5f,  0.5f,  0.10f, 0.10f, 0.10f,  // bottom-left
            0.5f, -0.5f,  0.5f,  0.10f, 0.10f, 0.10f,  // bottom-right
            0.5f,  0.5f,  0.5f,  0.10f, 0.10f, 0.10f,  // top-right
            -0.5f,  0.5f,  0.5f,  0.10f, 0.10f, 0.10f   // top-left
        };

    GLuint indices[] ={
        0, 1, 1, 2, 2, 3, // front
        3, 0, 4, 0, 4, 5, // back
        1, 5, 5, 6, 6, 2, // bottom
        6, 7, 7, 4, 7, 3, // top
    };

    // Create a vertex buffer object and copy the vertex data to it
    GLuint VBO, VAO, EBO;
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenVertexArrays(1, &VAO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Draw the cube
    glDrawElements(GL_LINES, 36, GL_UNSIGNED_INT, nullptr);

    // Unbind the VAO
    glBindVertexArray(0);
}

void mainwidget::drawAxes()
{
    // Define the vertices for the axes
    GLfloat vertices[] = {
        // positions          // colors
        -1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  // X axis
        1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,

        0.0f, -1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  // Y axis
        0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,

        0.0f, 0.0f, -1.0f,  0.0f, 0.0f, 1.0f,  // Z axis
        0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,
        // Y
        0.05f, 0.8f, 0.0f,   0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,   0.0f, 1.0f, 0.0f,
        -0.05f, 0.8f, 0.0f,  0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

        0.0f, 0.8f, 0.05f, 0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.8f, -0.05f, 0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        // X
        0.8f, 0.0f, 0.05f, 1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.8f, 0.0f, -0.05f, 1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,

        0.8f, 0.05f, 0.00f, 1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.8f, -0.05f, 0.00f, 1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,

        // Z
        0.05f, 0.0f, 0.8f,   0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,   0.0f, 0.0f, 1.0f,
        -0.05f, 0.0f, 0.8f,  0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,

        0.0f, 0.05f, 0.8f,   0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,   0.0f, 0.0f, 1.0f,
        0.0f, -0.05f, 0.8f,  0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
    };

    // Create a vertex buffer object and copy the vertex data to it
    GLuint VBO, VAO;
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Draw the axes
    glDrawArrays(GL_LINES, 0, 30);
}

void mainwidget::drawGridX()
{
    // Define the range and step size for x and z
    float yStart = -5.0f, yEnd = 5.0f, yStep = 0.1f;
    float zStart = -5.0f, zEnd = 5.0f, zStep = 0.1f;

    // Generate a grid of lines
    std::vector<GLfloat> vertices;
    for (float y = yStart; y <= yEnd; y += yStep) {
        // Line parallel to z-axis
        vertices.push_back(0);
        vertices.push_back(y);
        vertices.push_back(zStart);
        vertices.push_back(0.1f);// R
        vertices.push_back(0.1f);// G
        vertices.push_back(0.1f);// B

        vertices.push_back(0);
        vertices.push_back(y);
        vertices.push_back(zEnd);
        vertices.push_back(0.1f);// R
        vertices.push_back(0.1f);// G
        vertices.push_back(0.1f);// B
    }

    for (float z = zStart; z <= zEnd; z += zStep) {
        // Line parallel to x-axis
        vertices.push_back(0);
        vertices.push_back(yStart);
        vertices.push_back(z);
        vertices.push_back(0.1f);// R
        vertices.push_back(0.1f);// G
        vertices.push_back(0.1f);// B

        vertices.push_back(0);
        vertices.push_back(yEnd);
        vertices.push_back(z);
        vertices.push_back(0.1f);// R
        vertices.push_back(0.1f);// G
        vertices.push_back(0.1f);// B
    }

    // Create a vertex buffer object and copy the vertex data to it
    GLuint VBO, VAO;
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);

    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);


    // Draw the grid as lines
    glDrawArrays(GL_LINES, 0, vertices.size() / 6);

    // Unbind the VAO
    glBindVertexArray(0);
}

void mainwidget::drawGridY()
{
    // Define the range and step size for x and z
    float xStart = -5.0f, xEnd = 5.0f, xStep = 0.1f;
    float zStart = -5.0f, zEnd = 5.0f, zStep = 0.1f;

    // Generate a grid of lines
    std::vector<GLfloat> vertices;
    for (float x = xStart; x <= xEnd; x += xStep)
    {
        // Line parallel to z-axis
        vertices.push_back(x);
        vertices.push_back(0);
        vertices.push_back(zStart);
        vertices.push_back(0.1f); // R
        vertices.push_back(0.1f); // G
        vertices.push_back(0.1f); // B

        vertices.push_back(x);
        vertices.push_back(0);
        vertices.push_back(zEnd);
        vertices.push_back(0.1f); // R
        vertices.push_back(0.1f); // G
        vertices.push_back(0.1f); // B
    }

    for (float z = zStart; z <= zEnd; z += zStep)
    {
        // Line parallel to x-axis
        vertices.push_back(xStart);
        vertices.push_back(0);
        vertices.push_back(z);
        vertices.push_back(0.1f); // R
        vertices.push_back(0.1f); // G
        vertices.push_back(0.1f); // B

        vertices.push_back(xEnd);
        vertices.push_back(0);
        vertices.push_back(z);
        vertices.push_back(0.1f); // R
        vertices.push_back(0.1f); // G
        vertices.push_back(0.1f); // B
    }

    // Create a vertex buffer object and copy the vertex data to it
    GLuint VBO, VAO;
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Draw the grid as lines
    glDrawArrays(GL_LINES, 0, vertices.size() / 6);

    // Unbind the VAO
    glBindVertexArray(0);
}

void mainwidget::setXGrid()
{
    isXGrid = !isXGrid;
    update();
}

void mainwidget::setYGrid()
{
    isYGrid = !isYGrid;
    update();
}

void mainwidget::setCube()
{
    isCube = !isCube;
    update();
}

void mainwidget::setSections(std::vector<int> s)
{
    _sections = s;
    surface.drawSection(_sections);
}

void mainwidget::setSections()
{
    isSections = !isSections;
}

bool mainwidget::getSection()
{
    return isSections;
}

Surface& mainwidget::getSurface()
{
    return surface;
    update();
}
