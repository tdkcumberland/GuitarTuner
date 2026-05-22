/*
  ==============================================================================

    PresetEditorDialog.cpp
    Created: 20 May 2026 10:34:12pm
    Author:  Timothy

  ==============================================================================
*/

#include "PresetEditorDialog.h"

const juce::StringArray PresetEditorDialog::kNoteOptions = {
    "E1","F1","F#1","G1","G#1","A1","A#1","B1",
    "C2","C#2","D2","D#2","E2","F2","F#2","G2","G#2","A2","A#2","B2",
    "C3","C#3","D3","D#3","E3","F3","F#3","G3","G#3","A3","A#3","B3",
    "C4","C#4","D4","D#4","E4","F4","F#4","G4","G#4","A4","A#4","B4"
};

PresetEditorDialog::PresetEditorDialog(const TuningPreset& presetToEdit)
    : editingPreset(presetToEdit)
{
    setSize(400, 380);

    // Name editor
    addAndMakeVisible(nameEditor);
    nameEditor.setText(editingPreset.name);
    nameEditor.setJustification(juce::Justification::centredLeft);

    // String selectors
    for (int i = 0; i < 6; ++i)
    {
        addAndMakeVisible(stringSelectors[i]);
        for (int n = 0; n < kNoteOptions.size(); ++n)
            stringSelectors[i].addItem(kNoteOptions[n], n + 1);

        // Select current note
        juce::String currentNote = (i < (int)editingPreset.strings.size())
            ? editingPreset.strings[i].noteName
            : "E2";
        int idx = kNoteOptions.indexOf(currentNote);
        stringSelectors[i].setSelectedItemIndex(idx >= 0 ? idx : 0);
    }

    // Buttons
    addAndMakeVisible(saveButton);
    addAndMakeVisible(cancelButton);

    saveButton.onClick = [this] { saveAndClose(); };
    cancelButton.onClick = [this] { if (onCancel) onCancel(); };
}

void PresetEditorDialog::saveAndClose()
{
    editingPreset.name = nameEditor.getText();
    editingPreset.strings.clear();

    for (int i = 0; i < 6; ++i)
    {
        juce::String noteName = kNoteOptions[stringSelectors[i].getSelectedItemIndex()];

        // Find MIDI note number
        int midi = 28; // E1 default
        for (int m = 0; m < 128; ++m)
        {
            NoteResult nr = NoteUtils::getNoteResult(NoteUtils::midiToHz(m));
            if (nr.noteName == noteName)
            {
                midi = m;
                break;
            }
        }

        StringTuning st;
        st.noteName = noteName;
        st.targetHz = NoteUtils::midiToHz(midi);
        editingPreset.strings.push_back(st);
    }

    if (onSave) onSave(editingPreset);
}

void PresetEditorDialog::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff2b2b2b));
    g.setColour(juce::Colours::white);
    g.setFont(16.0f);
    g.drawText("Preset Name", 20, 15, 200, 24, juce::Justification::centredLeft);

    for (int i = 0; i < 6; ++i)
        g.drawText("String " + juce::String(i + 1),
            20, 65 + i * 44, 80, 24,
            juce::Justification::centredLeft);
}

void PresetEditorDialog::resized()
{
    nameEditor.setBounds(20, 40, 360, 30);

    for (int i = 0; i < 6; ++i)
        stringSelectors[i].setBounds(110, 65 + i * 44, 270, 30);

    saveButton.setBounds(20, 330, 170, 34);
    cancelButton.setBounds(210, 330, 170, 34);
}
