/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class SpheringerAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                        public juce::Slider::Listener,
                                        public juce::MidiKeyboardState::Listener
{
public:
    SpheringerAudioProcessorEditor (SpheringerAudioProcessor&);
    ~SpheringerAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    // Override pure virtual functions
    // juce::MidiKeyboardState::Listener
    void handleNoteOn (juce::MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity) override;
    void handleNoteOff (juce::MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity) override;
    
    void sliderValueChanged(juce::Slider* slider) override;

private:
    // Create a button for file load
    juce::TextButton mLoadButton {"Load a sample library folder..."};
    
    // Create 4 rotary sliders for ADSR envelope customization
    // Create 4 labels for these sliders
    // can be declared all on the same line
    juce::Slider mAttackSlider, mDecaySlider, mSustainSlider, mReleaseSlider;
    juce::Label mAttackLabel, mDecayLabel, mSustainLabel, mReleaseLabel;
    
    // Create output volume slider
    juce::Slider mVolumeSlider;
    juce::Label mVolumeLabel;
    
    // Create MIDI keyboard visualization
    juce::MidiKeyboardState keyboardState;
    juce::MidiKeyboardComponent keyboardComponent;
    
    // GUI constants
    static const int MARGIN = 4, MAX_WINDOW_HEIGHT = 800, MAX_WINDOW_WIDTH = 1200 + 2 * MARGIN,
    MAX_KEYB_WIDTH = 1200, MAX_KEYB_HEIGHT = 82, BUTTON_WIDTH = 50, BUTTON_HEIGHT = 30;
    
    SpheringerAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpheringerAudioProcessorEditor)
};
