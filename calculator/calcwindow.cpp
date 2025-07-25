#include "calcwindow.h"
#include "./ui_calcwindow.h"
#include "cmath"
#include "QToolButton"
#include "QPixmap"

static bool equaled;
static bool operated;
double prev_value;
static bool exist_prev_value;
static bool printed;
static bool sin_prev_value;
static bool calc_mode;
static bool polynomial_mode;


CalcWindow::CalcWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::CalcWindow)
{
    ui->setupUi(this);

    connect(ui->bt_0,SIGNAL(clicked()), this, SLOT(digits_numbers()));
    connect(ui->bt_1,SIGNAL(clicked()), this, SLOT(digits_numbers()));
    connect(ui->bt_2,SIGNAL(clicked()), this, SLOT(digits_numbers()));
    connect(ui->bt_3,SIGNAL(clicked()), this, SLOT(digits_numbers()));
    connect(ui->bt_4,SIGNAL(clicked()), this, SLOT(digits_numbers()));
    connect(ui->bt_5,SIGNAL(clicked()), this, SLOT(digits_numbers()));
    connect(ui->bt_6,SIGNAL(clicked()), this, SLOT(digits_numbers()));
    connect(ui->bt_7,SIGNAL(clicked()), this, SLOT(digits_numbers()));
    connect(ui->bt_8,SIGNAL(clicked()), this, SLOT(digits_numbers()));
    connect(ui->bt_9,SIGNAL(clicked()), this, SLOT(digits_numbers()));
    connect(ui->bt_plus,SIGNAL(clicked()), this, SLOT(math_signal()));
    connect(ui->bt_minus,SIGNAL(clicked()), this, SLOT(math_signal()));
    connect(ui->bt_divide,SIGNAL(clicked()), this, SLOT(math_signal()));
    connect(ui->bt_multiply,SIGNAL(clicked()), this, SLOT(math_signal()));
    connect(ui->bt_sqrt,SIGNAL(clicked()), this, SLOT(math_signal()));
    connect(ui->bt_sincos,SIGNAL(clicked()), this, SLOT(digits_numbers()));

    calc_mode = false;
    polynomial_mode = false;
    sin_prev_value = true;
    exist_prev_value = false;
    equaled = false;
    operated = false;
    printed = false;

    PlotGlWidget* PlGlWidget = new PlotGlWidget(this);

    ui->verticalLayout->insertWidget(0, PlGlWidget);

    PlGlWidget->setMinimumSize(200, 200);
    PlGlWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    PlGlWidget->setVisible(false);

    //QPixmap pixmap("switch_icon.jpg");
    //QIcon ButtonIcon(pixmap);

    QToolButton* bt_mode = new QToolButton(this);
    //bt_mode->setIcon(ButtonIcon);
    //bt_mode->setIconSize(pixmap.rect().size());
    bt_mode->setText("☰");
    bt_mode->setObjectName("bt_mode");
    bt_mode->setStyleSheet(
        "#bt_mode {"
        "  background: #121212;"
        "  color: white;"
        "  font-size: 16px;"
        "  border-radius: 4px;"
        "}"
        "#bt_mode:hover { background: #666; }"
        "#bt_mode:pressed { background: #888; }"
        );
    int btnSize = qMin(width(), height()) / 8; // relative to the shorter side
    bt_mode->setFixedSize(btnSize, btnSize);
    bt_mode->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(bt_mode, &QToolButton::clicked, this, &CalcWindow::on_bt_mode_clicked);

    ui->gridLayout->addWidget(ui->bt_sincos, 0, 0);

    ui->bt_sincos->setVisible(false);
}


void CalcWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);
    QToolButton* bt_mode = findChild<QToolButton*>("bt_mode");

    if (!bt_mode) {
        return;
    }

    if (bt_mode) {
        bt_mode->move(10, 10);
    }
}


CalcWindow::~CalcWindow()
{
    delete ui;
}

void CalcWindow::digits_numbers()
{
    QPushButton *button = (QPushButton *)sender();
    double current_value;
    if(equaled)
    {
        ui->prev_result->setText("");
        ui->calc_result->setText(button->text());
        equaled = false;
        operated = false;
        exist_prev_value = false;
        printed = true;
    } else if(!operated)
    {
        current_value = (ui->calc_result->text()).toDouble();
        if(QString::number(current_value, 'g', 16).size() < 15)
        {

            current_value = (ui->calc_result->text() + button->text()).toDouble();
            qDebug() << current_value;
        }
        QString value_output = QString::number(current_value, 'g', 16);
        qDebug() <<value_output;
        operated = false;

        ui->calc_result->setText(value_output);
        printed = true;
    } else {
        prev_value = (ui->calc_result->text()).toDouble();
        ui->calc_result->setText(button->text());
        operated = false;
        printed = true;
    }
    updateBackspaceButton();
}

void CalcWindow::math_signal(){
    QPushButton *button = (QPushButton *)sender();
    math_operations(button->text());
}

void CalcWindow::math_operations(QString calc_symbol)
{

    double current_value = (ui->calc_result->text()).toDouble();
    QString value_output;
    double inter_value;

    qDebug() << "Cur:" << ui->calc_result->text() << " prev:" << prev_value << " operated:" << operated;
    if(printed)
    {
        if(calc_symbol == "+")
        {
            qDebug() << calc_symbol;
            if(exist_prev_value)
            {
                inter_value = current_value;
                current_value = prev_value + current_value;
                prev_value = inter_value;
                value_output = QString::number(current_value, 'g', 16);
                ui->prev_result->setText(value_output + " " + calc_symbol);
                value_output = QString::number(current_value, 'g', 16);
                operated = true;
                ui->calc_result->setText(value_output);
            } else
            {
                prev_value = current_value;
                exist_prev_value = true;
                value_output = QString::number(prev_value, 'g', 16);
                ui->prev_result->setText(value_output  + " " + calc_symbol);
                ui->calc_result->setText("0");
            }
        } else if(calc_symbol == "-")
        {
            qDebug() << calc_symbol;
            if(exist_prev_value)
            {
                inter_value = current_value;
                current_value = prev_value - current_value;
                prev_value = inter_value;
                value_output = QString::number(current_value, 'g', 16);
                ui->prev_result->setText(value_output  + " " + calc_symbol);
                value_output = QString::number(current_value, 'g', 16);
                operated = true;
                ui->calc_result->setText(value_output);
            } else
            {
                prev_value = current_value;
                exist_prev_value = true;
                value_output = QString::number(prev_value, 'g', 16);
                ui->prev_result->setText(value_output  + " " + calc_symbol);
                ui->calc_result->setText("0");
            }
        } else if(calc_symbol == "/")
        {
            qDebug() << calc_symbol;
            if(exist_prev_value && current_value != 0)
            {
                inter_value = current_value;
                current_value = prev_value / current_value;
                prev_value = inter_value;
                value_output = QString::number(current_value, 'g', 16);
                ui->prev_result->setText(value_output  + " " + calc_symbol);
                value_output = QString::number(current_value, 'g', 16);
                operated = true;
                ui->calc_result->setText(value_output);
            } else if (exist_prev_value && current_value == 0)
            {
                ui->calc_result->setText("Cannot divide by zero");
                operated = true;
            } else {
                prev_value = current_value;
                exist_prev_value = true;
                value_output = QString::number(prev_value, 'g', 16);
                ui->prev_result->setText(value_output + " " + calc_symbol);
                ui->calc_result->setText("0");
            }
        } else if(calc_symbol == "x")
        {
            qDebug() << calc_symbol;
            if(exist_prev_value)
            {
                inter_value = current_value;
                current_value = prev_value + current_value;
                prev_value = inter_value;
                value_output = QString::number(current_value, 'g', 16);
                ui->prev_result->setText(value_output + " " + calc_symbol);
                value_output = QString::number(current_value, 'g', 19);
                operated = true;
                ui->calc_result->setText(value_output);
            } else
            {
                prev_value = current_value;
                exist_prev_value = true;
                value_output = QString::number(prev_value, 'g', 16);
                ui->prev_result->setText(value_output + " " + calc_symbol);
                ui->calc_result->setText("0");
            }
        }
        printed = false;
    } else if(!printed && exist_prev_value && !equaled)
    {
        value_output = ui->prev_result->text();
        value_output.chop(2);
        ui->prev_result->setText(value_output + " " + calc_symbol);
    }
    if(calc_symbol == "√")
    {
        qDebug() << calc_symbol;
        value_output = QString::number(current_value, 'g', 16);
        ui->prev_result->setText(calc_symbol + " " + value_output);
        inter_value = std::sqrt(current_value);
        value_output = QString::number(inter_value, 'g', 16);
        ui->calc_result->setText(value_output);
        equaled = true;
        operated = true;
        printed = false;
    }
    updateBackspaceButton();
}


void CalcWindow::on_bt_mode_clicked()
{
    PlotGlWidget* PlGlWidget = findChild<PlotGlWidget*>();

    if (!PlGlWidget) {
        return;
    }

    if (calc_mode) {
        ui->calc_result->setVisible(true);
        ui->prev_result->setVisible(true);
        ui->bt_backspace->setVisible(true);
        ui->bt_sincos->setVisible(false);
        PlGlWidget->setVisible(false);
        //ui->gridLayout->update();
        calc_mode = false;
    } else {
        ui->calc_result->setVisible(false);
        ui->prev_result->setVisible(false);
        ui->bt_backspace->setVisible(false);
        ui->bt_sincos->setVisible(true);
        PlGlWidget->setVisible(true);
        //ui->gridLayout->update();
        calc_mode = true;
    }
}


void CalcWindow::on_bt_fx_clicked()
{
    if (polynomial_mode) {
        //enter cubic polynomial
        polynomial_mode = false;
    }
    else {
        //enter sincos mode
        polynomial_mode = true;
    }
}


void CalcWindow::on_bt_sincos_clicked()
{
    if (sin_prev_value) {
        ui->bt_sincos->setText("cos(x)");
        sin_prev_value = false;
    } else {
        ui->bt_sincos->setText("sin(x)");
        sin_prev_value = true;
    }

    PlotGlWidget* plotWidget = findChild<PlotGlWidget*>();
    if (plotWidget) {
        plotWidget->setSinPrevValue(sin_prev_value);
    }
}


void CalcWindow::on_bt_equal_clicked()
{
    qDebug() << "Cur:" << ui->calc_result->text() << " prev:" << prev_value;
    qDebug() << "=";
    double current_value = (ui->calc_result->text()).toDouble();
    QString value_output = QString::number(current_value, 'g', 16);
    QString prev_value_output = ui->prev_result->text();
    if(!equaled)
    {
        value_output = prev_value_output + " " + value_output + " =";
        if(ui->calc_result->text().left(1) != "√")
        {
            QString math_symb = ui->prev_result->text().right(1);
            math_operations(math_symb);
            qDebug() <<value_output;
            ui->prev_result->setText(value_output);

        }
        prev_value = ui->calc_result->text().toDouble();
        equaled = true;
        printed = false;
    } else if(ui->calc_result->text().left(1) != "√"){
        ui->prev_result->setText(value_output + " =");
        equaled = true;
        printed = false;
    }
    operated = false;
    qDebug() << prev_value;
    updateBackspaceButton();
}


void CalcWindow::updateBackspaceButton()
{
    qDebug() << ui->calc_result->text();
    qDebug() << operated;
    if (exist_prev_value && ui->calc_result->text() == "0") {
        ui->bt_backspace->setText("CE");
    } else {
        ui->bt_backspace->setText("C");
    }
}


void CalcWindow::on_bt_backspace_clicked()
{

    if (ui->bt_backspace->text() == "C") {
        ui->calc_result->setText("0");
    } else {
        ui->calc_result->setText("0");
        ui->prev_result->setText("");
        equaled = false;
        exist_prev_value = false;
        operated = false;
    }
    updateBackspaceButton();
}


void CalcWindow::on_bt_plus_minus_clicked()
{
    double current_value; //= (ui->calc_result->text()).toDouble();
    QString value_output; //= Qstring::number(current_Value, 'g', 16);
    current_value = (ui->calc_result->text()).toDouble() * (-1);
    value_output = QString::number(current_value, 'g', 16);
    ui->calc_result->setText(value_output);
}


void CalcWindow::on_bt_comma_clicked()
{
    if(!(ui->calc_result->text().contains('.')))
    {
        ui->calc_result->setText(ui->calc_result->text() + '.');
    }
}

