#ifndef QPLOT_HPP
#define QPLOT_HPP

#include <QWidget>

class QPlot : public QWidget
{
    Q_OBJECT
public:
    QPlot(QWidget* parrent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
};

#endif // QPLOT_HPP
