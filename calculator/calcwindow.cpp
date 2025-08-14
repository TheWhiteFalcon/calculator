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
static int sin_prev_value;
static bool calc_mode;

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
    connect(ui->bt_refresh, SIGNAL(clicked()), this, SLOT(on_bt_refresh_clicked()));

    calc_mode = false;
    sin_prev_value = 1;
    exist_prev_value = false;
    equaled = false;
    operated = false;
    printed = false;

    PlotGlWidget* PlGlWidget = new PlotGlWidget(this);
    ui->verticalLayout->insertWidget(0, PlGlWidget);
    PlGlWidget->setMinimumSize(200, 200);
    PlGlWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

    // Устанавливаем начальные значения для спинбоксов
    ui->dsbox_a->setValue(1.0);
    ui->dsbox_b->setValue(0.0);
    ui->dsbox_c->setValue(0.0);
    ui->dsbox_d->setValue(0.0);

    // Подключаем сигналы изменения значений спинбоксов к обновлению графика
    connect(ui->dsbox_a, &QDoubleSpinBox::valueChanged, this, &CalcWindow::updateGraph);
    connect(ui->dsbox_b, &QDoubleSpinBox::valueChanged, this, &CalcWindow::updateGraph);
    connect(ui->dsbox_c, &QDoubleSpinBox::valueChanged, this, &CalcWindow::updateGraph);
    connect(ui->dsbox_d, &QDoubleSpinBox::valueChanged, this, &CalcWindow::updateGraph);

    QToolButton* bt_mode = new QToolButton(this);
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

    ui->gridLayout->addWidget(ui->bt_sincos, 0, 0, 1, 3);
    ui->gridLayout->addWidget(ui->lbl_a, 1, 3);
    ui->gridLayout->addWidget(ui->lbl_b, 2, 3);
    ui->gridLayout->addWidget(ui->lbl_c, 3, 3);
    ui->gridLayout->addWidget(ui->lbl_d, 4, 3);

    //setting default calc_mode
    ui->calc_result->show();
    ui->prev_result->show();
    ui->bt_backspace->show();
    ui->bt_sincos->hide();
    ui->dsbox_a->hide();
    ui->dsbox_b->hide();
    ui->dsbox_c->hide();
    ui->dsbox_d->hide();
    ui->lbl_a->hide();
    ui->lbl_b->hide();
    ui->lbl_c->hide();
    ui->lbl_d->hide();
    PlGlWidget->hide();
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
        ui->calc_result->show();
        ui->prev_result->show();
        ui->bt_backspace->show();
        ui->bt_sincos->hide();
        ui->dsbox_a->hide();
        ui->dsbox_b->hide();
        ui->dsbox_c->hide();
        ui->dsbox_d->hide();
        ui->bt_divide->show();
        ui->bt_equal->show();
        ui->bt_minus->show();
        ui->bt_multiply->show();
        ui->bt_plus->show();
        ui->bt_plus_minus->show();
        ui->bt_sqrt->show();
        ui->lbl_a->hide();
        ui->lbl_b->hide();
        ui->lbl_c->hide();
        ui->lbl_d->hide();
        PlGlWidget->hide();
        //ui->gridLayout->update();
        calc_mode = false;
    } else {
        ui->calc_result->hide();
        ui->prev_result->hide();
        ui->bt_backspace->hide();
        ui->bt_sincos->show();
        ui->dsbox_a->show();
        ui->dsbox_b->show();
        ui->dsbox_c->show();
        ui->dsbox_d->show();
        ui->bt_divide->hide();
        ui->bt_equal->hide();
        ui->bt_minus->hide();
        ui->bt_multiply->hide();
        ui->bt_plus->hide();
        ui->bt_plus_minus->hide();
        ui->bt_sqrt->hide();
        ui->lbl_a->show();
        ui->lbl_b->show();
        ui->lbl_c->show();
        ui->lbl_d->show();
        PlGlWidget->show();
        //ui->gridLayout->update();
        calc_mode = true;
    }
}


// Добавляем новую функцию для обновления графика
void CalcWindow::updateGraph()
{
    qDebug("updating graph");
    PlotGlWidget* plotWidget = findChild<PlotGlWidget*>();
    if (plotWidget) {
        double a = ui->dsbox_a->value();
        double b = ui->dsbox_b->value();
        double c = ui->dsbox_c->value();
        double d = ui->dsbox_d->value();

        plotWidget->generateFunction(a, b, c, d, sin_prev_value);
    }
}

// Модифицируем функцию on_bt_refresh_clicked
void CalcWindow::on_bt_refresh_clicked()
{
    ui->dsbox_a->setValue(1.00);
    ui->dsbox_b->setValue(0.00);
    ui->dsbox_c->setValue(0.00);
    ui->dsbox_d->setValue(0.00);
    updateGraph();
}

// Модифицируем функцию on_bt_sincos_clicked
void CalcWindow::on_bt_sincos_clicked()
{
    if (sin_prev_value == 1) {
        ui->bt_sincos->setText("cos(x)");
        sin_prev_value = 2;
        updateGraph();
    } else if (sin_prev_value == 2) {
        ui->bt_sincos->setText("ax³+bx²+cx+d");
        sin_prev_value = 3;
        updateGraph();
    } else {
        ui->bt_sincos->setText("sin(x)");
        sin_prev_value = 1;
        updateGraph();
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

