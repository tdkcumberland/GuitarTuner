/*
  ==============================================================================

    NoteUtils.cpp
    Created: 20 May 2026 9:18:39pm
    Author:  Timothy

  ==============================================================================
*/

#include "NoteUtils.h"
#include <cmath>

static const char* kNoteNames[] = {
    "C", "C#", "D", "D#", "E", "F",
    "F#", "G", "G#", "A", "A#", "B"
};

float NoteUtils::midiToHz(int midiNote)
{
    return kA4Hz * std::pow(2.0f, (midiNote - kA4Midi) / 12.0f);
}

NoteResult NoteUtils::getNoteResult(float freqHz)
{
    NoteResult result;
    result.noteName = "---";
    result.centsOff = 0.0f;
    result.targetHz = 0.0f;

    if (freqHz <= 0.0f) return result;

    // Convert freq to fractional MIDI note
    float fractionalMidi = kA4Midi + 12.0f * std::log2(freqHz / kA4Hz);
    int   nearestMidi = (int)std::round(fractionalMidi);

    // Clamp to valid MIDI range
    nearestMidi = juce::jlimit(0, 127, nearestMidi);

    int   octave = (nearestMidi / 12) - 1;
    int   noteIndex = nearestMidi % 12;

    result.noteName = juce::String(kNoteNames[noteIndex]) + juce::String(octave);
    result.targetHz = midiToHz(nearestMidi);
    result.centsOff = 100.0f * (fractionalMidi - nearestMidi);

    return result;
}
