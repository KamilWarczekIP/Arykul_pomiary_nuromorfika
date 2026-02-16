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
    backend();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{

}

