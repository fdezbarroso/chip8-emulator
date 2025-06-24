#include "qt_utils.hpp"

#include <QAudioDevice>
#include <QAudioSink>
#include <QBuffer>
#include <QKeyEvent>
#include <QMediaDevices>
#include <QPainter>
#include <QTimer>
#include <cmath>
#include <iostream>

#include "chip8_constants.hpp"
#include "emulator_utils.hpp"

Chip8EmulatorWidget::Chip8EmulatorWidget(Chip8 &chip8,
                                         const std::uint32_t cycle_frecuency,
                                         const std::uint32_t window_scale,
                                         QWidget *parent) :
    QWidget(parent),
    chip8(chip8),
    cycle_frecuency(cycle_frecuency),
    window_scale(window_scale),
    cpu_timer(nullptr),
    standard_timer(nullptr),
    audio_sink(nullptr),
    audio_buffer(nullptr),
    sound_playing(false)
{
    chip8.pc = START_ADDRESS;

    setup_display();
    setup_timers();
    setup_audio();

    setFocusPolicy(Qt::StrongFocus);

    std::cout << "CHIP-8 Emulator initialized!" << std::endl;
}

Chip8EmulatorWidget::~Chip8EmulatorWidget()
{
    if (cpu_timer)
    {
        cpu_timer->stop();
    }

    if (standard_timer)
    {
        standard_timer->stop();
    }

    stop_audio();

    delete audio_sink;
    delete audio_buffer;
}

void Chip8EmulatorWidget::setup_display()
{
    setFixedSize(WINDOW_WIDTH * window_scale, WINDOW_HEIGHT * window_scale);

    setWindowTitle("CHIP-8 Emulator");

    setStyleSheet("background-color: black;");
}

void Chip8EmulatorWidget::setup_timers()
{
    cpu_timer = new QTimer(this);
    cpu_timer->setInterval(1000 / cycle_frecuency);

    connect(cpu_timer, &QTimer::timeout, this, &Chip8EmulatorWidget::execute_cycle);

    standard_timer = new QTimer(this);
    standard_timer->setInterval(1000 / TIMER_FREQUENCY);

    connect(standard_timer, &QTimer::timeout, this, &Chip8EmulatorWidget::update_timers);

    cpu_timer->start();
    standard_timer->start();
}

// TODO: Simplify this
void Chip8EmulatorWidget::setup_audio()
{
    // Use the system's default output device
    QAudioDevice default_device = QMediaDevices::defaultAudioOutput();

    if (default_device.isNull())
    {
        std::cout << "No default audio device found - sound disabled" << std::endl;
        chip8.mute = true;
        return;
    }

    std::cout << "Using default audio device: " << default_device.description().toStdString() << std::endl;

    // Try many more formats
    QList<QAudioFormat> formats;

    // Common sample rates and formats for digital audio
    QList<int> sample_rates = {48000, 44100, 96000, 192000, 32000, 22050, 16000, 8000};
    QList<int> channel_counts = {2, 1, 6, 8};  // Stereo, Mono, 5.1, 7.1
    QList<QAudioFormat::SampleFormat> sample_formats = {
        QAudioFormat::Int16, QAudioFormat::Int32, QAudioFormat::Float, QAudioFormat::UInt8};

    // Generate all combinations
    for (int rate : sample_rates)
    {
        for (int channels : channel_counts)
        {
            for (QAudioFormat::SampleFormat format : sample_formats)
            {
                QAudioFormat audio_format;
                audio_format.setSampleRate(rate);
                audio_format.setChannelCount(channels);
                audio_format.setSampleFormat(format);
                formats.append(audio_format);
            }
        }
    }

    // Try formats on the default device
    for (const QAudioFormat &format : formats)
    {
        if (default_device.isFormatSupported(format))
        {
            audio_sink = new QAudioSink(default_device, format, this);
            audio_sink->setVolume(0.4);
            audio_data = generate_beep(format);

            std::cout << "Audio ready with format: " << format.sampleRate() << "Hz, " << format.channelCount()
                      << " channels, format " << static_cast<int>(format.sampleFormat()) << std::endl;
            return;
        }
    }

    // If default device doesn't work, fall back to trying all devices
    std::cout << "Default device doesn't support any tested formats, trying others..." << std::endl;

    for (const QAudioDevice &device : QMediaDevices::audioOutputs())
    {
        if (device.isNull() || device == default_device)
        {
            continue;  // Skip default (already tried)
        }


        std::cout << "Trying: " << device.description().toStdString() << std::endl;

        for (const QAudioFormat &format : formats)
        {
            if (device.isFormatSupported(format))
            {
                audio_sink = new QAudioSink(device, format, this);
                audio_sink->setVolume(0.4);
                audio_data = generate_beep(format);

                std::cout << "Audio ready: " << device.description().toStdString() << " with " << format.sampleRate()
                          << "Hz" << std::endl;
                return;
            }
        }
    }

    std::cout << "No compatible audio device found - sound disabled" << std::endl;
    chip8.mute = true;
}

void Chip8EmulatorWidget::start_audio()
{
    if (!audio_sink || audio_data.isEmpty())
    {
        return;
    }

    stop_audio();

    audio_buffer = new QBuffer(&audio_data, this);
    audio_buffer->open(QIODevice::ReadOnly);

    audio_sink->start(audio_buffer);

    sound_playing = true;
}

void Chip8EmulatorWidget::stop_audio()
{
    if (audio_sink && sound_playing)
    {
        audio_sink->stop();
        sound_playing = false;
    }

    if (audio_buffer)
    {
        audio_buffer->close();
        delete audio_buffer;
        audio_buffer = nullptr;
    }
}

void Chip8EmulatorWidget::execute_cycle()
{
    std::uint16_t opcode = chip8.memory.at(chip8.pc) << 8 | chip8.memory.at(chip8.pc + 1);
    chip8.pc += 2;

    if (!execute(chip8, opcode))
    {
        std::cerr << "Fatal error, execution aborted." << std::endl;
        close();
        return;
    }

    if (chip8.render)
    {
        update();
        chip8.render = false;
    }
}

void Chip8EmulatorWidget::update_timers()
{
    if (chip8.delay_timer > 0)
    {
        chip8.delay_timer--;
    }

    if (chip8.sound_timer > 0)
    {
        if (!chip8.mute && !sound_playing)
        {
            start_audio();
        }
        chip8.sound_timer--;
    }
    else if (sound_playing)
    {
        stop_audio();
    }
}

void Chip8EmulatorWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.fillRect(rect(), Qt::black);

    for (std::uint32_t x = 0; x < WINDOW_WIDTH; x++)
    {
        for (std::uint32_t y = 0; y < WINDOW_HEIGHT; y++)
        {
            if (chip8.display.at(x + y * WINDOW_WIDTH) != 0)
            {
                QRect pixel(x * window_scale, y * window_scale, window_scale, window_scale);
                painter.fillRect(pixel, Qt::white);
            }
        }
    }
}

void Chip8EmulatorWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat())
    {
        return;
    }

    int chip8_key = map_qt_key_to_chip8(event->key());
    if (chip8_key != -1)
    {
        chip8.keys.at(chip8_key) = 0x1;
        event->accept();
    }
}

void Chip8EmulatorWidget::keyReleaseEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat())
    {
        return;
    }

    int chip8_key = map_qt_key_to_chip8(event->key());
    if (chip8_key != -1)
    {
        chip8.keys.at(chip8_key) = 0x0;
        event->accept();
    }
}

int Chip8EmulatorWidget::map_qt_key_to_chip8(const int qt_key)
{
    switch (qt_key)
    {
        case Qt::Key_1:
            return 0x1;
        case Qt::Key_2:
            return 0x2;
        case Qt::Key_3:
            return 0x3;
        case Qt::Key_4:
            return 0xC;
        case Qt::Key_Q:
            return 0x4;
        case Qt::Key_W:
            return 0x5;
        case Qt::Key_E:
            return 0x6;
        case Qt::Key_R:
            return 0xD;
        case Qt::Key_A:
            return 0x7;
        case Qt::Key_S:
            return 0x8;
        case Qt::Key_D:
            return 0x9;
        case Qt::Key_F:
            return 0xE;
        case Qt::Key_Z:
            return 0xA;
        case Qt::Key_X:
            return 0x0;
        case Qt::Key_C:
            return 0xB;
        case Qt::Key_V:
            return 0xF;
        default:
            return -1;
    }
}

// TODO: Double check this
QByteArray generate_beep(const QAudioFormat &format, const int duration_ms)
{
    qint64 num_samples = (format.sampleRate() * duration_ms) / 1000;
    QByteArray buffer;
    const qreal freq = 2.0 * M_PI * 440.0 / format.sampleRate();  // 440Hz

    if (format.sampleFormat() == QAudioFormat::Float)
    {
        buffer.resize(num_samples * format.channelCount() * sizeof(float));
        float *data = reinterpret_cast<float *>(buffer.data());

        for (qint64 i = 0; i < num_samples; ++i)
        {
            float value = static_cast<float>(std::sin(i * freq) * 0.3);
            for (int ch = 0; ch < format.channelCount(); ++ch)
            {
                data[i * format.channelCount() + ch] = value;
            }
        }
    }
    else if (format.sampleFormat() == QAudioFormat::Int32)
    {
        buffer.resize(num_samples * format.channelCount() * sizeof(qint32));
        qint32 *data = reinterpret_cast<qint32 *>(buffer.data());

        for (qint64 i = 0; i < num_samples; ++i)
        {
            qint32 value = static_cast<qint32>(std::sin(i * freq) * 200000000);  // 32-bit range
            for (int ch = 0; ch < format.channelCount(); ++ch)
            {
                data[i * format.channelCount() + ch] = value;
            }
        }
    }
    else if (format.sampleFormat() == QAudioFormat::Int16)
    {
        buffer.resize(num_samples * format.channelCount() * sizeof(qint16));
        qint16 *data = reinterpret_cast<qint16 *>(buffer.data());

        for (qint64 i = 0; i < num_samples; ++i)
        {
            qint16 value = static_cast<qint16>(std::sin(i * freq) * 8000);
            for (int ch = 0; ch < format.channelCount(); ++ch)
            {
                data[i * format.channelCount() + ch] = value;
            }
        }
    }
    else  // UInt8
    {
        buffer.resize(num_samples * format.channelCount() * sizeof(quint8));
        quint8 *data = reinterpret_cast<quint8 *>(buffer.data());

        for (qint64 i = 0; i < num_samples; ++i)
        {
            quint8 value = static_cast<quint8>((std::sin(i * freq) + 1.0) * 96);
            for (int ch = 0; ch < format.channelCount(); ++ch)
            {
                data[i * format.channelCount() + ch] = value;
            }
        }
    }

    return buffer;
}
