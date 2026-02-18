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

    int top = 1;
    int bottom = height() - 1;
    int center = height() / 2;
    constexpr const int LETTER_WIDTH = 5;
    constexpr const int LETTER_HEIGHT = 13;
    if(backend().measurement_type == Backend::ZygZag_Odwrocony)
    {
        std::swap(top, bottom);
    }

    QPainter painter(this);
    int painter_x_position = 0;
    painter.setPen(Qt::green);
    painter.drawLine(painter_x_position, center, painter_x_position + backend().A, center);
    if(backend().A > LETTER_WIDTH * 2)
        painter.drawText(painter_x_position + backend().A / 2 - LETTER_WIDTH, center + LETTER_HEIGHT, "A");
    painter_x_position += backend().A;
    painter.setPen(Qt::white);
    painter.drawLine(painter_x_position, center, painter_x_position, top);
    painter.setPen(Qt::blue);
    if(backend().B > LETTER_WIDTH * 2)
        painter.drawText(painter_x_position + backend().B / 2 - LETTER_WIDTH, center + LETTER_HEIGHT, "B");
    painter.drawLine(painter_x_position, top, painter_x_position + backend().B, top);
    painter_x_position += backend().B;
    painter.setPen(Qt::white);
    painter.drawLine(painter_x_position, top, painter_x_position, center);

    switch (backend().measurement_type)
    {
    case Backend::Impulse:
        break;
    case Backend::ZygZag:
    case Backend::ZygZag_Odwrocony:
        painter.setPen(Qt::cyan);
        if(backend().C > LETTER_WIDTH * 2)
            painter.drawText(painter_x_position + backend().C / 2 - LETTER_WIDTH, center + LETTER_HEIGHT, "C");
        painter.drawLine(painter_x_position, center, painter_x_position + backend().C, center);
        painter_x_position += backend().C;
        painter.setPen(Qt::white);
        painter.drawLine(painter_x_position, center, painter_x_position, bottom);
        painter.setPen(Qt::blue);
        if(backend().B > LETTER_WIDTH * 2)
            painter.drawText(painter_x_position + backend().B / 2 - LETTER_WIDTH, center + LETTER_HEIGHT, "B");
        painter.drawLine(painter_x_position, bottom, painter_x_position + backend().B, bottom);
        painter_x_position += backend().B;
        painter.setPen(Qt::white);
        painter.drawLine(painter_x_position, bottom, painter_x_position, center);
        break;
    }
    painter.setPen(Qt::green);
    if(backend().A > LETTER_WIDTH * 2)
        painter.drawText(painter_x_position + backend().A / 2 - LETTER_WIDTH, center + LETTER_HEIGHT, "A");
    painter.drawLine(painter_x_position, center, painter_x_position + backend().A, center);
    painter_x_position += backend().A;

    int readout_height = center - ((double)backend().readout_amplitude / backend().amplitude) * center;
    painter.setPen(Qt::white);
    painter.drawLine(painter_x_position, center, painter_x_position, readout_height);
    painter.setPen(Qt::yellow);
    if(backend().D > LETTER_WIDTH * 2)
        painter.drawText(painter_x_position + backend().D / 2 - LETTER_WIDTH, center + LETTER_HEIGHT, "D");
    painter.drawLine(painter_x_position, readout_height, painter_x_position + backend().D, readout_height);
    painter.setPen(Qt::red);
    //Rysowanie linii triggera
    painter.drawText(painter_x_position + backend().trigger_offset + LETTER_WIDTH, height() - LETTER_HEIGHT, "TRIG");

    painter.drawLine(painter_x_position + backend().trigger_offset, top, painter_x_position + backend().trigger_offset, bottom);
    painter_x_position += backend().D;

    painter.setPen(Qt::white);
    painter.drawLine(painter_x_position, readout_height, painter_x_position, center);
    if(backend().E > LETTER_WIDTH)
        painter.drawText(painter_x_position + backend().E / 2 - LETTER_WIDTH, center + LETTER_HEIGHT, "E");
    painter.drawLine(painter_x_position, center, painter_x_position + backend().E, center);



    // Rysowanie linii co 100 microsekund [ok]
    QPen pen_lines = QPen();
    pen_lines.setColor(Qt::gray);
    pen_lines.setStyle(Qt::PenStyle::DashLine);
    painter.setPen(pen_lines);
    for (int offset = 0; offset < width(); offset += 100)
    {
        painter.drawLine(offset, top, offset, bottom);
        painter.drawText(offset + LETTER_WIDTH, LETTER_HEIGHT, QString::number(offset) + " μs");
    }

    // skalowanie do widget [TODO]
    // kolory oznaczenia [ok]
}
