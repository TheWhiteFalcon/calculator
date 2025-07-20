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

class PlotGlWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    explicit PlotGlWidget(QWidget* parent = nullptr);
    ~PlotGlWidget();

protected:
    // Core OpenGL overrides
    void initializeGL() override;    // Initialize OpenGL resources
    void paintGL() override;        // Render the scene
    void resizeGL(int w, int h) override; // Handle resize events

    // Mouse and keyboard interaction
    void mousePressEvent(QMouseEvent* event) override;    // Handle mouse press
    void mouseMoveEvent(QMouseEvent* event) override;     // Handle mouse movement
    void wheelEvent(QWheelEvent* event) override;         // Handle mouse wheel
    void keyPressEvent(QKeyEvent* event) override;

private:
    QOpenGLShaderProgram program;
    QOpenGLShaderProgram axesProgram;
    QOpenGLBuffer vbo;
    QOpenGLBuffer axesVbo;
    std::vector<float> functionPoints;
    int pointCount = 0;
    float zoomFactor = 1.0f;
    float xOffset = 0.0f;
    float yOffset = 0.0f;
    QPoint lastMousePos;

    void generateSineWave();
    void createAxes();
    void drawScaleMarkers(QPainter& painter); // Draw scale markers and labels
    float calculateStepSize(float range) const; // Calculate optimal step size for scale markers
};

#endif // PLOTGLWIDGET_H
