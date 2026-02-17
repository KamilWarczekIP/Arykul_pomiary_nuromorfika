#include "qplot.hpp"
#include <QPainter>


QPlot::QPlot(QWidget* parrent)
{
    this->setParent(parrent);
    this->setMinimumSize(0,200);
    signal.count = 0;
    signal.samples = nullptr;
}
void QPlot::repaint()
{
    update();
}


void QPlot::paintEvent(QPaintEvent* event)
{

    int top = 0;
    int bottom = height() - 1;
    int center = height() / 2;
    constexpr const int LETTER_WIDTH = 5;
    constexpr const int LETTER_HEIGHT = 13;
    if(backend().measurement_type == Backend::ZygZag_Odwrocony)
    {
        std::swap(top, bottom);
    }

    QPainter painter(this);
    painter.setBrush(Qt::white);
    int painter_x_position = 0;
    painter.drawText(painter_x_position + backend().A / 2 - LETTER_WIDTH, center + LETTER_HEIGHT, "A");
    painter.drawLine(painter_x_position, center, painter_x_position + backend().A, center);
    painter_x_position += backend().A;
    painter.drawLine(painter_x_position, center, painter_x_position, top);
    painter.drawText(painter_x_position + backend().B / 2 - LETTER_WIDTH, center + LETTER_HEIGHT, "B");
    painter.drawLine(painter_x_position, top, painter_x_position + backend().B, top);
    painter_x_position += backend().B;
    painter.drawLine(painter_x_position, top, painter_x_position, center);

    switch (backend().measurement_type)
    {
    case Backend::Impulse:
        painter.drawText(painter_x_position + backend().A / 2 - LETTER_WIDTH, center + LETTER_HEIGHT, "A");
        painter.drawLine(painter_x_position, center, painter_x_position + backend().A, center);
        painter_x_position += backend().A;
        break;
    case Backend::ZygZag:
    case Backend::ZygZag_Odwrocony:
        painter.drawText(painter_x_position + backend().C / 2 - LETTER_WIDTH, center + LETTER_HEIGHT, "C");
        painter.drawLine(painter_x_position, center, painter_x_position + backend().C, center);
        painter_x_position += backend().C;
        painter.drawLine(painter_x_position, center, painter_x_position, bottom);
        painter.drawText(painter_x_position + backend().B / 2 - LETTER_WIDTH, center + LETTER_HEIGHT, "B");
        painter.drawLine(painter_x_position, bottom, painter_x_position + backend().B, bottom);
        painter_x_position += backend().B;
        painter.drawLine(painter_x_position, bottom, painter_x_position, center);
        painter.drawText(painter_x_position + backend().A / 2 - LETTER_WIDTH, center + LETTER_HEIGHT, "A");
        painter.drawLine(painter_x_position, center, painter_x_position + backend().A, center);
        painter_x_position += backend().A;
        break;
    }

    int readout_height = center - ((double)backend().readout_amplitude / backend().amplitude) * center;
    painter.drawLine(painter_x_position, center, painter_x_position, readout_height);
    painter.drawText(painter_x_position + backend().D / 2 - LETTER_WIDTH, center + LETTER_HEIGHT, "D");
    painter.drawLine(painter_x_position, readout_height, painter_x_position + backend().D, readout_height);
    painter_x_position += backend().D;
    painter.drawLine(painter_x_position, readout_height, painter_x_position, center);
    painter.drawText(painter_x_position + backend().E / 2 - LETTER_WIDTH, center + LETTER_HEIGHT, "E");
    painter.drawLine(painter_x_position, center, painter_x_position + backend().E, center);

    // Rysowanie linii co 100 microsekund
    // skalowanie do widgetu
    // kolory oznaczenia
}
