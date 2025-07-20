#ifndef CALCWINDOW_H
#define CALCWINDOW_H

#include <QMainWindow>
#include "cmath"
#include <QOpenGLWidget>
#include "PlotGlWidget.h"

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

private:
    Ui::CalcWindow *ui;
    void updateBackspaceButton();

private slots:
    void digits_numbers();
    void math_operations(QString calc_symbol);
    void on_bt_fx_clicked();
    void on_bt_equal_clicked();
    void on_bt_backspace_clicked();
    void on_bt_plus_minus_clicked();
    void math_signal();
    void on_bt_comma_clicked();
};
#endif // CALCWINDOW_H
