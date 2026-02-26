#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void update_preview();

private slots:
    void sprawdzPopiecieAparatury();
    void recalculateSignalTime();
    void resetAfterFail(QString message);
    void resetUI();

    void on_spinBox_A_valueChanged(int arg1);

    void on_spinBox_B_valueChanged(int arg1);

    void on_spinBox_C_valueChanged(int arg1);

    void on_spinBox_D_valueChanged(int arg1);

    void on_spinBox_czest_valueChanged(int arg1);

    void on_comboBox_currentIndexChanged(int index);

    void on_spinBox_liczba_pomiarow_valueChanged(int arg1);

    void on_spinBox_amplituda_valueChanged(int arg1);

    void on_spinBox_amplituda_odczytu_valueChanged(int arg1);

    void on_lineEdit_nazwa_textChanged(const QString &arg1);

    void on_pushButton_max_czest_clicked();

    void on_spinBox_tigger_offset_valueChanged(int arg1);

    void on_pushButton_start_clicked();

    void on_pushButton_lokalizacja_pliku_clicked();

    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
    void recalculateLimits();
    void loadAnalogDataForFirstTime();
    bool analog_data_read_already = false;

    QTimer* aparatura_timer;
};
#endif // MAINWINDOW_HPP
