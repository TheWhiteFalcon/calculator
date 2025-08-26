#include "PlotGlWidget.h"
#include <cmath>
#include <QMouseEvent>
#include <QPainter>
#include <QFontMetrics>
#include <QDebug>

PlotGlWidget::PlotGlWidget(QWidget *parent)
    : QOpenGLWidget(parent),
    vbo(QOpenGLBuffer::VertexBuffer),
    axesVbo(QOpenGLBuffer::VertexBuffer),
    zoomFactor(1.0f),
    xOffset(0.0f),
    yOffset(0.0f),
    lastMousePos(QPoint(0,0))
{
    viewMatrix.setToIdentity();
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
}

QPointF PlotGlWidget::screenToWorld(const QPoint& screenPos) const
{
    float ndcX = 2.0f * screenPos.x() / width() - 1.0f;
    float ndcY = 1.0f - 2.0f * screenPos.y() / height();
    float worldX = (ndcX / zoomFactor) - xOffset;
    float worldY = (ndcY / zoomFactor) - yOffset;
    return QPointF(worldX, worldY);
}

PlotGlWidget::~PlotGlWidget()
{
    vbo.destroy();
    axesVbo.destroy();
}


void PlotGlWidget::createGrid()
{
    std::vector<float> gridVertices;

    const int range = 100; // Диапазон от -100 до 100
    const int step = 5;    // Шаг сетки

    // Горизонтальные линии (параллельные оси X)
    for (int y = -range; y <= range; y += step) {
        if (y == 0) continue; // Пропускаем ось Y
        gridVertices.push_back(-range);
        gridVertices.push_back(static_cast<float>(y));
        gridVertices.push_back(range);
        gridVertices.push_back(static_cast<float>(y));
    }

    // Вертикальные линии (параллельные оси Y)
    for (int x = -range; x <= range; x += step) {
        if (x == 0) continue; // Пропускаем ось X
        gridVertices.push_back(static_cast<float>(x));
        gridVertices.push_back(-range);
        gridVertices.push_back(static_cast<float>(x));
        gridVertices.push_back(range);
    }

    // Создаем или пересоздаем VBO для сетки
    if (!gridVbo.isCreated()) {
        gridVbo.create();
    }

    gridVbo.bind();
    gridVbo.allocate(gridVertices.data(), gridVertices.size() * sizeof(float));
    gridVbo.release();

    // Сохраняем количество вершин для отрисовки
    gridVertexCount = gridVertices.size() / 2; // Каждая линия = 2 точки (4 координаты)
}


void PlotGlWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    // Шейдер для графика
    program.addShaderFromSourceCode(QOpenGLShader::Vertex,
                                    "#version 330 core\n"
                                    "layout(location = 0) in vec2 position;\n"
                                    "out vec2 pos;\n"
                                    "uniform mat4 transform;\n"
                                    "void main() {\n"
                                    "   gl_Position = transform * vec4(position, 0.0, 1.0);\n"
                                    "   pos = position;\n"
                                    "}");
    program.addShaderFromSourceCode(QOpenGLShader::Fragment,
                                    "#version 330 core\n"
                                    "out vec4 outColor;\n"
                                    "in vec2 pos;\n"
                                    "void main() {\n"
                                    "   outColor = vec4(pos.x, 0.8, 1.0, 1.0);\n"
                                    "}");
    program.link();

    // Шейдер для осей
    axesProgram.addShaderFromSourceCode(QOpenGLShader::Vertex,
                                        "#version 330 core\n"
                                        "layout(location = 0) in vec2 position;\n"
                                        "uniform mat4 transform;\n"
                                        "void main() {\n"
                                        "   gl_Position = transform * vec4(position, 0.0, 1.0);\n"
                                        "}");
    axesProgram.addShaderFromSourceCode(QOpenGLShader::Fragment,
                                        "#version 330 core\n"
                                        "out vec4 outColor;\n"
                                        "void main() {\n"
                                        "   outColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
                                        "}");
    axesProgram.link();

    viewMatrix.setToIdentity();
    viewMatrix.scale(0.1f, 0.1f);
    zoomFactor = 0.1f;

    generateFunction(1.0, 0.0, 0.0, 0.0, 1); // Инициализация с параметрами по умолчанию
    createAxes();
    createGrid();
}

void PlotGlWidget::generateFunction(double a, double b, double c, double d, int sin_prev_value)
{
    qDebug("Generating function");
    const int segments = 500;
    functionPoints.clear();
    functionPoints.reserve(segments * 2);

    for (int i = 0; i < segments; ++i) {
        float x = -100.0f + 200.0f * i / segments;
        float y;

        switch(sin_prev_value) {
        case 1: // sin(x)
            y = sin(x) * a;
            qDebug("sinus");
            break;
        case 2: // cos(x)
            y = cos(x) * a;
            qDebug("cosinus");
            break;
        case 3: // polynomial
            y = a * x * x * x + b * x * x + c * x + d;
            qDebug("poly");
            break;
        default:
            y = sin(x) * 1;
            qDebug("Default");
        }

        functionPoints.push_back(x);
        functionPoints.push_back(y);
    }

    pointCount = functionPoints.size() / 2;

    if (!vbo.isCreated()) {
        vbo.create();
    }

    if (vbo.isCreated()) {
        vbo.bind();
        vbo.allocate(functionPoints.data(), functionPoints.size() * sizeof(float));
        vbo.release();
    }
    update();
}


void PlotGlWidget::createAxes()
{
    std::vector<float> axes = {
        -100.0f, 0.0f, 100.0f, 0.0f,    // X axis
        0.0f, -100.0f, 0.0f, 100.0f,     // Y axis
        9.5f, 0.5f, 10.0f, 0.0f,       // X arrow part 1
        9.5f, -0.5f, 10.0f, 0.0f,      // X arrow part 2
        -0.5f, 9.5f, 0.0f, 10.0f,       // Y arrow part 1
        0.5f, 9.5f, 0.0f, 10.0f       // Y arrow part 2
    };

    axesVbo.create();
    axesVbo.bind();
    axesVbo.allocate(axes.data(), axes.size() * sizeof(float));
    axesVbo.release();
}

void PlotGlWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);


    if (gridVbo.isCreated() && gridVertexCount > 0) {
        program.bind(); // ← Используем программу графика вместо axesProgram
        program.setUniformValue("transform", viewMatrix);
        program.setUniformValue("color", QVector4D(0.1f, 0.1f, 0.9f, 1.0f));

        gridVbo.bind();
        program.enableAttributeArray(0);
        program.setAttributeBuffer(0, GL_FLOAT, 0, 2);

        glDrawArrays(GL_LINES, 0, gridVertexCount);

        gridVbo.release();
        program.release();
    }


    // Отрисовка осей
    axesProgram.bind();
    axesProgram.setUniformValue("transform", viewMatrix);
    axesVbo.bind();
    axesProgram.enableAttributeArray(0);
    axesProgram.setAttributeBuffer(0, GL_FLOAT, 0, 2);

    // Рисуем оси
    glDrawArrays(GL_LINES, 0, 4);
    // Рисуем стрелки
    glDrawArrays(GL_LINES, 4, 8);

    axesProgram.release();

    // Отрисовка графика
    program.bind();
    program.setUniformValue("transform", viewMatrix);
    vbo.bind();
    program.enableAttributeArray(0);
    program.setAttributeBuffer(0, GL_FLOAT, 0, 2);
    glLineWidth(2.0f);
    glDrawArrays(GL_LINE_STRIP, 0, pointCount);
    glLineWidth(1.0f);
    program.release();

    // Отрисовка меток
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.end();
}


float PlotGlWidget::calculateStepSize(float range) const
{
    float logRange = log10(range);
    float exponent = floor(logRange);
    float fraction = logRange - exponent;

    if (fraction < log10(2.0)) return pow(10.0, exponent);
    if (fraction < log10(5.0)) return 2.0 * pow(10.0, exponent);
    return 5.0 * pow(10.0, exponent);
}

void PlotGlWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w * devicePixelRatio(), h * devicePixelRatio());
}

void PlotGlWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        lastMousePos = event->pos();
    }
}

void PlotGlWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::LeftButton) {
        QPoint delta = event->pos() - lastMousePos;
        xOffset += 2.0f * delta.x() / (width() * zoomFactor);
        yOffset -= 2.0f * delta.y() / (height() * zoomFactor);

        // Обновляем матрицу вида
        viewMatrix.setToIdentity();
        viewMatrix.scale(zoomFactor, zoomFactor);
        viewMatrix.translate(xOffset, yOffset);

        lastMousePos = event->pos();
        update();
    }
}

void PlotGlWidget::wheelEvent(QWheelEvent* event)
{
    QPointF mouseWorldBefore = screenToWorld(event->position().toPoint());

    float zoom = 1.0f + event->angleDelta().y() * 0.001f;
    zoomFactor = qBound(0.01f, zoomFactor * zoom, 10.0f);

    QPointF mouseWorldAfter = screenToWorld(event->position().toPoint());
    xOffset -= mouseWorldBefore.x() - mouseWorldAfter.x();
    yOffset -= mouseWorldBefore.y() - mouseWorldAfter.y();

    // Обновляем матрицу вида
    viewMatrix.setToIdentity();
    viewMatrix.scale(zoomFactor, zoomFactor);
    viewMatrix.translate(xOffset, yOffset);
    //viewMatrix.ortho(minX, maxX, minY, maxY, -1.0f, 1.0f);
    update();

    update();
}

void PlotGlWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_R) {
        // Reset view on 'R' key
        zoomFactor = 1.0f;
        xOffset = 0.0f;
        yOffset = 0.0f;
        update();
    }
}
