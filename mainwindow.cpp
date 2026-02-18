#include "mainwindow.hpp"
#include "backend.hpp"
#include "qplot.hpp"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , aparatura_timer(new QTimer())
{
    ui->setupUi(this);
    ui->widget->deleteLater();
    QWidget* qplot = new QPlot(this);
    ui->centralwidget->layout()->replaceWidget(ui->widget, qplot);
    QObject::connect(&backend(), &Backend::progress, ui->progressBar, &QProgressBar::setValue);
    QObject::connect(this, &MainWindow::update_preview, dynamic_cast<QPlot*>(qplot), &QPlot::repaint);
    QObject::connect(this, &MainWindow::update_preview, this, &MainWindow::recalculateSignalTime);
    setWindowTitle("Test neuromorfika");

    aparatura_timer->setInterval(500);
    QObject::connect(aparatura_timer, &QTimer::timeout, this, &MainWindow::sprawdzPopiecieAparatury);
    aparatura_timer->start();

    ui->spinBox_amplituda->setValue(backend().amplitude);
    ui->spinBox_amplituda_odczytu->setValue(backend().readout_amplitude);
    ui->spinBox_A->setValue(backend().A);
    ui->spinBox_B->setValue(backend().B);
    ui->spinBox_C->setValue(backend().C);
    ui->spinBox_D->setValue(backend().D);
    ui->spinBox_liczba_pomiarow->setValue(backend().repeat_times);
    ui->spinBox_C->setEnabled(backend().measurement_type != Backend::Impulse);
    ui->comboBox->setCurrentIndex((int)backend().measurement_type);

    recalculateLimits();
    ui->spinBox_czest->setValue(1.0 / backend().getSignalTimeInSeconds());
}

void MainWindow::sprawdzPopiecieAparatury()
{
    bool analog_status = backend().analogDiscoveryStatus();
    bool keythley_status = backend().keythleyStatus();
    ui->statusBar->showMessage(QString("Analog Discovery: %1\t\t DMM6500: %2").arg(analog_status ? "OK" : "Problem").arg(keythley_status ? "OK" : "Problem"));
    ui->pushButton_start->setEnabled(analog_status && keythley_status);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::recalculateSignalTime()
{
    ui->label_czas_sygnalu->setText("Czas trwania sygnału: <b>" + QString::number(backend().getSignalTimeInSeconds() * backend().MICROSEC_IN_SEC) + " μs</b>");
}

void MainWindow::recalculateLimits()
{
    ui->spinBox_czest->setMaximum(1.0 / backend().getSignalTimeInSeconds());
    backend().E = (backend().MICROSEC_IN_SEC / ui->spinBox_czest->value()) - backend().getSignalTimeInSeconds() * backend().MICROSEC_IN_SEC;

    ui->spinBox_tigger_offset->setMaximum(ui->spinBox_D->value() - 1);
    emit update_preview();
}


void MainWindow::on_spinBox_A_valueChanged(int arg1)
{
    backend().A = arg1;
    recalculateLimits();
}


void MainWindow::on_spinBox_B_valueChanged(int arg1)
{
    backend().B = arg1;
    recalculateLimits();
}


void MainWindow::on_spinBox_C_valueChanged(int arg1)
{
    backend().C = arg1;
    recalculateLimits();
}


void MainWindow::on_spinBox_D_valueChanged(int arg1)
{
    backend().D = arg1;
    recalculateLimits();
}


void MainWindow::on_spinBox_czest_valueChanged(int arg1)
{
    recalculateLimits();
}


void MainWindow::on_comboBox_currentIndexChanged(int index)
{
    switch (index) {
    case 0:
        backend().measurement_type = Backend::MeasurementType::Impulse;
        ui->spinBox_C->setEnabled(false);
        break;
    case 1:
        backend().measurement_type = Backend::MeasurementType::ZygZag;
        ui->spinBox_C->setEnabled(true);
        break;
    case 2:
        backend().measurement_type = Backend::MeasurementType::ZygZag_Odwrocony;
        ui->spinBox_C->setEnabled(true);
        break;
    }
    recalculateLimits();
}


void MainWindow::on_spinBox_liczba_pomiarow_valueChanged(int arg1)
{
    backend().repeat_times = arg1;
}


void MainWindow::on_spinBox_amplituda_valueChanged(int arg1)
{
    backend().amplitude = arg1;
    ui->spinBox_amplituda_odczytu->setMaximum(arg1);
    emit update_preview();
}


void MainWindow::on_spinBox_amplituda_odczytu_valueChanged(int arg1)
{
    backend().readout_amplitude = arg1;
    emit update_preview();
}


void MainWindow::on_lineEdit_nazwa_textChanged(const QString &arg1)
{
    backend().filename_suffix = arg1;
}


void MainWindow::on_pushButton_max_czest_clicked()
{
    // backend().max_freq = 10000000;
    // Signal sig = backend().generateSignal();
    // QString str = "";
    // for (int var = 0; var < sig.count; ++var)
    // {
    //     str += QString::number(sig.samples[var]);
    //     str += ", ";
    // }
    // qDebug() << str;

    ui->spinBox_czest->setValue(ui->spinBox_czest->maximum());
}


void MainWindow::on_spinBox_tigger_offset_valueChanged(int arg1)
{
    backend().trigger_offset = arg1;
    emit update_preview();
}


void MainWindow::on_pushButton_start_clicked()
{
    aparatura_timer->stop();
    ui->pushButton_start->setEnabled(false);
    ui->progressBar->setEnabled(true);
    backend().runMeasurement(ui->spinBox_czest->value());
    ui->pushButton_start->setEnabled(true);
    ui->progressBar->setEnabled(false);
    aparatura_timer->start();
}

