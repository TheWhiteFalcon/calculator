#ifndef CALCWINDOW_H
#define CALCWINDOW_H

#include <QMainWindow>
#include "cmath"
#include <QOpenGLWidget>
#include "PlotGlWidget.h"
#include "QDoubleSpinBox"

QT_BEGIN_NAMESPACE
namespace Ui {
class CalcWindow;
}
QT_END_NAMESPACE

class CalcWindow : public QMainWindow
{
    Q_OBJECT

public:
    CalcWindow(QWidget *parent = nullptr);
    ~CalcWindow();

protected:
    void resizeEvent(QResizeEvent* event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    Ui::CalcWindow *ui;
    void handleCalculatorInput(const QString& digit);
    void handleCoefficientInput(const QString& digit);
    QDoubleSpinBox* getCurrentCoefficientSpinBox();
    void setActiveCoefficient(int coefficientIndex);
    int currentCoefficient;
    bool coefficientEdited;

private slots:
    void digits_numbers();
    void math_operations(QString calc_symbol);
    void on_bt_refresh_clicked();
    void on_bt_equal_clicked();
    void on_bt_backspace_clicked();
    void on_bt_plus_minus_clicked();
    void math_signal();
    void on_bt_comma_clicked();
    void on_bt_sincos_clicked();
    void updateBackspaceButton();
    void on_bt_mode_clicked();
    void updateGraph();
    void onCoefficientBackspaceClicked();
    void onCoefficientCommaClicked();
    void onCoefficientPlusMinusClicked();
};
#endif // CALCWINDOW_H
