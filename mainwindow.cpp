#include "mainwindow.hpp"
#include "backend.hpp"
#include "qplot.hpp"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>

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
    QObject::connect(&backend(), &Backend::fail, this, &MainWindow::resetAfterFail);
    QObject::connect(&backend(), &Backend::finishedMeasurement, this, &MainWindow::resetUI);
    QObject::connect(ui->pushButton_start, &QPushButton::clicked, &backend(), &Backend::runMeasurement);
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
    ui->spinBox_tigger_offset->setValue(backend().trigger_offset);
    ui->spinBox_tigger_offset->setMinimum(0);
    ui->lineEdit_nazwa->setText(backend().filename_suffix);
    ui->spinBox_liczba_pomiarow->setValue(backend().repeat_times);
    ui->comboBox->setCurrentIndex((int)backend().measurement_type);
    recalculateLimits();
    ui->pushButton_max_czest->click();
    setUIEnabled(false);
    ui->spinBox_C->setEnabled(false);
    ui->spinBox_D->setEnabled(false);
}
void MainWindow::setUIEnabled(bool enabled)
{
    ui->pushButton_max_czest->setEnabled(enabled);
    ui->pushButton_lokalizacja_pliku->setEnabled(enabled);
    ui->spinBox_A->setEnabled(enabled);
    ui->spinBox_B->setEnabled(enabled);
    ui->spinBox_C->setEnabled(enabled);
    ui->spinBox_D->setEnabled(enabled);
    ui->comboBox->setEnabled(enabled);
    on_comboBox_currentIndexChanged(ui->comboBox->currentIndex());
    ui->spinBox_czest->setEnabled(enabled);
    ui->spinBox_amplituda->setEnabled(enabled);
    ui->spinBox_amplituda_odczytu->setEnabled(enabled);
    ui->spinBox_tigger_offset->setEnabled(enabled);
    ui->spinBox_liczba_pomiarow->setEnabled(enabled);
    ui->lineEdit_nazwa->setEnabled(enabled);
}

void MainWindow::sprawdzPopiecieAparatury()
{
    bool analog_status = backend().analogDiscoveryStatus();
    if (analog_status && !analog_data_read_already)
    {
        analog_data_read_already = true;
        loadAnalogDataForFirstTime();
        setUIEnabled(true);
    }
    bool keythley_status = backend().keythleyStatus();
    QString lokalizacja = backend().file_location.left(46);
    if(backend().file_location.size() > 42)
    {
        lokalizacja[43] = '.';
        lokalizacja[44] = '.';
        lokalizacja[45] = '.';
    }
    ui->statusBar->showMessage(QString("Analog Discovery: %1\t\t DMM6500: %2\t\t\t\t Lokalizacja pliku: %3").arg(analog_status ? "🟢 OK" : "🔴 Problem").arg(keythley_status ? "🟢 OK" : "🔴 Problem").arg(lokalizacja));
    ui->statusBar->setToolTip(backend().file_location);
    ui->statusBar->setToolTipDuration(5000);
    ui->pushButton_start->setEnabled(analog_status && keythley_status);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadAnalogDataForFirstTime()
{
    ui->label_ostrzeenie->hide();
    ui->label_ostrzezenie_icon->hide();

    backend().analogDiscoveryfetchMaxSamples();
    backend().analogDiscoveryfetchMaxWaitTime();
    backend().analogDiscoveryfetchMaxRepeat();
    ui->spinBox_liczba_pomiarow->setMaximum(backend().analogDiscoveryMaxRepeat());
}

void MainWindow::recalculateSignalTime()
{

    if(backend().signalSampleCount() >= backend().analogDiscoveryMaxSamples())
    {
        ui->label_czas_sygnalu->setStyleSheet("color: red;");
        ui->label_czas_sygnalu->setText("Czas trwania sygnału: <b> (za długo) " + QString::number(backend().getSignalTimeInSeconds() * backend().MICROSEC_IN_SEC) + " μs<b>");
    }
    else
    {
        ui->label_czas_sygnalu->setText("Czas trwania sygnału: <b>" + QString::number(backend().getSignalTimeInSeconds() * backend().MICROSEC_IN_SEC) + " μs</b>");
        ui->label_czas_sygnalu->setStyleSheet("");
    }
}

void MainWindow::recalculateLimits()
{
    ui->spinBox_czest->setMaximum(1.0 / backend().getSignalTimeInSeconds());
    backend().wait_time = (backend().MICROSEC_IN_SEC / ui->spinBox_czest->value()) - backend().getSignalTimeInSeconds() * backend().MICROSEC_IN_SEC;

    if(backend().measurement_type == Backend::MeasurementType::Impulse_pomiar_scalony)
        ui->spinBox_tigger_offset->setMaximum(ui->spinBox_B->value() - 1);
    else
        ui->spinBox_tigger_offset->setMaximum(ui->spinBox_D->value() - 1);
    emit update_preview();
}

void MainWindow::resetAfterFail(QString message)
{
    QMessageBox msgBox;
    msgBox.setText("Wystąpił problem");
    msgBox.setInformativeText(message);
    msgBox.setStandardButtons(QMessageBox::Ok);
    int ret = msgBox.exec();
    resetUI();
}

void MainWindow::resetUI()
{
        setUIEnabled(true);
        ui->pushButton_start->setText("START");
        ui->progressBar->setValue(0);
        ui->progressBar->setEnabled(false);
        aparatura_timer->start();
        ui->pushButton_start->setEnabled(true);

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
    switch (index)
    {
    case 0:
        backend().measurement_type = Backend::MeasurementType::Impulse;
        ui->spinBox_C->setEnabled(false);
        ui->spinBox_D->setEnabled(true);
        break;
    case 1:
        backend().measurement_type = Backend::MeasurementType::Impulse_odwrocony;
        ui->spinBox_C->setEnabled(false);
        ui->spinBox_D->setEnabled(true);
        break;
    case 2:
        backend().measurement_type = Backend::MeasurementType::Impulse_pomiar_scalony;
        ui->spinBox_D->setEnabled(false);
        ui->spinBox_C->setEnabled(false);
        break;
    case 3:
        backend().measurement_type = Backend::MeasurementType::ZygZag;
        ui->spinBox_C->setEnabled(true);
        ui->spinBox_D->setEnabled(true);
        break;
    case 4:
        backend().measurement_type = Backend::MeasurementType::ZygZag_Odwrocony;
        ui->spinBox_C->setEnabled(true);
        ui->spinBox_D->setEnabled(true);
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
    ui->spinBox_czest->setValue(ui->spinBox_czest->maximum());
}


void MainWindow::on_spinBox_tigger_offset_valueChanged(int arg1)
{
    backend().trigger_offset = arg1;
    emit update_preview();
}


void MainWindow::on_pushButton_start_clicked()
{
    setUIEnabled(false);
    aparatura_timer->stop();
    ui->pushButton_start->setEnabled(false);
    ui->progressBar->setEnabled(true);
    ui->pushButton_start->setText("Czekej mierze...");
}


void MainWindow::on_pushButton_lokalizacja_pliku_clicked()
{
    QString nowa_lokalizacja = QFileDialog::getExistingDirectory(
        nullptr,
        "Wybierz gdzie zapisać wyniki",
        backend().file_location,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
        );
    if(nowa_lokalizacja != "")
        backend().file_location = nowa_lokalizacja;
}


void MainWindow::on_pushButton_clicked()
{
    backend().outputPreview();
}

