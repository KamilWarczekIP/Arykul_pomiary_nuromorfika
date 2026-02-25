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
        Impulse_odwrocony,
        Impulse_pomiar_scalony,
        ZygZag,
        ZygZag_Odwrocony,
    };
    uint A, B, C, D, wait_time;
    uint repeat_times;
    MeasurementType measurement_type;
    uint amplitude; //milivolts
    uint readout_amplitude; //milivolts
    uint trigger_offset;
    QString filename_suffix;
    QString file_location;
    QString getMeasurementTypeName();
    double getMaxFrequency() const;
    double getSignalTimeInSeconds() const;
    bool analogDiscoveryStatus();
    bool keythleyStatus();
    static constexpr uint MICROSEC_IN_SEC = 1000000;
    double getCalculatedFrequencyOfExcitation();
    void analogDiscoveryfetchMaxSamples();
    void analogDiscoveryfetchMaxRepeat();
    void analogDiscoveryfetchMaxWaitTime();


    int signalSampleCount();
    int analogDiscoveryMaxSamples();
    int analogDiscoveryMaxRepeat();
    double analogDiscoveryMaxWaitTime();

signals:
    void finishedMeasurement();
    void progress(int proc);
    void fail(QString message);

public slots:
    void outputPreview();
    void runMeasurement(double const frequency);

private slots:
    void fail_cleanup(QString message);

private:
    friend class MainWindow;

    Signal generateSignal();
    Signal generateTriggerSignal();
    void setSignal(int channel, Signal& signal);
    double max_freq, min_freq; // 9-12 MHz

    int analog_discovery_max_samples = 4096;
    int analog_discovery_max_repeat = 1000;
    double analog_discovery_max_wait = 1.0;
    static constexpr double GENERATION_FREQ = 1000000; // ~1 MHz
    static constexpr uint VOLTS_IN_MILIVOLT = 1000;
    static constexpr double TRIGGER_AMPLITUDE = 4.5;
    unsigned long defaultRM;  // Resource manager
    unsigned long keythley_handle;   // DMM6500
    std::string visa_address_keythley;

    QString PARAMETER_AUTOSAVE_FILENAME = "parameters.jpx";
};

struct BackendAccess
{
    Backend& operator()();
};

extern BackendAccess backend;

#endif // BACKEND_HPP
