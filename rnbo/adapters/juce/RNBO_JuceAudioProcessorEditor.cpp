/*
 ==============================================================================

 RNBO_JuceAudioProcessorEditor.cpp
 Chord Trigger UI — 24 chord buttons (Major / minor × 12 keys)

 ==============================================================================
*/

#include "RNBO_JuceAudioProcessorEditor.h"

using namespace juce;

namespace RNBO {

// ---------------------------------------------------------------------------
// Layout constants
// ---------------------------------------------------------------------------
static constexpr int kWinW        = 440;
static constexpr int kWinH        = 640;
static constexpr int kPadX        = 20;
static constexpr int kTitleH      = 54;
static constexpr int kHeaderH     = 28;
static constexpr int kRowH        = 44;
static constexpr int kKeyLabelW   = 52;
static constexpr int kBtnGap      = 8;

// colours
static const Colour kBgColour      { 0xff1a1a2e };
static const Colour kPanelBg       { 0xff16213e };
static const Colour kMajOnColour   { 0xffe8a44a };  // warm amber
static const Colour kMajOffColour  { 0xff3a3050 };
static const Colour kMinOnColour   { 0xff5a9fd4 };  // cool blue
static const Colour kMinOffColour  { 0xff1e2d3d };
static const Colour kTextOn        { 0xff1a1a2e };
static const Colour kTextOff       { 0xffaaaacc };
static const Colour kKeyLabelCol   { 0xff888899 };
static const Colour kHeaderCol     { 0xffccccee };

// ---------------------------------------------------------------------------
// Chord metadata (parallel to RNBO parameter order 0..23)
// ---------------------------------------------------------------------------
struct ChordInfo {
    const char* label;   // button label
    const char* key;     // key name for row label
    bool        isMajor;
};

static const std::array<ChordInfo, 24> kChords = {{
    { "C",   "C",  true  },   //  0 CM
    { "Cm",  "C",  false },   //  1 Cm
    { "D\u266d", "D\u266d", true  },   //  2 DfM  (Db)
    { "D\u266dm","D\u266d", false },   //  3 Dfm
    { "D",   "D",  true  },   //  4 DM
    { "Dm",  "D",  false },   //  5 Dm
    { "E\u266d", "E\u266d", true  },   //  6 EfM  (Eb)
    { "E\u266dm","E\u266d", false },   //  7 Efm
    { "E",   "E",  true  },   //  8 EM
    { "Em",  "E",  false },   //  9 Em
    { "F",   "F",  true  },   // 10 FM
    { "Fm",  "F",  false },   // 11 Fm
    { "G\u266d", "G\u266d", true  },   // 12 GfM  (Gb)
    { "G\u266dm","G\u266d", false },   // 13 Gfm
    { "G",   "G",  true  },   // 14 GM
    { "Gm",  "G",  false },   // 15 Gm
    { "A\u266d", "A\u266d", true  },   // 16 AfM  (Ab)
    { "A\u266dm","A\u266d", false },   // 17 Afm
    { "A",   "A",  true  },   // 18 AM
    { "Am",  "A",  false },   // 19 Am
    { "B\u266d", "B\u266d", true  },   // 20 BfM  (Bb)
    { "B\u266dm","B\u266d", false },   // 21 Bfm
    { "B",   "B",  true  },   // 22 BM
    { "Bm",  "B",  false },   // 23 Bm
}};

// ---------------------------------------------------------------------------
// Custom LookAndFeel for chord buttons
// ---------------------------------------------------------------------------
class ChordButtonLAF : public LookAndFeel_V4
{
public:
    void drawButtonBackground(Graphics& g,
                              Button& btn,
                              const Colour& /*bgColour*/,
                              bool isMouseOver,
                              bool isButtonDown) override
    {
        const bool   isMaj  = btn.getProperties()["isMajor"];
        const bool   isOn   = btn.getToggleState();

        Colour fill;
        if (isOn)
            fill = isMaj ? kMajOnColour : kMinOnColour;
        else
            fill = isMaj ? kMajOffColour : kMinOffColour;

        if (isMouseOver || isButtonDown)
            fill = fill.brighter(0.15f);

        const float corner = 7.0f;
        const auto  bounds = btn.getLocalBounds().toFloat().reduced(1.5f);

        g.setColour(fill);
        g.fillRoundedRectangle(bounds, corner);

        if (isOn) {
            g.setColour(fill.brighter(0.35f));
            g.drawRoundedRectangle(bounds, corner, 1.5f);
        }
    }

    void drawButtonText(Graphics& g, TextButton& btn,
                        bool /*isMouseOver*/, bool /*isButtonDown*/) override
    {
        const bool isOn = btn.getToggleState();
        g.setColour(isOn ? kTextOn : kTextOff);
        g.setFont(Font(14.5f, Font::bold));
        g.drawFittedText(btn.getButtonText(),
                         btn.getLocalBounds(),
                         Justification::centred, 1);
    }
};

// ---------------------------------------------------------------------------
// Editor
// ---------------------------------------------------------------------------

// Shared LookAndFeel instance (must outlive all buttons)
static ChordButtonLAF sChordLAF;

RNBOAudioProcessorEditor::RNBOAudioProcessorEditor(JuceAudioProcessor* const p,
                                                   CoreObject& rnboObject)
    : AudioProcessorEditor(p)
    , _owner(p)
    , _rnboObject(rnboObject)
    , _parameterInterface(_rnboObject.createParameterInterface(
          ParameterEventInterface::SingleProducer, this))
{
    jassert(p != nullptr);
    setOpaque(true);

    for (int i = 0; i < 24; ++i)
    {
        auto btn = std::make_unique<TextButton>(kChords[i].label);
        btn->setClickingTogglesState(true);
        btn->setToggleState(false, dontSendNotification);
        btn->setLookAndFeel(&sChordLAF);
        btn->getProperties().set("isMajor", kChords[i].isMajor);

        btn->onClick = [this, i]
        {
            const bool on = _chordButtons[i]->getToggleState();
            triggerChord(i, on);
        };

        addAndMakeVisible(*btn);
        _chordButtons[i] = std::move(btn);
    }

    setSize(kWinW, kWinH);
}

RNBOAudioProcessorEditor::~RNBOAudioProcessorEditor()
{
    for (auto& btn : _chordButtons)
        if (btn) btn->setLookAndFeel(nullptr);
}

// ---------------------------------------------------------------------------
void RNBOAudioProcessorEditor::triggerChord(int paramIndex, bool on)
{
    auto* param = _owner->getParameters()[paramIndex];
    if (param)
    {
        param->beginChangeGesture();
        param->setValueNotifyingHost(on ? 1.0f : 0.0f);
        param->endChangeGesture();
    }
}

// ---------------------------------------------------------------------------
void RNBOAudioProcessorEditor::paint(Graphics& g)
{
    g.fillAll(kBgColour);

    // title bar
    g.setColour(kPanelBg);
    g.fillRect(0, 0, kWinW, kTitleH);

    g.setColour(Colour(0xffe8a44a));
    g.setFont(Font(22.0f, Font::bold));
    g.drawText("CHORDER", 0, 0, kWinW, kTitleH, Justification::centred);

    // column headers
    const int headerY     = kTitleH + 4;
    const int contentW    = kWinW - kPadX * 2;
    const int btnAreaW    = contentW - kKeyLabelW - kBtnGap;
    const int btnW        = (btnAreaW - kBtnGap) / 2;
    const int majX        = kPadX + kKeyLabelW + kBtnGap;
    const int minX        = majX + btnW + kBtnGap;

    g.setFont(Font(11.5f, Font::bold));

    g.setColour(kMajOnColour);
    g.drawText("MAJOR", majX, headerY, btnW, kHeaderH, Justification::centred);

    g.setColour(kMinOnColour);
    g.drawText("minor", minX, headerY, btnW, kHeaderH, Justification::centred);

    // key labels (one per two rows — major & minor share the same key)
    g.setFont(Font(13.5f, Font::bold));
    g.setColour(kKeyLabelCol);

    for (int row = 0; row < 12; ++row)
    {
        const int paramIdx = row * 2;           // major param index
        const int rowY     = kTitleH + kHeaderH + kBtnGap + row * kRowH + kPadX / 2;
        g.drawText(kChords[paramIdx].key,
                   kPadX, rowY, kKeyLabelW, kRowH,
                   Justification::centredLeft);
    }
}

// ---------------------------------------------------------------------------
void RNBOAudioProcessorEditor::resized()
{
    const int contentW = kWinW - kPadX * 2;
    const int btnAreaW = contentW - kKeyLabelW - kBtnGap;
    const int btnW     = (btnAreaW - kBtnGap) / 2;
    const int majX     = kPadX + kKeyLabelW + kBtnGap;
    const int minX     = majX + btnW + kBtnGap;
    const int startY   = kTitleH + kHeaderH + kBtnGap + kPadX / 2;
    const int btnH     = kRowH - 6;

    for (int row = 0; row < 12; ++row)
    {
        const int y      = startY + row * kRowH + 3;
        const int majIdx = row * 2;
        const int minIdx = row * 2 + 1;

        _chordButtons[majIdx]->setBounds(majX, y, btnW, btnH);
        _chordButtons[minIdx]->setBounds(minX, y, btnW, btnH);
    }
}

// ---------------------------------------------------------------------------
void RNBOAudioProcessorEditor::updateButtonStates()
{
    for (int i = 0; i < 24; ++i)
    {
        auto* param = _owner->getParameters()[i];
        if (param && _chordButtons[i])
        {
            const bool on = param->getValue() > 0.5f;
            _chordButtons[i]->setToggleState(on, dontSendNotification);
        }
    }
    repaint();
}

// ---------------------------------------------------------------------------
void RNBOAudioProcessorEditor::handleAsyncUpdate()
{
    drainEvents();
    updateButtonStates();
}

void RNBOAudioProcessorEditor::eventsAvailable()
{
    triggerAsyncUpdate();
}

void RNBOAudioProcessorEditor::handleParameterEvent(const RNBO::ParameterEvent&)
{
    updateButtonStates();
}

void RNBOAudioProcessorEditor::handlePresetEvent(const RNBO::PresetEvent& event)
{
    if (event.getType() == RNBO::PresetEvent::Touched)
        updateButtonStates();
}

} // namespace RNBO
