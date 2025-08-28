#ifndef PLOTGLWIDGET_H
#define PLOTGLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <vector>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QMatrix4x4>
#include <QPointF>

class PlotGlWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    explicit PlotGlWidget(QWidget* parent = nullptr);
    ~PlotGlWidget();

public slots:
    void generateFunction(double a, double b, double c, double d, int sin_prev_value);

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    void createGrid();

    QOpenGLShaderProgram program;
    QOpenGLShaderProgram axesProgram;
    QOpenGLShaderProgram gridProgram;
    QOpenGLBuffer vbo;
    QOpenGLBuffer axesVbo;
    QOpenGLBuffer gridVbo;
    int gridVertexCount;
    QMatrix4x4 viewMatrix;
    std::vector<float> functionPoints;

    int pointCount = 0;
    float zoomFactor = 1.0f;
    float xOffset = 0.0f;
    float yOffset = 0.0f;
    int sin_prev_value;
    QPoint lastMousePos;

    void createAxes();
    void drawScaleMarkers(QPainter& painter, const QMatrix4x4& transform);
    float calculateStepSize(float range) const;
    QPointF screenToWorld(const QPoint& screenPos) const;
};

#endif // PLOTGLWIDGET_H
