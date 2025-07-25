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
    viewMatrix.setToIdentity();
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
}

QPointF PlotGlWidget::screenToWorld(const QPoint& screenPos) const
{
    // Преобразуем экранные координаты в нормализованные [-1, 1]
    float ndcX = 2.0f * screenPos.x() / width() - 1.0f;
    float ndcY = 1.0f - 2.0f * screenPos.y() / height();

    // Учитываем текущий зум и смещение
    float worldX = (ndcX / zoomFactor) - xOffset;
    float worldY = (ndcY / zoomFactor) - yOffset;

    return QPointF(worldX, worldY);
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

    // Шейдер для осей и графика
    program.addShaderFromSourceCode(QOpenGLShader::Vertex,
                                    "#version 330 core\n"
                                    "layout(location = 0) in vec2 position;\n"
                                    "out vec2 pos;\n"
                                    "uniform mat4 transform;\n"
                                    "void main() {\n"
                                    "   gl_Position = transform * vec4(position, 0.0, 1.0);\n"
                                    "pos = position;\n"
                                    "}");
    program.addShaderFromSourceCode(QOpenGLShader::Fragment,
                                    "#version 330 core\n"
                                    "out vec4 outColor;\n"
                                    "in vec2 pos;\n"
                                    "void main() {\n"
                                    "   outColor = vec4(pos.x, 0.8, 1.0, 1.0);\n" // Цвет графика
                                    "}");
    program.link();

    axesProgram.addShaderFromSourceCode(QOpenGLShader::Vertex,
                                        "#version 330 core\n"
                                        "layout(location = 0) in vec2 position;\n"
                                        "uniform mat4 transform;\n"
                                        "void main() {\n"
                                        "   gl_Position = transform * vec4(position, 0.0, 1.0);\n"
                                        //"if (position.x == 10.0f) glPosition.x = 1.;\n"

                                        "}");
    axesProgram.addShaderFromSourceCode(QOpenGLShader::Fragment,
                                        "#version 330 core\n"
                                        "out vec4 outColor;\n"
                                        "void main() {\n"
                                        "   outColor = vec4(1.0, 1.0, 1.0, 1.0);\n" // Цвет осей
                                        "}");
    axesProgram.link();

    // Инициализация матрицы вида
    viewMatrix.setToIdentity();
    viewMatrix.scale(0.1f, 0.1f); // Начальный масштаб
    zoomFactor = 0.1f;

    generateSineWave();
    createAxes();
}


void PlotGlWidget::generateSineWave()
{
    const int segments = 500;
    functionPoints.clear();
    functionPoints.reserve(segments * 2);

    for (int i = 0; i < segments; ++i) {
        float x = -2.0f * M_PI + 4.0f * M_PI * i / segments; // Range [-2π, 2π]
        float y = sin_prev_value ? cos(x) : sin(x); // Function change
        functionPoints.push_back(x);
        functionPoints.push_back(y);
    }

    pointCount = functionPoints.size() / 2;

    vbo.create();

    if (vbo.isCreated()) {
        vbo.bind();
        vbo.allocate(functionPoints.data(), functionPoints.size() * sizeof(float));
        vbo.release();
    }
}


void PlotGlWidget::setSinPrevValue(bool value)
{
    sin_prev_value = value;
    generateSineWave();
    update();
}


void PlotGlWidget::createAxes()
{
    // Создаем оси, которые будут масштабироваться вместе с графиком
    std::vector<float> axes = {
        // X axis (от -10 до 10 в мировых координатах)
        -10.0f, 0.0f,
        10.0f, 0.0f,
        // Y axis (от -10 до 10 в мировых координатах)
        0.0f, -10.0f,
        0.0f,  10.0f,
        // X arrow (относительные координаты)
        9.5f, 0.5f,
        10.0f, 0.0f,
        9.5f, -0.5f,
        // Y arrow (относительные координаты)
        0.5f, 9.5f,
        0.0f, 10.0f,
        -0.5f, 9.5f
    };

    axesVbo.create();
    axesVbo.bind();
    axesVbo.allocate(axes.data(), axes.size() * sizeof(float));
}


void PlotGlWidget::drawScaleMarkers(QPainter& painter, const QMatrix4x4& transform)
{
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 10));

    // Получаем границы видимой области в мировых координатах
    QPointF worldMin = screenToWorld(QPoint(0, height()));
    QPointF worldMax = screenToWorld(QPoint(width(), 0));

    /*
    // Метки оси X
    float xStep = calculateStepSize(worldMax.x() - worldMin.x());
    for (float x = std::ceil(worldMin.x()/xStep)*xStep; x <= worldMax.x(); x += xStep) {
        QVector4D pos = transform * QVector4D(x, 0, 0, 1);
        int screenX = (pos.x() + 1.0f) * 0.5f * width();
        painter.drawLine(screenX, height()/2 - 5, screenX, height()/2 + 5);
        painter.drawText(screenX - 10, height()/2 + 20, QString::number(x, 'g', 2));
    }

    // Метки оси Y
    //float yStep = calculateStepSize(worldMax.y() - worldMin.y());
    for (float y = -10; y <= 10; y += 1) {
        QVector4D pos = transform * QVector4D(0, y, 0, 1);
        int screenY = pos.y();
        painter.drawLine(0.5, screenY, 0.05, screenY);
        painter.drawText(0.5, screenY, QString::number(y, 'g', 2));
    }


    // Фиксированные подписи осей (в углах экрана)
    painter.drawText(width() - 30, height()/2 + 30, "X");
    painter.drawText(width()/2 + 30, 30, "Y");

*/
}


void PlotGlWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);

    // Отрисовка осей с единой матрицей
    axesProgram.bind();
    axesProgram.setUniformValue("transform", viewMatrix);
    axesVbo.bind();
    axesProgram.enableAttributeArray(0);
    axesProgram.setAttributeBuffer(0, GL_FLOAT, 0, 2);
    glDrawArrays(GL_LINES, 0, 4);
    glDrawArrays(GL_TRIANGLES, 4, 3);
    glDrawArrays(GL_TRIANGLES, 7, 3);
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

    // Отрисовка меток с учетом преобразований
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    drawScaleMarkers(painter, viewMatrix);
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
    xOffset += mouseWorldBefore.x() - mouseWorldAfter.x();
    yOffset += mouseWorldBefore.y() - mouseWorldAfter.y();

    // Обновляем матрицу вида
    viewMatrix.setToIdentity();
    viewMatrix.scale(zoomFactor, zoomFactor);
    viewMatrix.translate(xOffset, yOffset);

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
