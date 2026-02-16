#include "backend.hpp"
#include <qdebug.h>
#include <visa.h>
#include <dwf.h>

static constexpr int CHANNEL0 = 0;
static constexpr int CHANNEL1 = 1;
static constexpr int CHANNEL_BOTH = -1;

BackendAccess backend = BackendAccess();

Backend::Backend(QObject *parent)
    : QObject{parent}
{
    FDwfParamSet(DwfParamOnClose, 0);
    FDwfDeviceOpen(-1, &analog_handle);
    FDwfDeviceAutoConfigureSet(analog_handle, 0);
    FDwfDeviceReset(analog_handle);
    FDwfAnalogOutFrequencyInfo(analog_handle, CHANNEL0, &min_freq, &max_freq);
}

Backend::~Backend()
{
    FDwfDeviceCloseAll();
}


void Backend::generatPreview()
{




}

Signal Backend::generateSignal()
{
    double normalized_readout_amplitude = (double)readout_amplitude / amplitude * VOLTS_IN_MILIVOLT;
    double normalized_amplitude = amplitude * VOLTS_IN_MILIVOLT * (measurement_type == ZygZag_Odwrocony ?  - 1.0 : 1.0);

    // Liczba sekund * częstotliwość = liczba sampli
    Signal signal;
    signal.count = 0;

    double A_sec = ((double)A / MICROSEC_IN_SEC);
    double B_sec = ((double)B / MICROSEC_IN_SEC);
    double C_sec = ((double)C / MICROSEC_IN_SEC);
    double D_sec = ((double)D / MICROSEC_IN_SEC);

    uint A_samples = A_sec * max_freq;
    uint B_samples = B_sec * max_freq;
    uint C_samples = C_sec * max_freq;
    uint D_samples = D_sec * max_freq;

    switch(measurement_type)
    {
    case ZygZag:
    case ZygZag_Odwrocony:
        signal.count += C_samples;
        signal.count += B_samples;
    case Impulse:
        signal.count += A_samples;
        signal.count += B_samples;
        signal.count += D_samples;
        break;
    }

    signal.samples = new double[signal.count];

    size_t index = 0;
    size_t last_index = 0;
    for(last_index = index; index < A_samples + last_index; index++)
    {
        signal.samples[index] = 0.0;
    }
    for(last_index = index; index < B_samples + last_index; index++)
    {
        signal.samples[index] = normalized_amplitude;
    }

    if(measurement_type == ZygZag_Odwrocony || measurement_type == ZygZag)
    {
        for(last_index = index; index < C_samples + last_index; index++)
        {
            signal.samples[index] = 0.0;
        }
        for(last_index = index; index < B_samples + last_index; index++)
        {
            signal.samples[index] = normalized_amplitude * -1.0;
        }
    }
    for(last_index = index; index < D_samples + last_index; index++)
    {
        signal.samples[index] = normalized_readout_amplitude;
    }

    //sanity check
    if(index == signal.count)
        qDebug() << "Spoko";

    return signal;
}

void Backend::sendToKeithley()
{

}

Backend& Backend::instance()
{
    static Backend _instance = Backend();
    return _instance;
}

Backend& BackendAccess::operator()()
{
    return Backend::instance();
}

void Backend::setupMeasurement()
{
    // Defualt hold inital voltage
    FDwfAnalogOutIdleSet(analog_handle, CHANNEL_BOTH, DwfAnalogOutIdleInitial);

    // Wait to 0
    FDwfAnalogOutWaitSet(analog_handle, CHANNEL_BOTH, 0.0);

    // Repat n times
    FDwfAnalogOutRepeatSet(analog_handle, CHANNEL_BOTH, repeat_times);

    // Custom function
    FDwfAnalogOutNodeEnableSet(analog_handle, CHANNEL_BOTH, AnalogOutNodeCarrier, true);
    FDwfAnalogOutNodeFunctionSet(analog_handle, CHANNEL_BOTH, AnalogOutNodeCarrier, funcCustom);

    // Frequency set to max
    FDwfAnalogOutFrequencySet(analog_handle, CHANNEL_BOTH, max_freq);

    // Set bias voltage to 0
    FDwfAnalogOutOffsetSet(analog_handle, CHANNEL_BOTH, 0.0);

    // Set amplitude voltage
    FDwfAnalogOutAmplitudeSet(analog_handle, CHANNEL_BOTH, amplitude);

    // Set signals
    Signal signal = generateSignal();

    // Set run time
    FDwfAnalogOutRunSet(analog_handle, CHANNEL_BOTH, (double)signal.count / max_freq );

    // Link and enable channels


    // Set parameters
    // Send setup to keithley
}
void Backend::runMeasurement()
{
    // Run analog signal
    // Update progressbar
    // Download results keythle
    // Save results
    // Display plot with results preview
}
void Backend::setSignal(int channel, Signal& signal)
{
    int samples_count_min, samples_count_max;
    FDwfAnalogOutNodeDataInfo(analog_handle, channel, AnalogOutNodeCarrier, &samples_count_min, &samples_count_max);
    // SAnity is lost RIP
    qDebug() << "Possible samples: " << samples_count_min << " === " << samples_count_max << "   actual: " << signal.count;
    FDwfAnalogOutNodeDataSet(analog_handle, channel, AnalogOutNodeCarrier, signal.samples, signal.count);
}

