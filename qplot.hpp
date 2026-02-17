#ifndef QPLOT_HPP
#define QPLOT_HPP

#include "backend.hpp"
#include <QWidget>
#include <QTimer>

class QPlot : public QWidget
{
    Q_OBJECT
    Signal signal;
public:
    QPlot(QWidget* parrent = nullptr);
public slots:
    void repaint();

protected:
    void paintEvent(QPaintEvent* event) override;
    QColor color_red = QColor(255, 0, 0);
};

#endif // QPLOT_HPP
