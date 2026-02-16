#ifndef BACKEND_HPP
#define BACKEND_HPP

#include <QObject>

struct BackendAccess;

struct Signal
{
    double* samples;
    size_t count;
};

class Backend : public QObject
{
    Q_OBJECT
    friend BackendAccess;
    int analog_handle;
    explicit Backend(QObject *parent = nullptr);
    ~Backend();
    static Backend& instance();

public:
    enum MeasurementType
    {
        Impulse,
        ZygZag,
        ZygZag_Odwrocony,
    };
    uint A, B, C, D;
    uint repeat_times;
    MeasurementType measurement_type;
    uint amplitude;
    uint readout_amplitude;

    double max_freq, min_freq; // 9-12 MHz
    static constexpr uint MICROSEC_IN_SEC = 1000000;
    static constexpr uint VOLTS_IN_MILIVOLT = 1000;
    Signal generateSignal();

signals:
    void finishedMeasurement();
    void generatedPreview(Signal signal);

public slots:
    void generatPreview();
    void sendToKeithley();
    void setupMeasurement();
    void runMeasurement();
    void setSignal(int channel, Signal& signal);
};

struct BackendAccess
{
    Backend& operator()();
};

extern BackendAccess backend;

#endif // BACKEND_HPP
