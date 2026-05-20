/*
 ==============================================================================

 RNBO_JuceAudioProcessorEditor.h
 Chord Trigger UI — 24 chord buttons (Major / minor × 12 keys)

 ==============================================================================
*/

#ifndef RNBO_JUCEAUDIOPROCESSOREDITOR_H_INCLUDED
#define RNBO_JUCEAUDIOPROCESSOREDITOR_H_INCLUDED

#include "RNBO.h"
#include "RNBO_JuceAudioProcessor.h"
#include <array>
#include <unordered_map>

namespace RNBO {

    class RNBOAudioProcessorEditor
        : public juce::AudioProcessorEditor
        , public juce::AsyncUpdater
        , public RNBO::EventHandler
        , public juce::MessageListener
    {
    public:
        RNBOAudioProcessorEditor(JuceAudioProcessor* owner, CoreObject& rnboObject);
        ~RNBOAudioProcessorEditor() override;

        void paint(juce::Graphics&) override;
        void resized() override;

        void handleAsyncUpdate() override;
        void eventsAvailable() override;
        void handleParameterEvent(const RNBO::ParameterEvent& event) override;
        void handlePresetEvent(const RNBO::PresetEvent& event) override;

        // kept for API compatibility
        void chooseFileForDataRef(const juce::String) {}
        JuceAudioProcessor* owner() const { return _owner; }
        void handleMessage(const juce::Message&) override {}

    private:
        void updateButtonStates();
        void triggerChord(int paramIndex, bool on);

        JuceAudioProcessor*                        _owner;
        CoreObject&                                _rnboObject;
        ParameterEventInterfaceUniquePtr           _parameterInterface;

        // 24 chord toggle buttons (index matches RNBO parameter index)
        std::array<std::unique_ptr<juce::TextButton>, 24> _chordButtons;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RNBOAudioProcessorEditor)
    };

} // namespace RNBO

#endif // RNBO_JUCEAUDIOPROCESSOREDITOR_H_INCLUDED
