/*
  ==============================================================================

    TuningPreset.cpp
    Created: 20 May 2026 9:36:10pm
    Author:  Timothy

  ==============================================================================
*/

#include "TuningPreset.h"

// --- TuningPreset serialization ---

juce::var TuningPreset::toVar() const
{
    juce::DynamicObject* obj = new juce::DynamicObject();
    obj->setProperty("name", name);

    juce::Array<juce::var> stringsArray;
    for (auto& s : strings)
    {
        juce::DynamicObject* sObj = new juce::DynamicObject();
        sObj->setProperty("note", s.noteName);
        sObj->setProperty("hz", s.targetHz);
        stringsArray.add(juce::var(sObj));
    }
    obj->setProperty("strings", stringsArray);
    return juce::var(obj);
}

TuningPreset TuningPreset::fromVar(const juce::var& v)
{
    TuningPreset preset;
    preset.name = v["name"].toString();

    auto* arr = v["strings"].getArray();
    if (arr != nullptr)
    {
        for (auto& s : *arr)
        {
            StringTuning st;
            st.noteName = s["note"].toString();
            st.targetHz = (float)s["hz"];
            preset.strings.push_back(st);
        }
    }
    return preset;
}

// --- PresetManager ---

PresetManager::PresetManager()
{
    // Always ensure standard tuning exists as first preset
    loadFromDisk();
    if (presets.empty())
    {
        presets.push_back(standardTuning());
        saveToDisk();
    }
}

TuningPreset PresetManager::standardTuning()
{
    TuningPreset p;
    p.name = "Standard (EADGBe)";
    for (auto& noteName : { "E2", "A2", "D3", "G3", "B3", "E4" })
    {
        StringTuning st;
        st.noteName = noteName;
        st.targetHz = NoteUtils::midiToHz(
            [&]() -> int {
                // Map note name back to MIDI — simple lookup
                struct { const char* n; int midi; } table[] = {
                    {"E2",40},{"A2",45},{"D3",50},
                    {"G3",55},{"B3",59},{"E4",64}
                };
                for (auto& e : table)
                    if (noteName == e.n) return e.midi;
                return 60;
            }()
                );
        p.strings.push_back(st);
    }
    return p;
}

juce::File PresetManager::getPresetsFile() const
{
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("GuitarTuner")
        .getChildFile("presets.json");
}

void PresetManager::loadFromDisk()
{
    auto file = getPresetsFile();
    if (!file.existsAsFile()) return;

    auto json = juce::JSON::parse(file.loadFileAsString());
    auto* arr = json.getArray();
    if (arr == nullptr) return;

    presets.clear();
    for (auto& v : *arr)
        presets.push_back(TuningPreset::fromVar(v));
}

void PresetManager::saveToDisk() const
{
    auto file = getPresetsFile();
    file.getParentDirectory().createDirectory();

    juce::Array<juce::var> arr;
    for (auto& p : presets)
        arr.add(p.toVar());

    file.replaceWithText(juce::JSON::toString(juce::var(arr)));
}

void PresetManager::addPreset(const TuningPreset& preset)
{
    presets.push_back(preset);
    saveToDisk();
}

void PresetManager::deletePreset(int index)
{
    if (index > 0 && index < (int)presets.size()) // protect index 0 (standard)
    {
        presets.erase(presets.begin() + index);
        saveToDisk();
    }
}

const TuningPreset& PresetManager::getPreset(int index) const
{
    return presets[index];
}

void PresetManager::replacePreset(int index, const TuningPreset& preset)
{
    if (index > 0 && index < (int)presets.size())
    {
        presets[index] = preset;
        saveToDisk();
    }
}

int PresetManager::getNumPresets() const
{
    return (int)presets.size();
}
