#ifndef QT_UTILS_HPP
#define QT_UTILS_HPP

#include <QWidget>

#include "chip8.hpp"

class QAudioFormat;
class QAudioSink;
class QBuffer;
class QByteArray;
class QTimer;

// Qt-based widget that handles display rendering, input processing, and audio output
class Chip8EmulatorWidget : public QWidget
{
    Q_OBJECT

public:
    // Creates the emulator widget with the specified CHIP-8 instance, cycle frequency, and window scale
    explicit Chip8EmulatorWidget(Chip8 &chip8,
                                 std::uint32_t cycle_frecuency,
                                 std::uint32_t window_scale,
                                 QWidget *parent = nullptr);

    ~Chip8EmulatorWidget();

protected:
    // Renders the CHIP-8 display to the Qt widget
    void paintEvent(QPaintEvent *event) override;
    // Processes key press events and maps them to CHIP-8 keys
    void keyPressEvent(QKeyEvent *event) override;
    // Processes key release events and maps them to CHIP-8 keys
    void keyReleaseEvent(QKeyEvent *event) override;

private slots:
    // Executes one CHIP-8 CPU cycle
    void execute_cycle();
    // Updates the delay and sound timers at 60Hz
    void update_timers();

private:
    Chip8 &chip8;
    std::uint32_t cycle_frecuency;
    std::uint32_t window_scale;

    QTimer *cpu_timer;
    QTimer *standard_timer;

    QAudioSink *audio_sink;
    QBuffer *audio_buffer;
    QByteArray audio_data;

    bool sound_playing;

    // Configures the widget display properties
    void setup_display();
    // Initializes CPU and timer update timers
    void setup_timers();
    // Sets up audio output with compatible format detection
    void setup_audio();

    // Starts audio playback
    void start_audio();
    // Stops audio playback and cleans up audio buffer
    void stop_audio();

    // Maps Qt key codes to CHIP-8 keypad values. Returns -1 for unmapped keys
    int map_qt_key_to_chip8(int qt_key);
};

// Generates a simple beep tone for the given audio format
QByteArray generate_beep(const QAudioFormat &format, const int duration_ms = 500);

#endif  // QT_UTILS_HPP