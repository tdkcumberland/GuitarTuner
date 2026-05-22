/*
  ==============================================================================

    PresetEditorDialog.h
    Created: 20 May 2026 10:34:12pm
    Author:  Timothy

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "TuningPreset.h"
#include "NoteUtils.h"

class PresetEditorDialog : public juce::Component
{
public:
    // onSave called with the edited preset, onCancel called on dismiss
    std::function<void(TuningPreset)> onSave;
    std::function<void()>             onCancel;

    PresetEditorDialog(const TuningPreset& presetToEdit);

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void buildNoteList();
    void saveAndClose();

    TuningPreset                         editingPreset;
    juce::TextEditor                     nameEditor;
    std::array<juce::ComboBox, 6>        stringSelectors;
    juce::TextButton                     saveButton{ "Save" };
    juce::TextButton                     cancelButton{ "Cancel" };

    static const juce::StringArray       kNoteOptions;
};
