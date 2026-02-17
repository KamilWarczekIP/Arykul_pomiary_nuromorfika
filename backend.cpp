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
    , A(10)
    , B(20)
    , C(10)
    , D(5)
    , E(0)
    , measurement_type(ZygZag)
    , amplitude(1000)
    , readout_amplitude(50)
    , repeat_times(10)
    , max_freq(10000.0)
    , min_freq(0.0)
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


void Backend::generatePreview()
{
    Signal sig = generateSignal();
    // for (size_t index = 0; index < sig.count; index++)
    // {
    //     if(sig.samples[index] == 1.0)
    //         sig.samples[index] = (double)amplitude / VOLTS_IN_MILIVOLT;
    //     else if(sig.samples[index] == -1.0)
    //         sig.samples[index] = -(double)amplitude / VOLTS_IN_MILIVOLT;
    //     else if(sig.samples[index] != 0.0)
    //         sig.samples[index] = -(double)readout_amplitude / VOLTS_IN_MILIVOLT;
    // }

    emit generatedPreview(sig);
}

Signal Backend::generateTriggerSignal()
{
    Signal signal;
    signal.count = 0;
    double A_sec = ((double)A / MICROSEC_IN_SEC);
    double B_sec = ((double)B / MICROSEC_IN_SEC);
    double C_sec = ((double)C / MICROSEC_IN_SEC);
    double D_sec = ((double)D / MICROSEC_IN_SEC);
    double E_sec = ((double)E / MICROSEC_IN_SEC);

    uint A_samples = A_sec * max_freq;
    uint B_samples = B_sec * max_freq;
    uint C_samples = C_sec * max_freq;
    uint D_samples = D_sec * max_freq;
    uint E_samples = E_sec * max_freq;

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
        signal.count += E_samples;
        break;
    }

    signal.samples = new double[signal.count];

    size_t offset_index = 0;
    if(measurement_type == Impulse)
        offset_index = A_samples * 2 + B_samples;
    else
        offset_index = A_samples * 2 + B_samples * 2 + C_samples;

    for(size_t index = 0; index < offset_index; index++)
    {
        signal.samples[index] = 0.0;
    }
    for(size_t index = 0; index < D_samples; index++)
    {
        signal.samples[index + offset_index] = 1.0;
    }
    // TODO trigger przesuniecie
    offset_index += D_samples;
    for(size_t index = 0; index < E_samples; index++)
    {
        signal.samples[index + offset_index] = 0.0;
    }
    return signal;
}

Signal Backend::generateSignal()
{
    double normalized_readout_amplitude = (double)readout_amplitude / amplitude;
    double normalized_amplitude = 1.0 * (measurement_type == ZygZag_Odwrocony ?  - 1.0 : 1.0);

    // Liczba sekund * częstotliwość = liczba sampli
    Signal signal;
    signal.count = 0;

    double A_sec = ((double)A / MICROSEC_IN_SEC);
    double B_sec = ((double)B / MICROSEC_IN_SEC);
    double C_sec = ((double)C / MICROSEC_IN_SEC);
    double D_sec = ((double)D / MICROSEC_IN_SEC);
    double E_sec = ((double)E / MICROSEC_IN_SEC);

    uint A_samples = A_sec * max_freq;
    uint B_samples = B_sec * max_freq;
    uint C_samples = C_sec * max_freq;
    uint D_samples = D_sec * max_freq;
    uint E_samples = E_sec * max_freq;

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
        signal.count += E_samples;
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
    for(last_index = index; index < E_samples + last_index; index++)
    {
        signal.samples[index] = 0.0;
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
double Backend::getMaxFrequency() const
{
    return max_freq;
}

double Backend::getSignalTimeInSeconds() const
{
    switch (measurement_type)
    {
    case Impulse:
        return (double)(A + B + A + D) / MICROSEC_IN_SEC;
    case ZygZag:
    case ZygZag_Odwrocony:
        return (double)(A + B + C + B + A + D) / MICROSEC_IN_SEC;
    }
}

void Backend::runMeasurement()
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
    FDwfAnalogOutAmplitudeSet(analog_handle, CHANNEL0, (double)amplitude / VOLTS_IN_MILIVOLT);
    FDwfAnalogOutAmplitudeSet(analog_handle, CHANNEL1, TRIGGER_AMPLITUDE);

    emit progress(5);
    // generate signals
    Signal signal = generateSignal();
    Signal trigger_signal = generateTriggerSignal();

    // Set run time
    FDwfAnalogOutRunSet(analog_handle, CHANNEL_BOTH, (double)signal.count / max_freq );

    // set signals and free memory
    setSignal(CHANNEL0, signal);
    setSignal(CHANNEL1, trigger_signal);

    delete[] signal.samples;
    delete[] trigger_signal.samples;

    // Send setup to keithley
    ViSession defaultRM = VI_NULL;  // Resource manager session
    ViSession instr = VI_NULL;      // Instrument session
    ViStatus status;
    ViChar *buffer = new ViChar[4096];
    ViUInt32 retCount;
    std::string visaAddress = "USB0::0x05E6::0x6500::04645729::INSTR";
    status = viOpenDefaultRM(&defaultRM);
    if (status != VI_SUCCESS) {
        qDebug() << "Failed to open VISA resource manager.";
        emit fail();
        return;
    }
    status = viOpen(defaultRM, const_cast<ViRsrc>(visaAddress.c_str()), VI_NULL, VI_NULL, &instr);
    if (status != VI_SUCCESS) {
        qDebug() << "Failed to open instrument session.";
        viClose(defaultRM);
        emit fail();
        return;
    }
    viSetAttribute(instr, VI_ATTR_TMO_VALUE, 5000);
    QString tspCommand = "MEASURE:VOLTAGE?";
    status = viWrite(instr, reinterpret_cast<ViConstBuf>(tspCommand.constData()), tspCommand.size(), &retCount);
    if (status != VI_SUCCESS) {
        qDebug() << "Failed to write command to instrument.";
        viClose(instr);
        viClose(defaultRM);
        emit fail();
        return;
    }

    // 2 micro sekundy minimum między triggerami
    /*
    reset()
    dmm.reset()
    dmm.digitize.func = dmm.FUNC_DC_CURRENT
    dmm.digitize.range = 0.000001
    dmm.digitize.samplerate = 100000
    dmm.measure.nplc = 0.0005

    display.clear()
    display.changescreen(display.SCREEN_USER_SWIPE)
    display.settext(display.TEXT1, "Pomiar trwa")
    display.settext(display.TEXT2, "Nie dotykej")

--     zmienic na liczba pomairow + 1
    defbuffer1.capacity = 100000
    defbuffer1.clear()
    trigger.clear()
    trigger.model.load("Empty")
    trigger.extin.edge = trigger.EDGE_RISING

    -- Trigger zewnetrzny
    trigger.model.setblock(1, trigger.BLOCK_WAIT, trigger.EVENT_EXTERNAL)
    trigger.model.setblock(2, trigger.BLOCK_MEASURE_DIGITIZE, defbuffer1, 1)
    trigger.model.setblock(3, trigger.BLOCK_BRANCH_COUNTER, $$$$$$$$$$, 1)
    -- Start triggera
    trigger.model.initiate()



format.data = format.REAL64
printbuffer(1, defbuffer1.n, defbuffer1, defbuffer1.timestamps)
    */

    // Run analog signal
    FDwfAnalogOutMasterSet(analog_handle, CHANNEL0, CHANNEL1);
    FDwfAnalogOutConfigure(analog_handle, CHANNEL_BOTH, true);
    emit progress(10);
    // Download results keythle
    status = viRead(instr, reinterpret_cast<ViBuf>(buffer), sizeof(buffer) - 1, &retCount);
    if (status == VI_SUCCESS || status == VI_SUCCESS_MAX_CNT) {
        buffer[retCount] = '\0';  // Null-terminate the string
        qDebug() << "Instrument response: " << buffer;
    } else {
        qDebug() << "Failed to read response from instrument.";
    }
    viClose(instr);
    viClose(defaultRM);
    // Save results
    emit progress(90);
    // Display plot with results preview
    emit progress(100);
}
void Backend::setSignal(int channel, Signal& signal)
{
    int samples_count_min, samples_count_max;
    FDwfAnalogOutNodeDataInfo(analog_handle, channel, AnalogOutNodeCarrier, &samples_count_min, &samples_count_max);
    // SAnity is lost RIP
    qDebug() << "Possible samples: " << samples_count_min << " === " << samples_count_max << "   actual: " << signal.count;
    FDwfAnalogOutNodeDataSet(analog_handle, channel, AnalogOutNodeCarrier, signal.samples, signal.count);
}

