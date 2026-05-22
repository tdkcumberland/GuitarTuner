/*
  ==============================================================================

    TuningPreset.h
    Created: 20 May 2026 9:36:10pm
    Author:  Timothy

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "NoteUtils.h"

struct StringTuning
{
    juce::String noteName;  // e.g. "E2"
    float        targetHz;  // computed from noteName
};

struct TuningPreset
{
    juce::String                  name;
    std::vector<StringTuning>     strings;  // index 0 = string 1 (lowest)

    // Serialize to JUCE var (JSON)
    juce::var toVar() const;

    // Deserialize from JUCE var
    static TuningPreset fromVar(const juce::var& v);
};

class PresetManager
{
public:
    PresetManager();

    void                         loadFromDisk();
    void                         saveToDisk() const;

    void                         addPreset(const TuningPreset& preset);
    void                         deletePreset(int index);
    const TuningPreset& getPreset(int index) const;
    int                          getNumPresets() const;

    // Returns the built-in standard tuning
    static TuningPreset          standardTuning();
    void replacePreset(int index, const TuningPreset& preset);

private:
    std::vector<TuningPreset>    presets;
    juce::File                   getPresetsFile() const;
};
