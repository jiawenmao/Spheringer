/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SpheringerAudioProcessorEditor::SpheringerAudioProcessorEditor (SpheringerAudioProcessor& p)
    : AudioProcessorEditor (&p),
    keyboardComponent(p.keyboardState,juce::MidiKeyboardComponent::horizontalKeyboard),
    audioProcessor (p)
{
    // Add load file button
    mLoadButton.onClick = [&]() { audioProcessor.loadFile(); };
    addAndMakeVisible(mLoadButton); // make button visible
    
    // Link audio processor to keyboard state Make MIDI keyboard visible
    p.keyboardState.addListener(this);
    addAndMakeVisible(keyboardComponent);

    
    // Add ADSR rotary sliders
    // Attack
    mAttackSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    mAttackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 20);
    mAttackSlider.setRange(0.01f, 5.0f, 0.01f); // min, max, increment
    mAttackSlider.setDoubleClickReturnValue(true, 0.1f); // default: 0.1s
    mAttackSlider.addListener(this);
    addAndMakeVisible(mAttackSlider); // make slider visible and add to child component of editor
    
    // Decay
    mDecaySlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    mDecaySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 20);
    mDecaySlider.setRange(0.01f, 5.0f, 0.01f);
    mDecaySlider.setDoubleClickReturnValue(true, 0.1f); // default: 0.1s
    mDecaySlider.addListener(this);
    addAndMakeVisible(mDecaySlider);
    
    // Sustain
    mSustainSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    mSustainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 20);
    mSustainSlider.setRange(0.01f, 5.0f, 0.01f);
    mSustainSlider.setDoubleClickReturnValue(true, 1.0f); // default: 1.0s
    mSustainSlider.addListener(this);
    addAndMakeVisible(mSustainSlider);
    
    // Release
    mReleaseSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    mReleaseSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 20);
    mReleaseSlider.setRange(0.01f, 5.0f, 0.01f);
    mReleaseSlider.setDoubleClickReturnValue(true, 0.1f); // default: 0.1s
    mReleaseSlider.addListener(this);
    addAndMakeVisible(mReleaseSlider);
    
    
    // Add volume slider
    mVolumeSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    mVolumeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 20);
    mVolumeSlider.setRange(-20.0f, 20.0f, 0.1f); // in dB
    mVolumeSlider.setDoubleClickReturnValue(true, 0.0f); // double click to return to detente/neutral position
    mVolumeSlider.addListener(this);
    addAndMakeVisible(mVolumeSlider);
    
    // on volume value change
    mVolumeSlider.onValueChange = [this]()
    {
        audioProcessor.volume.setTargetValue(mVolumeSlider.getValue());
    };
    
    ////////////// Font and UI =================================================================
    // Set UI window size
    setSize (600, 400);
    
    // Set label texts for the sliders
    const auto fontSize = 10.0f;
    
    // Attack
    mAttackLabel.setFont(fontSize); // set font size
    mAttackLabel.setText("Attack (s)", juce::NotificationType::dontSendNotification); // set text content
    mAttackLabel.setJustificationType(juce::Justification::centredTop); // set text alignment
    mAttackLabel.attachToComponent(&mAttackSlider, false);
    
    // Decay
    mDecayLabel.setFont(fontSize);
    mDecayLabel.setText("Decay (s)", juce::NotificationType::dontSendNotification);
    mDecayLabel.setJustificationType(juce::Justification::centredTop);
    mDecayLabel.attachToComponent(&mDecaySlider, false);
    
    // Sustain
    mSustainLabel.setFont(fontSize);
    mSustainLabel.setText("Sustain (s)", juce::NotificationType::dontSendNotification);
    mSustainLabel.setJustificationType(juce::Justification::centredTop);
    mSustainLabel.attachToComponent(&mSustainSlider, false);
    
    // Release
    mReleaseLabel.setFont(fontSize);
    mReleaseLabel.setText("Release (s)", juce::NotificationType::dontSendNotification);
    mReleaseLabel.setJustificationType(juce::Justification::centredTop);
    mReleaseLabel.attachToComponent(&mReleaseSlider, false);
    
    // Volume
    mVolumeLabel.setFont(fontSize);
    mVolumeLabel.setText("Volume (dB)", juce::NotificationType::dontSendNotification);
    mVolumeLabel.setJustificationType(juce::Justification::centredTop);
    mVolumeLabel.attachToComponent(&mVolumeSlider, false);
    

}

SpheringerAudioProcessorEditor::~SpheringerAudioProcessorEditor()
{
}

//==============================================================================
void SpheringerAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
}

void SpheringerAudioProcessorEditor::resized()
{
    // Set button size and position
    mLoadButton.setBounds(getWidth()/2 - 100, getHeight()/3 - 30, 200, 60);
    
    // Set MIDI keyboard size and position
    juce::Rectangle<int> r = getLocalBounds();
    float resizedKeybWidth = r.getWidth() - MARGIN * 2, resizedKeybHeight = r.getHeight() - 5;
    float keybWidth = resizedKeybWidth > MAX_KEYB_WIDTH ? MAX_KEYB_WIDTH : resizedKeybWidth;
    float keybHeight = resizedKeybHeight > MAX_KEYB_HEIGHT ? MAX_KEYB_HEIGHT : resizedKeybHeight;
    keyboardComponent.setBounds (MARGIN, MARGIN, keybWidth, keybHeight);
    
    
    // Set ADSR slider positions, values are relative
    // Current window size: 600 * 400
    const auto startX = 0.4f;
    const auto startY = 0.7f;
    const auto dialWidth = 0.15f;
    const auto dialHeight = 0.225f;
    
    mAttackSlider.setBoundsRelative(startX, startY, dialWidth, dialHeight); // proportional x,y, height, width
    mDecaySlider.setBoundsRelative(startX + dialWidth, startY, dialWidth, dialHeight);
    mSustainSlider.setBoundsRelative(startX + dialWidth*2, startY, dialWidth, dialHeight);
    mReleaseSlider.setBoundsRelative(startX + dialWidth*3 , startY, dialWidth, dialHeight);
    
    // Set volume slider position
    const auto startXX = 0.2f;
    mVolumeSlider.setBoundsRelative(startXX , startY, dialWidth, dialHeight);
}

void SpheringerAudioProcessorEditor::handleNoteOn(juce::MidiKeyboardState *source, int midiChannel, int midiNoteNumber, float velocity)
{
    // should I write the play sound from here???
    // no I don't think so - the midi messages are read into processBlock already
}
 
void SpheringerAudioProcessorEditor::handleNoteOff(juce::MidiKeyboardState *source, int midiChannel, int midiNoteNumber, float velocity)
{}

void SpheringerAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    /*
    if (slider == &mAttackSlider)
    {
        audioProcessor.getADSRParams().attack = mAttackSlider.getValue();
    }
    else if (slider == &mDecaySlider)
    {
        audioProcessor.getADSRParams().decay = mDecaySlider.getValue();
    }
    else if (slider == &mSustainSlider)
    {
        audioProcessor.getADSRParams().sustain = mSustainSlider.getValue();
    }
    else if (slider == &mReleaseSlider)
    {
        audioProcessor.getADSRParams().release = mReleaseSlider.getValue();
    }
    
    // Update ADSR upon user input
    audioProcessor.updateADSR();
    */
}
