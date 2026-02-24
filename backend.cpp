#include "backend.hpp"
#include <QFile>
#include <QDir>
#include <qdebug.h>
#include <dwf.h>
#include <visa.h>

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
    , wait_time(0)
    , measurement_type(ZygZag)
    , amplitude(1000)
    , readout_amplitude(50)
    , repeat_times(10)
    , max_freq(1000000.0) // 10 MHz
    , min_freq(0.0)
    , defaultRM(VI_NULL)
    , keythley_handle(VI_NULL)
    , visa_address_keythley("USB0::0x05E6::0x6500::04645729::INSTR")
    , filename_suffix("")
    , file_location(QDir::currentPath())
{
    QObject::connect(this, &Backend::fail, this, &Backend::fail_cleanup);

    FDwfParamSet(DwfParamOnClose, 0);
    ViStatus status;
    status = viOpenDefaultRM(&defaultRM);
    if (status != VI_SUCCESS)
        qDebug() << "Problem z managerem zasobów VISA32";
}

Backend::~Backend()
{
    FDwfDeviceCloseAll();
    viClose(keythley_handle);
    viClose(defaultRM);
}


bool Backend::analogDiscoveryStatus()
{
    FDwfDeviceCloseAll();
    int number_of_devices;
    FDwfEnum(enumfilterDiscovery2, &number_of_devices);
    if(number_of_devices == 0)
        return false;
    FDwfDeviceOpen(number_of_devices - 1, &analog_handle);
    DWFERC rec;
    FDwfGetLastError(&rec);
    if(rec != dwfercNoErc)
        return false;
    FDwfDeviceCloseAll();
    return true;
}
bool Backend::keythleyStatus()
{
    ViStatus status;
    status = viOpen(defaultRM, const_cast<ViRsrc>(visa_address_keythley.c_str()), VI_NULL, VI_NULL, &keythley_handle);
    if (status != VI_SUCCESS)
        return false;

    viClose(keythley_handle);
    return true;

}

void Backend::analogDiscoveryfetchMaxSamples()
{
    FDwfDeviceCloseAll();
    FDwfDeviceOpen(-1, &analog_handle);
    int min;
    FDwfAnalogOutDataInfo(analog_handle, CHANNEL0, &min, &analog_discovery_max_samples);
    FDwfDeviceCloseAll();
}
void Backend::analogDiscoveryfetchMaxRepeat()
{
    FDwfDeviceCloseAll();
    FDwfDeviceOpen(-1, &analog_handle);
    int min;
    FDwfAnalogOutRepeatInfo(analog_handle, CHANNEL0, &min, &analog_discovery_max_repeat);
    FDwfDeviceCloseAll();

}
void Backend::analogDiscoveryfetchMaxWaitTime()
{
    FDwfDeviceCloseAll();
    FDwfDeviceOpen(-1, &analog_handle);
    double min;
    FDwfAnalogOutWaitInfo(analog_handle, CHANNEL0, &min, &analog_discovery_max_wait);
    FDwfDeviceCloseAll();
}


int Backend::signalSampleCount()
{
    int count = 0;
    double A_sec = ((double)A / MICROSEC_IN_SEC);
    double B_sec = ((double)B / MICROSEC_IN_SEC);
    double C_sec = ((double)C / MICROSEC_IN_SEC);
    double D_sec = ((double)D / MICROSEC_IN_SEC);
    double trigger_offset_sec = ((double)trigger_offset / MICROSEC_IN_SEC);

    uint A_samples = A_sec * GENERATION_FREQ;
    uint B_samples = B_sec * GENERATION_FREQ;
    uint C_samples = C_sec * GENERATION_FREQ;
    uint D_samples = D_sec * GENERATION_FREQ;
    uint trigger_offset_samples = trigger_offset_sec * GENERATION_FREQ;

    switch(measurement_type)
    {
    case ZygZag:
    case ZygZag_Odwrocony:
        count += C_samples;
        count += B_samples;
    case Impulse:
    case Impulse_odwrocony:
        count += D_samples;
    case Impulse_pomiar_scalony:
        count += A_samples;
        count += A_samples;
        count += B_samples;
        break;
    }
    return count;
}
int Backend::analogDiscoveryMaxSamples()
{
    return analog_discovery_max_samples;
}
int Backend::analogDiscoveryMaxRepeat()
{
    return analog_discovery_max_repeat;
}
double Backend::analogDiscoveryMaxWaitTime()
{
    return analog_discovery_max_wait;
}

void Backend::outputPreview()
{
    Signal sig = generateSignal();
    QString signal_string = "";
    for (size_t index = 0; index < sig.count; index++)
    {
        if(sig.samples[index] == 1.0)
            sig.samples[index] = (double)amplitude / VOLTS_IN_MILIVOLT;
        else if(sig.samples[index] == -1.0)
            sig.samples[index] = -(double)amplitude / VOLTS_IN_MILIVOLT;
        else if(sig.samples[index] != 0.0)
            sig.samples[index] = -(double)readout_amplitude / VOLTS_IN_MILIVOLT;
        signal_string += QString::number(sig.samples[index]) + ", ";
    }
    qDebug() << signal_string;
    qDebug() << "SAMPLES: " << sig.count;
    qDebug() << "WAIT TIME: " << wait_time;
    qDebug() << "ALL FREQ. CALC.: " << getCalculatedFrequencyOfExcitation();
    qDebug() << "FILE: " << file_location.append("/wyniki-%1-%2.csv").arg(filename_suffix, QDateTime::currentDateTime().toString(Qt::DateFormat::ISODate).replace(":", "-"));
}

Signal Backend::generateTriggerSignal()
{
    Signal signal;
    signal.count = 0;
    double A_sec = ((double)A / MICROSEC_IN_SEC);
    double B_sec = ((double)B / MICROSEC_IN_SEC);
    double C_sec = ((double)C / MICROSEC_IN_SEC);
    double D_sec = ((double)D / MICROSEC_IN_SEC);
    double trigger_offset_sec = ((double)trigger_offset / MICROSEC_IN_SEC);

    uint A_samples = A_sec * GENERATION_FREQ;
    uint B_samples = B_sec * GENERATION_FREQ;
    uint C_samples = C_sec * GENERATION_FREQ;
    uint D_samples = D_sec * GENERATION_FREQ;
    uint trigger_offset_samples = trigger_offset_sec * GENERATION_FREQ;

    switch(measurement_type)
    {
    case ZygZag:
    case ZygZag_Odwrocony:
        signal.count += C_samples;
        signal.count += B_samples;
    case Impulse:
    case Impulse_odwrocony:
        signal.count += D_samples;
    case Impulse_pomiar_scalony:
        signal.count += A_samples;
        signal.count += A_samples;
        signal.count += B_samples;
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

    //trigger przesuniecie - nadpisuje zerami
    for(size_t index = 0; index < trigger_offset_samples; index++)
    {
        signal.samples[index + offset_index] = 0.0;
    }
    return signal;
}

Signal Backend::generateSignal()
{
    double normalized_readout_amplitude = (double)readout_amplitude / amplitude;
    double normalized_amplitude = 1.0 * (measurement_type == ZygZag_Odwrocony || measurement_type == Impulse_odwrocony  ?  - 1.0 : 1.0);

    // Liczba sekund * częstotliwość = liczba sampli
    Signal signal;
    signal.count = 0;

    double A_sec = ((double)A / MICROSEC_IN_SEC);
    double B_sec = ((double)B / MICROSEC_IN_SEC);
    double C_sec = ((double)C / MICROSEC_IN_SEC);
    double D_sec = ((double)D / MICROSEC_IN_SEC);

    uint A_samples = A_sec * GENERATION_FREQ;
    uint B_samples = B_sec * GENERATION_FREQ;
    uint C_samples = C_sec * GENERATION_FREQ;
    uint D_samples = D_sec * GENERATION_FREQ;

    switch(measurement_type)
    {
    case ZygZag:
    case ZygZag_Odwrocony:
        signal.count += C_samples;
        signal.count += B_samples;
    case Impulse:
    case Impulse_odwrocony:
        signal.count += D_samples;
    case Impulse_pomiar_scalony:
        signal.count += A_samples;
        signal.count += A_samples;
        signal.count += B_samples;
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
    for(last_index = index; index < A_samples + last_index; index++)
    {
        signal.samples[index] = 0.0;
    }
    if(measurement_type != Impulse_pomiar_scalony)
        for(last_index = index; index < D_samples + last_index; index++)
        {
            signal.samples[index] = normalized_readout_amplitude;
        }

    //sanity check
    if(index == signal.count - 1)
        qDebug() << "Spoko";


    return signal;
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
    case Impulse_odwrocony:
        return (double)(A + B + A + D) / MICROSEC_IN_SEC;
    case Impulse_pomiar_scalony:
        return (double)(A + B + A) / MICROSEC_IN_SEC;
    case ZygZag:
    case ZygZag_Odwrocony:
        return (double)(A + B + C + B + A + D) / MICROSEC_IN_SEC;
    }
}
void Backend::fail_cleanup(QString message)
{
    qDebug() << "Problem: " << message;
    viClose(keythley_handle);
    FDwfDeviceClose(analog_handle);
}

void Backend::runMeasurement(double const frequency)
{
    FDwfParamSet(DwfParamOnClose, 0);
    FDwfDeviceOpen(-1, &analog_handle);
    DWFERC rec;
    FDwfGetLastError(&rec);
    if(rec != dwfercNoErc)
    {
        emit fail("Nie udało się połączyć z analogiem");
        return;
    }
    FDwfDeviceAutoConfigureSet(analog_handle, 0);
    FDwfDeviceReset(analog_handle);

    FDwfAnalogOutFrequencyInfo(analog_handle, CHANNEL0, &min_freq, &max_freq);

    // Defualt hold inital voltage
    FDwfAnalogOutIdleSet(analog_handle, CHANNEL_BOTH, DwfAnalogOutIdleOffset);

    // Set Node carrier
    FDwfAnalogOutNodeEnableSet(analog_handle, CHANNEL_BOTH, AnalogOutNodeCarrier, true);

    // Wait
    double calculated_wait_time = (double)wait_time / MICROSEC_IN_SEC;
    if(calculated_wait_time > analogDiscoveryMaxWaitTime())
    {
        emit fail("Czas oczekiwania pomiędzy wymuszeniami zbyt długi. Zmniejsz częstotliwość wymuszeń.");
        return;
    }
    FDwfAnalogOutWaitSet(analog_handle, CHANNEL_BOTH, calculated_wait_time);

    // Repat n times
    if(repeat_times > analogDiscoveryMaxRepeat())
    {
        emit fail("Liczba pomiarów zbyt duża.");
        return;
    }
    FDwfAnalogOutRepeatSet(analog_handle, CHANNEL_BOTH, repeat_times);

    // Custom function
    FDwfAnalogOutNodeFunctionSet(analog_handle, CHANNEL_BOTH, AnalogOutNodeCarrier, funcCustom);

    // Frequency set to generation frequency
    FDwfAnalogOutNodeFrequencySet(analog_handle, CHANNEL_BOTH, AnalogOutNodeCarrier, GENERATION_FREQ);

    // Set bias voltage to 0
    FDwfAnalogOutNodeOffsetSet(analog_handle, CHANNEL_BOTH, AnalogOutNodeCarrier, 0.0);

    // Set amplitude voltage
    FDwfAnalogOutNodeAmplitudeSet(analog_handle, CHANNEL0, AnalogOutNodeCarrier, (double)amplitude / VOLTS_IN_MILIVOLT);
    FDwfAnalogOutNodeAmplitudeSet(analog_handle, CHANNEL1, AnalogOutNodeCarrier, TRIGGER_AMPLITUDE);




    // generate signals
    emit progress(5);
    Signal signal = generateSignal();
    if(signal.count > analogDiscoveryMaxSamples())
    {
        emit fail("Sygnał wymuszenia jest zbyt długi");
        return;
    }


    emit progress(7);
    Signal trigger_signal = generateTriggerSignal();

    // Set run time
    FDwfAnalogOutRunSet(analog_handle, CHANNEL_BOTH, (double)signal.count / GENERATION_FREQ );

    // Set master/slave synchronization
    FDwfAnalogOutMasterSet(analog_handle, CHANNEL0, CHANNEL1);

    // set signals and remember to free memory
    setSignal(CHANNEL0, signal);
    setSignal(CHANNEL1, trigger_signal);


    char error_message_buffer[256];
    FDwfGetLastErrorMsg(error_message_buffer);
    emit fail("Analog Discovery error: " + QString::fromLocal8Bit(error_message_buffer));

    FDwfGetLastError(&rec);
    if(rec != dwfercNoErc)
    {
        char error_message_buffer[256];
        FDwfGetLastErrorMsg(error_message_buffer);
        emit fail("Analog Discovery error: " + QString::fromLocal8Bit(error_message_buffer));
        return;
    }


    // Send setup to keithley
    ViUInt32 retCount;
    ViStatus keythley_status;
    keythley_status = viOpen(defaultRM, const_cast<ViRsrc>(visa_address_keythley.c_str()), VI_NULL, VI_NULL, &keythley_handle);
    if (keythley_status != VI_SUCCESS)
    {
        emit fail("Nie udalo sie nawiazac polaczniea z keythleyem");
        return;
    }
    viSetAttribute(keythley_handle, VI_ATTR_TMO_VALUE, 5000);
    QString tsp_command = QString(
    // -- resetowanie i ustawienia początkowe
    "dmm.reset()"
    "dmm.digitize.func = dmm.FUNC_DIGITIZE_CURRENT\n"
    "dmm.digitize.range = 0.000001\n"
    "dmm.digitize.samplerate = 1000000\n"
    // "dmm.measure.nplc = 0.0005\n"

    //-- wyswietlanie informacji
    "display.clear()"
    "display.changescreen(display.SCREEN_USER_SWIPE)\n"
    "display.settext(display.TEXT1, \"Pomiar trwa\")\n"
    "display.settext(display.TEXT2, \"Nie dotykej\")\n"

    //--     zmienic na liczba pomairow + 1
    "defbuffer1.capacity = %1\n"
    "defbuffer1.clear()\n"
    "dmm.digitize.read()" //TODO -remove
    "trigger.clear()\n"
    "trigger.model.load(\"Empty\")\n"
    "trigger.extin.edge = trigger.EDGE_RISING\n"

    //-- Trigger zewnetrzny
    // 2 micro sekundy minimum między triggerami
    "trigger.model.setblock(1, trigger.BLOCK_WAIT, trigger.EVENT_EXTERNAL)\n"
    "trigger.model.setblock(2, trigger.BLOCK_MEASURE_DIGITIZE, defbuffer1, 1)\n"
    "trigger.model.setblock(3, trigger.BLOCK_BRANCH_COUNTER, %1, 1)\n"
    //-- Start triggera
    "trigger.model.initiate() \n"
    ).arg(repeat_times);

    keythley_status = viWrite(keythley_handle, reinterpret_cast<ViConstBuf>(tsp_command.toStdString().c_str()), tsp_command.size(), &retCount);
    if (keythley_status != VI_SUCCESS)
    {
        emit fail("Nie udalo sie przeslac skryptu na keythleya");
        return;
    }

    // Run analog signal
    FDwfAnalogOutConfigure(analog_handle, CHANNEL1, true);
    emit progress(10);


    // Download results keythley
    tsp_command = QString(
        "display.clear()"
        "format.data = format.REAL64\n"
        "printbuffer(1, defbuffer1.n, defbuffer1, defbuffer1.timestamps)\n"
    );



    char* buffer = new char[128 * repeat_times];
    keythley_status = viQueryf(keythley_handle, reinterpret_cast<ViConstString>(tsp_command.toStdString().c_str()), "%t", buffer);
    // keythley_status = viWrite(keythley_handle, reinterpret_cast<ViConstBuf>(tsp_command.c_str()), tsp_command.size(), &retCount);
    if (keythley_status != VI_SUCCESS)
    {
        emit fail("Nie udalo sie przeslac skryptu odczytywania na keythleya");
        return;
    }

    // unsigned char buffer[4096*5];
    // keythley_status = viRead(keythley_handle, buffer, 4096*5 -1, &retCount);
    // if (keythley_status < VI_SUCCESS)
    // {
    //     emit fail("Nie udało się odczytać odpowiedzi z Keythleya");
    //     viClose(keythley_handle);
    //     return;
    // }
    // buffer[retCount] = 0;  // Null-terminate the string
    qDebug() << "Instrument response: " << buffer;
    // QString ret = "";
    // for (int var = 0; var < retCount; ++var)
    // {
    //     ret.append(*(char*)(void*)(&buffer[var]));
    // }
    // qDebug() << ret;

    // Save results
    emit progress(90);
    QString likalizacja_pliku = file_location.append("/wyniki-%1-%2.csv").arg(filename_suffix, QDateTime::currentDateTime().toString(Qt::DateFormat::ISODate).replace(":", "-"));
    QFile plik_wyniki = QFile(likalizacja_pliku);
    if(!plik_wyniki.open(QIODevice::Text | QIODevice::Truncate | QIODevice::WriteOnly))
    {
        qDebug() << plik_wyniki.filesystemFileName();
        emit fail("Nie udało się otworzyć pliku: " + plik_wyniki.errorString());
        return;
    }

    QString measurement_parameters = QString("A [us], %1 \n").arg(A);
    measurement_parameters += QString("B [us], %1 \n").arg(B);
    measurement_parameters += QString("C [us], %1 \n").arg(C);
    measurement_parameters += QString("D [us], %1 \n").arg(D);
    measurement_parameters += QString("WAIT [us], %1 \n").arg(wait_time);
    measurement_parameters += QString("Impulse Type, %1 \n").arg(getMeasurementTypeName());
    measurement_parameters += QString("Amplitude [mV], %1 \n").arg(amplitude);
    measurement_parameters += QString("Readout amplitude [mV], %1 \n").arg(readout_amplitude);
    measurement_parameters += QString("Repat times, %1 \n").arg(repeat_times);
    measurement_parameters += QString("Frequency set, %1 \n").arg(frequency);
    measurement_parameters += QString("Frequency calculated, %1 \n").arg(getCalculatedFrequencyOfExcitation());
    plik_wyniki.write(measurement_parameters.toLocal8Bit());

    if(plik_wyniki.write(buffer) != retCount)
    {
        emit fail("Nie udało się zapisać całego pliku :< ");
    }


    plik_wyniki.close();
    delete[] buffer;
    delete[] signal.samples;
    delete[] trigger_signal.samples;


    // Display plot with results preview - close all connections
    emit progress(100);
    viClose(keythley_handle);
    FDwfDeviceClose(analog_handle);
}
QString Backend::getMeasurementTypeName()
{
    switch (measurement_type)
    {
    case Impulse:
        return "Impulse";
    case ZygZag:
        return "ZygZag";
    case ZygZag_Odwrocony:
        return "ZygZag_Odwrocony";
    case Impulse_odwrocony:
        return "Impulse_odwrocony";
    case Impulse_pomiar_scalony:
        return "Impulse_pomiar_scalony";
    }
}

double Backend::getCalculatedFrequencyOfExcitation()
{
    return 1.0 / ((double)wait_time / MICROSEC_IN_SEC + getSignalTimeInSeconds());
}

void Backend::setSignal(int channel, Signal& signal)
{
    int samples_count_min = 2137, samples_count_max = 420;
    FDwfAnalogOutNodeDataInfo(analog_handle, channel, AnalogOutNodeCarrier, &samples_count_min, &samples_count_max);
    // SAnity is lost RIP
    qDebug() << "Possible samples: " << samples_count_min << " === " << samples_count_max << "   actual: " << signal.count;
    FDwfAnalogOutNodeDataSet(analog_handle, channel, AnalogOutNodeCarrier, signal.samples, signal.count);    
}

