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
    gridVbo.destroy();
}

void PlotGlWidget::createGrid()
{
    std::vector<float> gridVertices;

    const int range = 100;
    const int step = 5;

    // Horizontal grid lines
    for (int y = -range; y <= range; y += step) {
        if (y == 0) continue;
        gridVertices.push_back(-range);
        gridVertices.push_back(static_cast<float>(y));
        gridVertices.push_back(range);
        gridVertices.push_back(static_cast<float>(y));
    }

    // Vertical grid lines
    for (int x = -range; x <= range; x += step) {
        if (x == 0) continue;
        gridVertices.push_back(static_cast<float>(x));
        gridVertices.push_back(-range);
        gridVertices.push_back(static_cast<float>(x));
        gridVertices.push_back(range);
    }

    if (!gridVbo.isCreated()) {
        gridVbo.create();
    }

    gridVbo.bind();
    gridVbo.allocate(gridVertices.data(), gridVertices.size() * sizeof(float));
    gridVbo.release();

    gridVertexCount = gridVertices.size() / 2;
}

void PlotGlWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    // Main graph shader program
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

    // Axes shader program
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

    // Grid shader program
    gridProgram.addShaderFromSourceCode(QOpenGLShader::Vertex,
                                        "#version 330 core\n"
                                        "layout(location = 0) in vec2 position;\n"
                                        "uniform mat4 transform;\n"
                                        "void main() {\n"
                                        "   gl_Position = transform * vec4(position, 0.0, 1.0);\n"
                                        "}");
    gridProgram.addShaderFromSourceCode(QOpenGLShader::Fragment,
                                        "#version 330 core\n"
                                        "out vec4 outColor;\n"
                                        "void main() {\n"
                                        "   outColor = vec4(0.5, 0.5, 0.5, 0.3);\n"
                                        "}");
    gridProgram.link();

    viewMatrix.setToIdentity();
    viewMatrix.scale(0.1f, 0.1f);
    zoomFactor = 0.1f;

    generateFunction(1.0, 0.0, 0.0, 0.0, 1);
    createAxes();
    createGrid();
}

void PlotGlWidget::generateFunction(double a, double b, double c, double d, int functionType)
{
    const int segments = 500;
    functionPoints.clear();
    functionPoints.reserve(segments * 2);

    for (int i = 0; i < segments; ++i) {
        float x = -100.0f + 200.0f * i / segments;
        float y;

        switch(functionType) {
        case 1: // sin(x)
            y = sin(x) * a;
            break;
        case 2: // cos(x)
            y = cos(x) * a;
            break;
        case 3: // polynomial
            y = a * x * x * x + b * x * x + c * x + d;
            break;
        default:
            y = sin(x);
        }

        functionPoints.push_back(x);
        functionPoints.push_back(y);
    }

    pointCount = functionPoints.size() / 2;

    if (!vbo.isCreated()) {
        vbo.create();
    }

    vbo.bind();
    vbo.allocate(functionPoints.data(), functionPoints.size() * sizeof(float));
    vbo.release();
    update();
}

void PlotGlWidget::createAxes()
{
    std::vector<float> axes = {
        -100.0f, 0.0f, 100.0f, 0.0f,    // X axis
        0.0f, -100.0f, 0.0f, 100.0f,    // Y axis
        9.5f, 0.5f, 10.0f, 0.0f,        // X arrow part 1
        9.5f, -0.5f, 10.0f, 0.0f,       // X arrow part 2
        -0.5f, 9.5f, 0.0f, 10.0f,       // Y arrow part 1
        0.5f, 9.5f, 0.0f, 10.0f         // Y arrow part 2
    };

    axesVbo.create();
    axesVbo.bind();
    axesVbo.allocate(axes.data(), axes.size() * sizeof(float));
    axesVbo.release();
}

void PlotGlWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw grid
    if (gridVbo.isCreated() && gridVertexCount > 0) {
        gridProgram.bind();
        gridProgram.setUniformValue("transform", viewMatrix);

        gridVbo.bind();
        gridProgram.enableAttributeArray(0);
        gridProgram.setAttributeBuffer(0, GL_FLOAT, 0, 2);

        glDrawArrays(GL_LINES, 0, gridVertexCount);

        gridVbo.release();
        gridProgram.release();
    }

    // Draw axes
    axesProgram.bind();
    axesProgram.setUniformValue("transform", viewMatrix);
    axesVbo.bind();
    axesProgram.enableAttributeArray(0);
    axesProgram.setAttributeBuffer(0, GL_FLOAT, 0, 2);

    glDrawArrays(GL_LINES, 0, 4);
    glDrawArrays(GL_LINES, 4, 8);

    axesProgram.release();

    // Draw function graph
    program.bind();
    program.setUniformValue("transform", viewMatrix);
    vbo.bind();
    program.enableAttributeArray(0);
    program.setAttributeBuffer(0, GL_FLOAT, 0, 2);
    glLineWidth(2.0f);
    glDrawArrays(GL_LINE_STRIP, 0, pointCount);
    glLineWidth(1.0f);
    program.release();

    // Draw labels
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

    viewMatrix.setToIdentity();
    viewMatrix.scale(zoomFactor, zoomFactor);
    viewMatrix.translate(xOffset, yOffset);
    update();
}

void PlotGlWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_R) {
        zoomFactor = 1.0f;
        xOffset = 0.0f;
        yOffset = 0.0f;
        update();
    }
}
