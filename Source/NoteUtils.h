/*
  ==============================================================================

    NoteUtils.h
    Created: 20 May 2026 9:18:39pm
    Author:  Timothy

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

struct NoteResult
{
    juce::String noteName;   // e.g. "E2"
    float        centsOff;   // negative = flat, positive = sharp
    float        targetHz;   // exact frequency of the nearest note
};

class NoteUtils
{
public:
    // Convert Hz to nearest note + cents deviation
    static NoteResult getNoteResult(float freqHz);

    // Convert MIDI note number to Hz
    static float midiToHz(int midiNote);

    // Standard tuning target frequencies for reference
    static constexpr int kA4Midi = 69;
    static constexpr float kA4Hz = 440.0f;
};