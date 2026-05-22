#pragma once

/*
  ==============================================================================
   GuitarTuner - MainComponent Tuning Reference
  ==============================================================================

   AUDIO / PITCH DETECTION
   -----------------------
   kSilenceThreshold  (MainComponent.h)  default: 0.03f
       RMS level below which audio is ignored. Raise to gate more aggressively.

   kHoldBlocks        (MainComponent.h)  default: 30
       How many accumulation cycles to freeze the last good reading during decay.
       At 2048 samples / 48000 Hz ~= 42ms per cycle, so 30 ~= 1.25 seconds hold.

   kDetectionSize     (MainComponent.h)  default: 2048
       YIN analysis window in samples. Larger = more stable, slower response.
       Minimum for E2 (82 Hz) at 48kHz = ~1170 samples (2x period).

   PitchDetector::threshold  (PitchDetector.h)  default: 0.15f
       YIN confidence threshold. Lower = stricter, more -1 (no pitch) returns.
       Range: 0.05 (very strict) to 0.20 (permissive).

   Guitar range gate  (MainComponent.cpp)  default: 60–1400 Hz
       Pitches outside this range are discarded as YIN hallucinations.
       E2 = 82 Hz, E4 = 330 Hz, E5 = 659 Hz. 1400 covers all harmonics.

   setup.sampleRate   (MainComponent.cpp)  default: 48000.0
   setup.bufferSize   (MainComponent.cpp)  default: 512

   UI / FEEL
   ---------
   kSmoothing         (MainComponent.h)  default: 0.15f
       Needle smoothing factor per timer tick (exponential moving average).
       0.05 = very sluggish, 0.30 = snappy, 1.0 = no smoothing.

   startTimerHz       (MainComponent.cpp)  default: 60
       UI repaint rate in Hz. 60 is smooth; lower to reduce CPU.

   In-tune threshold  (paint())  default: ±5.0f cents
       Cents window for green needle/note. Tighten to ±3 for stricter tuning.

   Arc radius         (paint())  default: 0.38f * width
   Arc centre Y       (paint())  default: 0.72f * height

   PRESETS / DATA
   --------------
   Presets file:  %APPDATA%\GuitarTuner\presets.json
       JSON array of TuningPreset objects. Delete to reset to standard tuning.

   Standard tuning (index 0) is always protected from deletion.
   Standard notes: E2 A2 D3 G3 B3 E4 (MIDI 40 45 50 55 59 64)

  ==============================================================================
*/

#include <JuceHeader.h>
#include "PitchDetector.h"
#include "NoteUtils.h"
#include "TuningPreset.h"
#include "PresetEditorDialog.h"

class MainComponent : public juce::AudioAppComponent,
    private juce::Timer
{
public:
    MainComponent();
    ~MainComponent() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setActivePreset(int index);
    void setActiveString(int index);

private:
    void timerCallback() override;
    void refreshPresetSelector();
    void updateStringButtons();
    void updateDetectionSize();

    double currentSampleRate = 0.0;
    int    currentBlockSize = 0;

    // Accumulation buffer for pitch detection
    static constexpr int kDetectionSizeLow = 4096; // strings 1-3 (E2, A2, D3)
    static constexpr int kDetectionSizeHigh = 2048; // strings 4-6 (G3, B3, E4)
    int currentDetectionSize = kDetectionSizeLow;
    juce::AudioBuffer<float> inputBuffer;
    std::vector<float>       accumBuffer;
    int                      accumWritePos = 0;

    PitchDetector            pitchDetector;
    std::atomic<float>       detectedPitch{ -1.0f };

    std::atomic<float> detectedCents{ 0.0f };
    juce::String       detectedNote{ "---" };
    juce::CriticalSection noteLock;

    float smoothedCents = 0.0f;
    static constexpr float kSmoothing = 0.15f; // 0.0 = no smoothing, 1.0 = frozen

    PresetManager        presetManager;
    int                  activePresetIndex = 0;
    int                  activeStringIndex = 0; // which string we're currently tuning

    juce::ComboBox presetSelector;
    std::array<juce::TextButton, 6> stringButtons;
    juce::TextButton addPresetButton;

    std::atomic<float> inputLevel{ 0.0f };
    static constexpr float kSilenceThreshold = 0.03f; // more aggressive
    float heldCents = 0.0f;
    juce::String heldNote = "---";
    float heldPitch = -1.0f;
    int   holdCountdown = 0;
    static constexpr int kHoldBlocks = 30; // hold for ~30 accumulation cycles

    juce::TextButton                          editPresetButton;
    juce::Component::SafePointer<juce::DialogWindow> editorWindow;

    juce::TextButton deletePresetButton;

    int accumFilled = 0;
    std::vector<float> linearBuf;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};