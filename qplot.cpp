#include "qplot.hpp"
#include <QPainter>

QPlot::QPlot(QWidget* parrent)
{
    this->setParent(parrent);
    this->setMinimumSize(0,200);
}

void QPlot::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setBrush(Qt::yellow);
    painter.drawRect(0, 0, width() / 50, height());
    painter.setBrush(Qt::red);
    painter.drawRect(width() / 50, 0, width(), height());
}
