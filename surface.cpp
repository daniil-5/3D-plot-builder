#include "surface.h"

bool boundingBoxesIntersect(QVector<GLfloat>& v_1 ,QVector<GLfloat>& v_2)
{

}
bool IsinBound (float z)
{
    return (z <= 1.0f) && (z >= -1.0f);
}
static bool Equals(Color c_1, Color c_2)
{
    return c_1.red == c_2.red && c_1.green == c_2.green && c_1.blue == c_2.blue;
}

void Surface::setColor(Color color)
{
    _color = color;
}

void Surface::setColor(QColor color)
{
    Color c;
    c.red = color.redF();
    c.green = color.greenF();
    c.blue = color.blueF();
    setColor(c);
}

void Surface::setStep(float step)
{
    _step = step;
}

void Surface::setBounds(float bound)
{
    _bound = bound;
}

Color Surface::getColor()
{
    return _color;
}

float Surface::getStep()
{
    return _step;
}

void Surface::setShader(std::shared_ptr<QOpenGLShaderProgram> shader)
{
    program = shader;
}

void Surface::setLayout(int index)
{
    _currentIndex = index;
}

void Surface::removeSurface(int index)
{
    if(index < _vertex.size())
    {
        _vertex.removeAt(index);
        _expressions.removeAt(index);
        _colors.removeAt(index);
    }
}

QVector<QString> &Surface::getExpressions()
{
    return _expressions;
}

void Surface::draw(float(*func)(float x, float y, float z))
{
    // Define the range and step size for x and y
    float xStart = -1.0f, xEnd = 1.0f, xStep = getStep();
    float yStart = -1.0f, yEnd = 1.0f, yStep = getStep();
    float zStart = -1.0f, zEnd = 1.0f, zStep = getStep();
    float res = 0.001f;

    // Generate a grid of (x, y) coordinates
    std::vector<GLfloat> vertices;
    std::vector<GLuint> indices;
    int gridWidth = (xEnd - xStart) / xStep ;
    int gridHeight = (yEnd - yStart) / yStep ;

    for (float x = xStart; x < xEnd; x += xStep)
    {
        for (float y = yStart; y < yEnd; y += yStep)
        {
            for (float z = zStart; z < zEnd; z += zStep)
            {
                // Calculate the corresponding z value using the implicit function
                if (func(x, y, z) <= res && func(x, y, z) >= -res)
                {
                    vertices.push_back(x);
                    vertices.push_back(y);
                    vertices.push_back(z);

                    if(Equals(_color, NO_COLOR))
                    {
                        vertices.push_back(z - x); // R
                        vertices.push_back(y - z); // G
                        vertices.push_back(0.5f); // B
                    }
                    else
                    {
                        vertices.push_back(_color.red);
                        vertices.push_back(_color.green);
                        vertices.push_back(_color.blue);
                    }
                }
            }
        }
    }

    // Generate indices for the triangles
    for (int i = 0; i < gridHeight - 1; ++i)
    {
        for (int j = 0; j < gridWidth - 1; ++j)
        {
            int topLeft = i * gridWidth + j;
            int topRight = topLeft + 1;
            int bottomLeft = topLeft + gridWidth;
            int bottomRight = bottomLeft + 1;

            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);

            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }

}

void Surface::addSurface(std::string expr)
{
    _expressions.push_back(QString::fromStdString(expr));
    Parser parser;
    parser.setEquatation(expr);
    float xStart = -_bound, xEnd = _bound, xStep = getStep();
    float yStart = -_bound, yEnd = _bound, yStep = getStep();
    // Generate a grid of (x, y) coordinates
    std::vector<GLfloat> vertices;
    std::vector<GLuint> indices;
    int gridWidth = (xEnd - xStart) / xStep ;
    int gridHeight = (yEnd - yStart) / yStep;
    _colors.emplace_back(_color);
    for (int i = 0; i < gridHeight; ++i)
    {
        for (int j = 0; j < gridWidth; ++j)
        {
            float x = xStart + j * xStep;
            float y = yStart + i * yStep;

            float z = parser.getvalue(x,y);
            if (std::isinf(z) || std::isnan(z) || abs(z) > 100 )
            {
                // Handle the case where z is inf or nan
                // For example, you can set z to a default value
                z = 0.0f;
            }
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            if (Equals(_color, NO_COLOR))
            {
                vertices.push_back((x - xStart) / (xEnd - xStart));// R
                vertices.push_back((y - yStart) / (yEnd - yStart));// G
                vertices.push_back(0.5f);                          // B
            }
            else
            {
                vertices.push_back(_color.red);
                vertices.push_back(_color.green);
                vertices.push_back(_color.blue);
            }
            // Generate indices for two triangles (a square) if we're not on the edge of the grid
            if (i < gridHeight - 1 && j < gridWidth - 1)
            {
                int topLeft = i * gridWidth + j;
                int topRight = topLeft + 1;
                int bottomLeft = topLeft + gridWidth;
                int bottomRight = bottomLeft + 1;

                indices.push_back(topLeft);
                indices.push_back(bottomLeft);
                indices.push_back(topRight);

                indices.push_back(topRight);
                indices.push_back(bottomLeft);
                indices.push_back(bottomRight);
            }
        }
    }
    Vertex temp;
    temp.vertices = vertices;
    temp.indices = indices;
    _vertex.push_back(temp);
    _hidden.push_back(false);
    temp.indices.clear();
    temp.vertices.clear();
}
void Surface::draw()
{
    for (int i = 0; i < _vertex.size(); ++i)
    {
        if (_hidden.at(i))
            continue;
        // Create a vertex buffer object and copy the vertex data to it
        GLuint VBO, VAO, EBO;
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, _vertex.at(i).vertices.size() * sizeof(GLfloat), &_vertex.at(i).vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, _vertex.at(i).indices.size() * sizeof(GLuint), &_vertex.at(i).indices[0], GL_STATIC_DRAW);

        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Color attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        switch (_currentIndex)
        {
        case 0:
            glDrawElements(GL_LINES, _vertex.at(i).indices.size(), GL_UNSIGNED_INT, 0);
            break;
        case 1:
            glDrawElements(GL_TRIANGLES, _vertex.at(i).indices.size(), GL_UNSIGNED_INT, 0);
            break;
        case 2:
            glDrawElements(GL_POINTS, _vertex.at(i).indices.size(), GL_UNSIGNED_INT, 0);
            break;
        default:
            break;
        }
        // Unbind the VAO
        glBindVertexArray(0);
    }
}

void Surface::drawSection()
{
    const int stride = 6; // 3 for position (x, y, z), 3 for color (r, g, b)

    // Draw intersection points
    GLuint VBO, VAO;
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, _sections.size() * sizeof(GLfloat), &_sections[0], GL_DYNAMIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Draw the intersection points
    glPointSize(10.0f);
    glDrawArrays(GL_POINTS, 0, _sections.size() / stride);

    // Unbind the VAO
    glBindVertexArray(0);
}

void Surface::drawSection( std::vector<int> sections1, std::vector<int> sections2)
{
    QVector<GLfloat> intersectionVertices;
    // Define a color for intersection
    GLfloat darkColor[3] = {1.0f, 0.2f, 0.3f};

    // Find intersection points
    for (int section1 : sections1)
    {
        if (section1 < 0 || section1 >= _vertex.size())
        {
            qDebug() << "Invalid section index: " << section1 << "\n";
            continue;
        }

        for (int section2 : sections2)
        {
            if (section2 < 0 || section2 >= _vertex.size())
            {
                qDebug() << "Invalid section index: " << section2 << "\n";
                continue;
            }

            // Compare each vertex in section1 with each vertex in section2
            for (int i = 0; i < _vertex.at(section1).vertices.size(); i += 6)
            {
                for (int j = 0; j < _vertex.at(section2).vertices.size(); j += 6)
                {
                    if (std::abs(_vertex.at(section1).vertices[i] - _vertex.at(section2).vertices[j]) <= 0.02f &&
                        std::abs(_vertex.at(section1).vertices[i+1] - _vertex.at(section2).vertices[j+1]) <= 0.02f &&
                        std::abs(_vertex.at(section1).vertices[i+2] - _vertex.at(section2).vertices[j+2]) <= 0.02f)
                    {
                        // Intersection point found, add it to the intersectionVertices
                        intersectionVertices.push_back(_vertex.at(section1).vertices[i]);
                        intersectionVertices.push_back(_vertex.at(section1).vertices[i+1]);
                        intersectionVertices.push_back(_vertex.at(section1).vertices[i+2]);

                        // Add the dark color for the intersection point
                        intersectionVertices.push_back(darkColor[0]); // R
                        intersectionVertices.push_back(darkColor[1]); // G
                        intersectionVertices.push_back(darkColor[2]); // B
                    }
                }
            }
        }
    }

    // Draw intersection points
    GLuint VBO, VAO;
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, intersectionVertices.size() * sizeof(GLfloat), &intersectionVertices[0], GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Draw the intersection points
    glPointSize(15.0f);
    glDrawArrays(GL_POINTS, 0, intersectionVertices.size() / 6);

    // Unbind the VAO
    glBindVertexArray(0);
}

void Surface::drawSection(std::vector<int> sections)
{
    _sections.clear();
    const int stride = 6;
    const float tolerance = 0.1F;
    const GLfloat darkColor[3] = {1.0F, 0.2F, 0.3F};

    std::vector<GLfloat> intersectionVertices = _vertex.at(sections.at(0)).vertices;
    std::vector<GLfloat> result;

    // Find intersection points
    for (int k = 0; k <  intersectionVertices.size(); k += stride)
    {
        for (int i = 1; i < sections.size(); ++i)
        {
            std::vector<GLfloat> currentVertices = _vertex.at(sections.at(i)).vertices;
            for (int j = 0; j < currentVertices.size(); j += stride)
            {
                if (std::abs(currentVertices[j] - intersectionVertices[k]) <= tolerance &&
                    std::abs(currentVertices[j+1] - intersectionVertices[k+1]) <= tolerance &&
                    std::abs(currentVertices[j+2] - intersectionVertices[k+2]) <= tolerance)
                {
                    // Intersection point found, add it to the intersectionVertices
                    result.emplace_back(currentVertices[j]);
                    result.emplace_back(currentVertices[j+1]);
                    result.emplace_back(currentVertices[j+2]);

                    // Add the dark color for the intersection point
                    result.insert(result.end(), {darkColor[0], darkColor[1], darkColor[2]});
                }
            }
        }
    }
    _sections = result;
    // Draw intersection points
    GLuint VBO, VAO;
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, result.size() * sizeof(GLfloat), &result[0], GL_DYNAMIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Draw the intersection points
    glPointSize(10.0f);
    glDrawArrays(GL_POINTS, 0, result.size() / stride);

    // Unbind the VAO
    glBindVertexArray(0);
}

void Surface::hide(int index)
{
    if(index < 0 || index >= _hidden.size())
    {
        qDebug() << "Invalid index" << "\n";
        return;
    }
    _hidden.at(index) = !_hidden.at(index);
}

void Surface::saveToFile()
{
    clearFile();
    QFile file(FILE_NAME);
    file.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "Unable to open file: " << file.errorString();
        qDebug() << "Error: " << file.error(); // This will output the error number
        return;
    }

    QTextStream out(&file);
    for (int i = 0; i < _expressions.size(); ++i)
    {
        out << _colors.at(i).red << " " << _colors.at(i).green << " " << _colors.at(i).blue << "\n";
        out << _expressions.at(i) << "\n";
    }
    file.close();
}

void Surface::loadFromFile()
{
    QFile file(FILE_NAME);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Unable to open file";
        return;
    }
    QTextStream in(&file);
    Color t;
    while(!in.atEnd())
    {
        char c;
        in >> t.red >> t.green >> t.blue >> c;
        _color = t;
        QString expr = in.readLine();
        std::string s = expr.toStdString();
        addSurface(s);
    }
    file.close();
}

void Surface::clearFile()
{
    QFile file(FILE_NAME);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        qDebug() << "Unable to open file";
        return;
    }
    file.close();
}
Surface::Surface()
{
   // clearFile();
   loadFromFile();
}

Surface::~Surface() = default;

