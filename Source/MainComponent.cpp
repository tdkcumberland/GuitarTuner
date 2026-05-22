#include "MainComponent.h"

MainComponent::MainComponent()
{
    setSize(800, 600);

    auto setup = deviceManager.getAudioDeviceSetup();
    setup.sampleRate = 48000.0;
    setup.bufferSize = 512;
    deviceManager.setAudioDeviceSetup(setup, true);

    setAudioChannels(1, 0);
    auto currentSetup = deviceManager.getAudioDeviceSetup();
    startTimerHz(60);

    // Preset selector
    addAndMakeVisible(presetSelector);
    presetSelector.onChange = [this]
        {
            setActivePreset(presetSelector.getSelectedItemIndex());
            updateStringButtons();
        };

    // Add preset button
    addAndMakeVisible(addPresetButton);
    addPresetButton.setButtonText("+ New Preset");
    addPresetButton.onClick = [this]
        {
            // Placeholder — will wire up editor later
            TuningPreset p = PresetManager::standardTuning();
            p.name = "Custom " + juce::String(presetManager.getNumPresets());
            presetManager.addPreset(p);
            refreshPresetSelector();
        };

    // Edit preset button
    addAndMakeVisible(editPresetButton);
    editPresetButton.setButtonText("Edit");
    editPresetButton.onClick = [this]
        {
            auto& current = presetManager.getPreset(activePresetIndex);
            auto* editor = new PresetEditorDialog(current);

            editor->onSave = [this](TuningPreset saved)
                {
                    if (activePresetIndex == 0)
                    {
                        saved.name = "Custom " + juce::String(presetManager.getNumPresets());
                        presetManager.addPreset(saved);
                        activePresetIndex = presetManager.getNumPresets() - 1;
                    }
                    else
                        presetManager.replacePreset(activePresetIndex, saved);

                    refreshPresetSelector();
                    presetSelector.setSelectedItemIndex(activePresetIndex);

                    if (editorWindow != nullptr)
                        editorWindow->closeButtonPressed();
                };

            editor->onCancel = [this]
                {
                    if (editorWindow != nullptr)
                        editorWindow->closeButtonPressed();
                };

            auto* dialogContent = editor;
            juce::DialogWindow::LaunchOptions options;
            options.content.setOwned(dialogContent);
            options.dialogTitle = "Edit Preset";
            options.dialogBackgroundColour = juce::Colour(0xff2b2b2b);
            options.escapeKeyTriggersCloseButton = true;
            options.useNativeTitleBar = false;
            options.resizable = false;
            editorWindow = options.launchAsync();
        };

    addAndMakeVisible(deletePresetButton);
    deletePresetButton.setButtonText("Delete");
    deletePresetButton.onClick = [this]
        {
            // Protect standard tuning
            if (activePresetIndex == 0)
            {
                juce::AlertWindow::showMessageBoxAsync(
                    juce::AlertWindow::WarningIcon,
                    "Cannot Delete",
                    "The Standard tuning preset cannot be deleted.");
                return;
            }

            juce::AlertWindow::showOkCancelBox(
                juce::AlertWindow::WarningIcon,
                "Delete Preset",
                "Delete \"" + presetManager.getPreset(activePresetIndex).name + "\"?",
                "Delete", "Cancel", nullptr,
                juce::ModalCallbackFunction::create([this](int result)
                    {
                        if (result == 1)
                        {
                            presetManager.deletePreset(activePresetIndex);
                            activePresetIndex = 0;
                            refreshPresetSelector();
                            presetSelector.setSelectedItemIndex(0);
                        }
                    })
            );
        };

    // String buttons
    auto& preset = presetManager.getPreset(activePresetIndex);
    for (int i = 0; i < 6; ++i)
    {
        addAndMakeVisible(stringButtons[i]);
        stringButtons[i].setButtonText(preset.strings[i].noteName
            + "\nString " + juce::String(i + 1));
        stringButtons[i].setClickingTogglesState(false);
        stringButtons[i].onClick = [this, i]
            {
                setActiveString(i);
                updateStringButtons();
            };
    }

    refreshPresetSelector();
    updateStringButtons();
}

MainComponent::~MainComponent()
{
    stopTimer();
    shutdownAudio();
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlockExpected;
    inputBuffer.setSize(1, samplesPerBlockExpected);
    accumBuffer.assign(kDetectionSizeLow, 0.0f);
    linearBuf.assign(kDetectionSizeLow, 0.0f);
    accumWritePos = 0;
    pitchDetector.prepare(sampleRate);

    DBG("Sample rate: " + juce::String(sampleRate));
    DBG("Block size: " + juce::String(samplesPerBlockExpected));
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    const float* inData = bufferToFill.buffer->getReadPointer(0, bufferToFill.startSample);
    int          numSamples = bufferToFill.numSamples;

    // RMS level check
    float sum = 0.0f;
    for (int i = 0; i < numSamples; ++i)
        sum += inData[i] * inData[i];
    float rms = std::sqrt(sum / numSamples);
    inputLevel.store(rms);

    if (rms > kSilenceThreshold)
    {
        // Slide samples into accumBuffer
        for (int i = 0; i < numSamples; ++i)
        {
            accumBuffer[accumWritePos] = inData[i];
            accumWritePos = (accumWritePos + 1) % currentDetectionSize;
        }

        accumFilled = juce::jmin(accumFilled + numSamples, currentDetectionSize);

        // Run YIN as soon as buffer has enough data, every block thereafter
        if (accumFilled >= currentDetectionSize)
        {
            // Reconstruct linear buffer from circular
            for (int i = 0; i < currentDetectionSize; ++i)
                linearBuf[i] = accumBuffer[(accumWritePos + i) % currentDetectionSize];

            float pitch = pitchDetector.detectPitch(linearBuf.data(), currentDetectionSize);

            if (pitch > 60.0f && pitch < 1400.0f)
            {
                NoteResult nr = NoteUtils::getNoteResult(pitch);
                detectedPitch.store(pitch);
                detectedCents.store(nr.centsOff);
                {
                    const juce::ScopedLock sl(noteLock);
                    detectedNote = nr.noteName;
                }
                heldPitch = pitch;
                heldCents = nr.centsOff;
                heldNote = nr.noteName;
                holdCountdown = kHoldBlocks;
            }
        }
    }
    else
    {
        if (holdCountdown > 0)
        {
            --holdCountdown;
            detectedPitch.store(heldPitch);
            detectedCents.store(heldCents);
            {
                const juce::ScopedLock sl(noteLock);
                detectedNote = heldNote;
            }
        }
        else
        {
            detectedPitch.store(-1.0f);
            accumFilled = 0;
            accumWritePos = 0;
        }
    }

    bufferToFill.clearActiveBufferRegion();
}

void MainComponent::releaseResources()
{
    inputBuffer.setSize(0, 0);
}

void MainComponent::timerCallback()
{
    float target = detectedCents.load();
    smoothedCents += kSmoothing * (target - smoothedCents);
    repaint();
}

void MainComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.fillAll(juce::Colour(0xff111111));

    float pitch = detectedPitch.load();
    float cents = smoothedCents;

    bool  active = pitch > 0.0f;
    float clampedCents = juce::jlimit(-50.0f, 50.0f, cents);

    // --- Target note resolution ---
    auto& preset = presetManager.getPreset(activePresetIndex);
    juce::String targetNote = preset.strings[activeStringIndex].noteName;
    juce::String note;
    {
        const juce::ScopedLock sl(noteLock);
        note = detectedNote;
    }
    bool onCorrectNote = active && (note == targetNote);
    bool inTune = onCorrectNote && std::abs(cents) <= 5.0f;

    // --- Arc geometry ---
    // Inset cx/cy to sit between the button columns (80px each side + 10 margin)
    float leftInset = 100.0f;
    float rightInset = 100.0f;
    float usableWidth = bounds.getWidth() - leftInset - rightInset;
    float cx = leftInset + usableWidth * 0.5f;
    float cy = bounds.getBottom() * 0.72f;
    float radius = usableWidth * 0.46f;

    // --- Arc track (recessed look) ---
    juce::Path arc;
    arc.addCentredArc(cx, cy, radius, radius, 0.0f,
        -juce::MathConstants<float>::pi * 0.35f,
        juce::MathConstants<float>::pi * 0.35f,
        true);
    g.setColour(juce::Colour(0xff1e1e1e));
    g.strokePath(arc, juce::PathStrokeType(18.0f));
    g.setColour(juce::Colour(0xff2a2a2a));
    g.strokePath(arc, juce::PathStrokeType(1.5f));

    // --- Tick marks + labels ---
    struct Tick { int step; const char* label; };
    Tick ticks[] = { {-5,"-50"},{-3,"-25"},{0,"0"},{3,"+25"},{5,"+50"} };

    for (auto& tick : ticks)
    {
        float t = tick.step / 5.0f;
        float angle = t * juce::MathConstants<float>::pi * 0.35f;
        float sinA = std::sin(angle);
        float cosA = -std::cos(angle);
        bool  isCenter = (tick.step == 0);

        float tickLen = isCenter ? 18.0f : 10.0f;
        float r1 = radius - tickLen;

        g.setColour(isCenter ? juce::Colour(0xff555555) : juce::Colour(0xff3a3a3a));
        g.drawLine(cx + sinA * r1, cy + cosA * r1,
            cx + sinA * radius, cy + cosA * radius, 1.5f);

        // Label just outside arc
        float labelR = radius + 14.0f;
        float lx = cx + sinA * labelR;
        float ly = cy + cosA * labelR;
        g.setColour(juce::Colour(0xff333333));
        g.setFont(9.0f);
        g.drawText(tick.label,
            juce::Rectangle<float>(lx - 14.0f, ly - 7.0f, 28.0f, 14.0f),
            juce::Justification::centred, false);
    }

    // --- Needle ---
    if (active)
    {
        float t = clampedCents / 50.0f;
        float angle = t * juce::MathConstants<float>::pi * 0.35f;
        float sinA = std::sin(angle);
        float cosA = -std::cos(angle);

        juce::Colour needleColour = inTune ? juce::Colour(0xff6dbf6d)
            : juce::Colour(0xffe06030);
        g.setColour(needleColour);
        g.drawLine(cx, cy,
            cx + sinA * (radius - 12.0f),
            cy + cosA * (radius - 12.0f),
            2.5f);

        // Pivot
        g.fillEllipse(cx - 6.0f, cy - 6.0f, 12.0f, 12.0f);
        g.setColour(juce::Colour(0xff111111));
        g.fillEllipse(cx - 3.0f, cy - 3.0f, 6.0f, 6.0f);
    }

    // --- Cents text ---
    float centsY = cy + 28.0f;
    g.setFont(14.0f);
    g.setColour(active ? juce::Colour(0xff666666) : juce::Colour(0xff2a2a2a));
    g.drawText(active ? juce::String(detectedCents.load(), 1) + " cents" : "---",
        juce::Rectangle<float>(cx - 80.0f, centsY, 160.0f, 24.0f),
        juce::Justification::centred, false);

    // --- Flat / Sharp labels ---
    float arcBaseY = cy + 18.0f;
    g.setFont(10.0f);
    g.setColour(juce::Colour(0xff2a2a2a));
    g.drawText("< flat", juce::Rectangle<float>(leftInset, arcBaseY, 60.0f, 16.0f),
        juce::Justification::centredLeft, false);
    g.drawText("sharp >", juce::Rectangle<float>(cx + usableWidth * 0.5f - 60.0f, arcBaseY, 60.0f, 16.0f),
        juce::Justification::centredRight, false);
}

void MainComponent::setActivePreset(int index)
{
    activePresetIndex = juce::jlimit(0, presetManager.getNumPresets() - 1, index);
}

void MainComponent::setActiveString(int index)
{
    activeStringIndex = juce::jlimit(0, 5, index);
    updateDetectionSize();
}

void MainComponent::refreshPresetSelector()
{
    presetSelector.clear();
    for (int i = 0; i < presetManager.getNumPresets(); ++i)
        presetSelector.addItem(presetManager.getPreset(i).name, i + 1);
    presetSelector.setSelectedItemIndex(activePresetIndex);
}

void MainComponent::updateStringButtons()
{
    auto& preset = presetManager.getPreset(activePresetIndex);
    for (int i = 0; i < 6; ++i)
    {
        bool active = (i == activeStringIndex);
        stringButtons[i].setButtonText(preset.strings[i].noteName
            + "\nString " + juce::String(i + 1));
        stringButtons[i].setColour(juce::TextButton::buttonColourId,
            active ? juce::Colour(0xff1e2a1e)
            : juce::Colour(0xff1e1e1e));
        stringButtons[i].setColour(juce::TextButton::textColourOffId,
            active ? juce::Colour(0xff6dbf6d)
            : juce::Colour(0xff555555));
    }
}

void MainComponent::updateDetectionSize()
{
    currentDetectionSize = (activeStringIndex < 3)
        ? kDetectionSizeLow
        : kDetectionSizeHigh;

    const juce::ScopedLock sl(noteLock);
    accumBuffer.assign(currentDetectionSize, 0.0f);
    linearBuf.assign(currentDetectionSize, 0.0f);
    accumWritePos = 0;
    accumFilled = 0;
}

void MainComponent::resized()
{
    auto area = getLocalBounds().reduced(10);
    auto topBar = area.removeFromTop(40);

    presetSelector.setBounds(topBar.removeFromLeft(220));
    topBar.removeFromLeft(6);
    editPresetButton.setBounds(topBar.removeFromLeft(52));
    topBar.removeFromLeft(6);
    addPresetButton.setBounds(topBar.removeFromLeft(90));
    topBar.removeFromLeft(6);
    deletePresetButton.setBounds(topBar.removeFromLeft(60));

    // String buttons — 3 left, 3 right
    auto leftCol = area.removeFromLeft(80);
    auto rightCol = area.removeFromRight(80);

    int buttonHeight = area.getHeight() / 3;

    for (int i = 2; i >= 0; i--)
        stringButtons[i].setBounds(leftCol.removeFromTop(buttonHeight).reduced(0, 4));

    for (int i = 3; i < 6; ++i)
        stringButtons[i].setBounds(rightCol.removeFromTop(buttonHeight).reduced(0, 4));
}