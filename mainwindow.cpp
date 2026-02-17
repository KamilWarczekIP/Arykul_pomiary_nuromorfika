#include "mainwindow.hpp"
#include "backend.hpp"
#include "qplot.hpp"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->widget->deleteLater();
    QWidget* qplot = new QPlot(this);
    ui->centralwidget->layout()->replaceWidget(ui->widget, qplot);
    QObject::connect(&backend(), &Backend::progress, ui->progressBar, &QProgressBar::setValue);
    QObject::connect(this, &MainWindow::update_preview, dynamic_cast<QPlot*>(qplot), &QPlot::repaint);
    setWindowTitle("Test neuromorfika");

    ui->spinBox_amplituda->setValue(backend().amplitude);
    ui->spinBox_amplituda_odczytu->setValue(backend().readout_amplitude);
    ui->spinBox_A->setValue(backend().A);
    ui->spinBox_B->setValue(backend().B);
    ui->spinBox_C->setValue(backend().C);
    ui->spinBox_D->setValue(backend().D);
    ui->spinBox_liczba_->setValue(backend().repeat_times);
    ui->spinBox_czest->setMaximum(1.0 / backend().getSignalTimeInSeconds());
    ui->spinBox_czest->setValue(1.0 / backend().getSignalTimeInSeconds());
    ui->spinBox_C->setEnabled(backend().measurement_type != Backend::Impulse);
    ui->comboBox->setCurrentIndex((int)backend().measurement_type);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::recalculateLimits()
{
    ui->spinBox_czest->setMaximum(1.0 / backend().getSignalTimeInSeconds());
    backend().E = (backend().MICROSEC_IN_SEC / ui->spinBox_czest->value()) - backend().getSignalTimeInSeconds() * backend().MICROSEC_IN_SEC;
    emit update_preview();
}

void MainWindow::on_pushButton_clicked()
{

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


void MainWindow::on_spinBox_liczba__valueChanged(int arg1)
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

}


void MainWindow::on_pushButton_2_clicked()
{
    ui->spinBox_czest->setValue(ui->spinBox_czest->maximum());
}

