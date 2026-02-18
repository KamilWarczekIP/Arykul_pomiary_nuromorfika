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
    uint A, B, C, D, E;
    uint repeat_times;
    MeasurementType measurement_type;
    uint amplitude; //milivolts
    uint readout_amplitude; //milivolts
    uint trigger_offset;
    QString filename_suffix;
    double getMaxFrequency() const;
    double getSignalTimeInSeconds() const;
    bool analogDiscoveryStatus();
    bool keythleyStatus();
    static constexpr uint MICROSEC_IN_SEC = 1000000;

signals:
    void finishedMeasurement();
    void generatedPreview(Signal signal);
    void progress(int proc);
    void fail();

public slots:
    void generatePreview();
    void runMeasurement(double const frequency);

private:
    friend class MainWindow;

    Signal generateSignal();
    double max_freq, min_freq; // 9-12 MHz
    static constexpr uint VOLTS_IN_MILIVOLT = 1000;
    static constexpr double TRIGGER_AMPLITUDE = 4.5;
    Signal generateTriggerSignal();
    void setSignal(int channel, Signal& signal);
    void sendToKeithley();
    unsigned long defaultRM;  // Resource manager
    unsigned long keythley_handle;   // DMM6500
    std::string visa_address_keythley;
};

struct BackendAccess
{
    Backend& operator()();
};

extern BackendAccess backend;

#endif // BACKEND_HPP
