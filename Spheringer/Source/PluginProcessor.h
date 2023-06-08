/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class SpheringerAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    SpheringerAudioProcessor();
    ~SpheringerAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    // Load file ===================================================================
    void loadFile();
    
    // Playback State =====================================================================
    //bool playState = false;
    int currentPos = 0; // current playhead position
    int currentNum = -1; // current MIDI number, for storage
    int prevNum = 1000; // previously triggered MIDI number
    bool isPlaying = false;
    bool isStopping = false;
    
    // Volume value
    juce::SmoothedValue<float> volume {0.0f};
    
    //std::unique_ptr<juce::AudioBuffer<float>> tempBuffer; // shorter tha 3 seconds
    juce::AudioSampleBuffer tempBuffer;
    
    
    // UI ==========================================================================
    juce::MidiKeyboardState keyboardState;

private:
    //==============================================================================
    juce::AudioFormatManager mFormatManager;
    juce::AudioFormatReader* mFormatReader {nullptr};
    
    //std::unique_ptr<juce::AudioFormatReaderSource> playSource;
    //juce::AudioTransportSource transportSource;


    
    // Buffer for storing pre-loaded files after reader input
    // Map MIDI number (int) to audio files in the buffer
    //std::map<int, juce::AudioBuffer<float>> audioBuffers;
    std::map<int, juce::AudioSampleBuffer> audioBuffers;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpheringerAudioProcessor)
};
