#include "PlotGlWidget.h"
#include <cmath>
#include <QMouseEvent>
#include <QPainter>
#include <QFontMetrics>

PlotGlWidget::PlotGlWidget(QWidget *parent)
    : QOpenGLWidget(parent),
    vbo(QOpenGLBuffer::VertexBuffer),
    axesVbo(QOpenGLBuffer::VertexBuffer),
    zoomFactor(1.0f),
    xOffset(0.0f),
    yOffset(0.0f),
    lastMousePos(QPoint(0,0))
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
}

PlotGlWidget::~PlotGlWidget()
{
    vbo.destroy();
    axesVbo.destroy();
}

void PlotGlWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    // Main shader program for the sine wave
    program.addShaderFromSourceCode(QOpenGLShader::Vertex,
                                    "#version 330 core\n"
                                    "layout(location = 0) in vec2 position;\n"
                                    "void main() {\n"
                                    "   gl_Position = vec4(position, 0.0, 1.0);\n"
                                    "}");

    program.addShaderFromSourceCode(QOpenGLShader::Fragment,
                                    "#version 330 core\n"
                                    "out vec4 outColor;\n"
                                    "void main() {\n"
                                    "   outColor = vec4(0.0, 0.8, 1.0, 1.0);\n"  // Blue color for graph
                                    "}");
    program.link();

    // Shader program for axes and arrows
    axesProgram.addShaderFromSourceCode(QOpenGLShader::Vertex,
                                        "#version 330 core\n"
                                        "layout(location = 0) in vec2 position;\n"
                                        "void main() {\n"
                                        "   gl_Position = vec4(position, 0.0, 1.0);\n"
                                        "}");

    axesProgram.addShaderFromSourceCode(QOpenGLShader::Fragment,
                                        "#version 330 core\n"
                                        "out vec4 outColor;\n"
                                        "void main() {\n"
                                        "   outColor = vec4(1.0, 1.0, 1.0, 1.0);\n"  // White color for axes
                                        "}");
    axesProgram.link();

    // Generate sine wave points
    generateSineWave();
    createAxes();
}

void PlotGlWidget::generateSineWave()
{
    const int segments = 500;
    functionPoints.clear();
    functionPoints.reserve(segments * 2);

    for (int i = 0; i <= segments; ++i) {
        float x = -2.0f * M_PI + 4.0f * M_PI * i / segments; // Range [-2π, 2π]
        float y = sin(x);
        functionPoints.push_back(x);
        functionPoints.push_back(y);
    }

    pointCount = functionPoints.size() / 2;

    if (vbo.isCreated()) {
        vbo.bind();
        vbo.allocate(functionPoints.data(), functionPoints.size() * sizeof(float));
        vbo.release();
    }
}

void PlotGlWidget::createAxes()
{
    std::vector<float> axes = {
        // X axis
        -1.0f, 0.0f,
        1.0f, 0.0f,
        // Y axis
        0.0f, -1.0f,
        0.0f, 1.0f,
        // X arrow
        0.95f, 0.05f,
        1.0f, 0.0f,
        0.95f, -0.05f,
        // Y arrow
        0.05f, 0.95f,
        0.0f, 1.0f,
        -0.05f, 0.95f
    };

    axesVbo.create();
    axesVbo.bind();
    axesVbo.allocate(axes.data(), axes.size() * sizeof(float));
}

void PlotGlWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);

    // Set up transformation matrix
    QMatrix4x4 transform;
    transform.scale(zoomFactor, zoomFactor);
    transform.translate(xOffset, yOffset);

    // Draw axes
    axesProgram.bind();
    axesVbo.bind();

    axesProgram.setUniformValue("transform", transform);
    axesProgram.enableAttributeArray(0);
    axesProgram.setAttributeBuffer(0, GL_FLOAT, 0, 2);

    glDrawArrays(GL_LINES, 0, 4);
    glDrawArrays(GL_TRIANGLES, 4, 3);
    glDrawArrays(GL_TRIANGLES, 7, 3);

    axesProgram.release();

    // Draw sine wave
    if (pointCount > 0) {
        program.bind();
        vbo.bind();

        program.setUniformValue("transform", transform);
        program.enableAttributeArray(0);
        program.setAttributeBuffer(0, GL_FLOAT, 0, 2);

        glLineWidth(2.0f);
        glDrawArrays(GL_LINE_STRIP, 0, pointCount);
        glLineWidth(1.0f);

        program.release();
    }

    // Draw axis labels using QPainter
    QPainter painter(this);
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 10));

    // X axis label
    painter.drawText(width() - 20, height() / 2 - 5, "X");

    // Y axis label
    painter.drawText(width() / 2 + 5, 20, "Y");

    // Draw scale markers
    drawScaleMarkers(painter);

    painter.end();
}

void PlotGlWidget::drawScaleMarkers(QPainter& painter)
{
    // Calculate visible range
    float visibleLeft = -1.0f/zoomFactor - xOffset;
    float visibleRight = 1.0f/zoomFactor - xOffset;
    float visibleBottom = -1.0f/zoomFactor - yOffset;
    float visibleTop = 1.0f/zoomFactor - yOffset;

    // Draw X axis markers
    float xStep = calculateStepSize(visibleRight - visibleLeft);
    for (float x = std::ceil(visibleLeft/xStep)*xStep; x <= visibleRight; x += xStep) {
        int screenX = ((x + xOffset) * zoomFactor + 1.0f) * 0.5f * width();
        painter.drawLine(screenX, height()/2 - 5, screenX, height()/2 + 5);
        painter.drawText(screenX - 10, height()/2 + 20, QString::number(x, 'g', 2));
    }

    // Draw Y axis markers
    float yStep = calculateStepSize(visibleTop - visibleBottom);
    for (float y = std::ceil(visibleBottom/yStep)*yStep; y <= visibleTop; y += yStep) {
        int screenY = height() - ((y + yOffset) * zoomFactor + 1.0f) * 0.5f * height();
        painter.drawLine(width()/2 - 5, screenY, width()/2 + 5, screenY);
        painter.drawText(width()/2 + 10, screenY + 5, QString::number(y, 'g', 2));
    }
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

void PlotGlWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        QPoint delta = event->pos() - lastMousePos;
        xOffset += 2.0f * delta.x() / (width() * zoomFactor);
        yOffset -= 2.0f * delta.y() / (height() * zoomFactor);
        lastMousePos = event->pos();
        update();
    }
}

void PlotGlWidget::wheelEvent(QWheelEvent *event)
{
    float zoom = 1.0f + event->angleDelta().y() * 0.001f;
    zoomFactor *= zoom;
    zoomFactor = qBound(0.1f, zoomFactor, 10.0f); // Limit zoom range
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
