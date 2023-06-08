/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SpheringerAudioProcessor::SpheringerAudioProcessor()
     : AudioProcessor (BusesProperties()
                       .withOutput ("Output1", juce::AudioChannelSet::quadraphonic(), true))
                       
{
    // allows plugin to use basic audio formats, e.g. .mp3, .wav, ...
    mFormatManager.registerBasicFormats();
    // Initialize MIDI keyboard state
    keyboardState.reset();

}

SpheringerAudioProcessor::~SpheringerAudioProcessor()
{
    mFormatReader = nullptr;
}

//==============================================================================
const juce::String SpheringerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SpheringerAudioProcessor::acceptsMidi() const
{
    return true;
}

bool SpheringerAudioProcessor::producesMidi() const
{
    return false;
}

bool SpheringerAudioProcessor::isMidiEffect() const
{
    return false;
}

double SpheringerAudioProcessor::getTailLengthSeconds() const
{
    return 0.3;
}

int SpheringerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SpheringerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SpheringerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SpheringerAudioProcessor::getProgramName (int index)
{
    return {};
}

void SpheringerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SpheringerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    
    // Reset volume value
    volume.reset(sampleRate, 0.02f); // ramp length in seconds: 0.02
    
    // Print host output channel number
    std::cout << "Host output channel count is: " << getChannelCountOfBus(false, 0) << std::endl;
}

void SpheringerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    //transportSource.releaseResources();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SpheringerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::quadraphonic())
     //&& layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    else
        return true;

}
#endif

void SpheringerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    // Read MIDI message for keyboard visualization
    keyboardState.processNextMidiBuffer (midiMessages, 0, buffer.getNumSamples(), true);

    // Create an allocated quadData array to store buffer info and do DSP (gain volume)
    // Check if output and buffer channel number match
    if (buffer.getNumChannels() != totalNumOutputChannels)
    {
        DBG("Buffer and Output channel number don't match?!?! OMG");
        std::cout << "Buffer and Output channel number don't match?!?! OMG" << std::endl;
    }
    else
    {
        
        int numChannels = buffer.getNumChannels();
        int numSamples = buffer.getNumSamples();
        //std::cout << "Buffer channel#: " << numChannels << ", Buffer sample#: " << numSamples << std::endl;
        
        /*
        //juce::AudioBuffer<float> tempQuadBuffer(quadData, numChannels, numSamples);
        juce::AudioBuffer<float> tempQuadBuffer(numChannels, numSamples);
        // let the buffer do the parsing automatically
        mSampler.renderNextBlock(tempQuadBuffer, midiMessages, 0, buffer.getNumSamples());
    
       // float* quadData = new float[numChannels * numSamples];
        
        for (int channel = 0; channel < numChannels; ++channel)
        {
            buffer.copyFrom(channel, 0, tempQuadBuffer, channel, 0, numSamples);
        }
         */
        
        // Parse MIDI message here
        for (const auto metadata : midiMessages)
        {
            // Read Midi message objects from MidiBuffer
            const auto message = metadata.getMessage();
            
            // The transport source method does not care about Midi off events - as soon as the Midi on is triggered it play the audio file as a whole to its end. Kinda works with the current samples I have as they are not too long.
            // Def will not working with looping samples, like the ambience singing.
            
            if (message.isNoteOn())
            {
                isPlaying = true;
                // play file with the same midi number
                // std::map<int, juce::AudioSampleBuffer> audioBuffers;
                currentNum = message.getNoteNumber();
                
            }
            
            if (message.isNoteOff() && isStopping) // let's do one note at a time
            {
                isPlaying = false;
                currentPos = 0;
                prevNum = 1000;
            }
                
        }
        

        
        // Playback from loaded audio buffer storage
        if (isPlaying)
        {
            // File loader
            const auto iterator = audioBuffers.find(currentNum); // this iterator exists only within the loop
            // make sure the iterator find a key in the mapping folder matching the input MIDI number
            // if no match: the return value is std::map::end
            if (iterator == audioBuffers.end()) // no match
            {
                isPlaying = false;
            }
                
            
            if (iterator != audioBuffers.end() && iterator->first != prevNum)
            {
                
                prevNum = currentNum;
                std::cout << "MIDI number triggered is: " << iterator->first << std::endl;
                // ->first returns the key (the first column)
                // ->second return the data mapped to the key
                // create a pointer? or AudioSampleBuffer? to hold the data
                tempBuffer.clear();
                tempBuffer = iterator->second; // only reload when change in MIDI number
                
            }
            
            
            int tempSamples = tempBuffer.getNumSamples();
            
            std::cout << "tempSamples = " << tempSamples << ", currentPos = " << currentPos << std::endl;
        
            for (int channel = 0; channel < numChannels; ++channel)
            {
                auto* channelData = buffer.getWritePointer (channel);
                auto* tempData = tempBuffer.getReadPointer (channel);

                for (int sample = 0; sample < numSamples; ++sample)
                {
                    if (currentPos + sample < tempSamples)
                        channelData[sample] = tempData[sample + currentPos];
                    else
                    {
                        channelData[sample] = 0;
                        isStopping = true; // flag that file is played to end
                    }
                    
                    // Adjust output volume in dB
                    channelData[sample] *= juce::Decibels::decibelsToGain(volume.getNextValue());
                        
                }
                
            }
            
            // reset to new playhead position
            currentPos += numSamples;
        }
        

    
    }
    
    // Clear MidiBuffer as the plugin does not have MIDI output
    midiMessages.clear();

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    
    // let's add in a volume slider!
    /*
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        // ..do something to the data...
    }
    */
    
    /*
    // tempBuffer checked! everything is correctly loaded - 0504 checkpoint
    // check if the audio buffer is correctly loaded into processBlock
    for (int channel = 0; channel < tempBuffer.getNumChannels(); ++channel)
    {
        float* channelData = tempBuffer.getWritePointer(channel);
        for (int sample = 0; sample < tempBuffer.getNumSamples(); ++sample)
        {
            std::cout << "Channel#: " << channel << ", Sample#: " << sample << ", Data: " << channelData[sample] << std::endl;
        }
    }
    */

}

//==============================================================================
bool SpheringerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SpheringerAudioProcessor::createEditor()
{
    return new SpheringerAudioProcessorEditor (*this);
}

//==============================================================================
void SpheringerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SpheringerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}


//==============================================================================
void SpheringerAudioProcessor::loadFile()
{
    /*
     The loadFile() method
     On button click, the plugin prompts the user to choose a folder and pre-load all audio files in that folder into pre-allocated memory for callback.
     */
    
    // Pre-load a folder of sample audio files
    juce::FileChooser chooser {"Please load a library folder...", juce::File::getSpecialLocation(juce::File::userDesktopDirectory), "*"};
    // Print out the loaded folder path on screen
    //juce::FileChooser chooser {"Please load a file"};
    //chooser = std::make_unique<juce::FileChooser> ("Please load a file", juce::File{}, "*.wav");
    
    // Returns True if user choose directory; read directory via .getResult() method
    if (chooser.browseForDirectory())
    {
        juce::File folder = chooser.getResult(); // get and store file?
        std::cout << folder.getFullPathName() << std::endl;
        
        juce::Array<juce::File> audioFiles; // pre-load files allocate to this array
        folder.findChildFiles(audioFiles, juce::File::TypesOfFileToFind::findFiles, true, "*.wav"); // search for .wav files
        
        // Initialize a reader to use for loading all files
        std::unique_ptr <juce::AudioFormatReader> mFormatReader;
        
        
        for (auto file : audioFiles)
        {
            std::cout << "File loaded pre-reader! Filename: " << file.getFileName() << std::endl;
            mFormatReader.reset(mFormatManager.createReaderFor(file));
            
            if (mFormatReader != nullptr)
            {
                // Print number of channels
                int numChannels = mFormatReader->numChannels;
                int numSamples = static_cast<int>(mFormatReader->lengthInSamples); //int64 to int32
                std::cout << "File loaded! Filename: " << file.getFileName() << ", Number of channels: " << numChannels << std::endl;
                
                // Create a buffer for file
                //juce::AudioBuffer<float> fileBuffer(numChannels, numSamples);
                juce::AudioSampleBuffer fileBuffer(numChannels, numSamples);
                //auto fileBuffer = std::make_unique<juce::AudioBuffer<float>>(numChannels, numSamples);
                
                // Read audio data and separate channel
                // set the `useReaderLeftChannel` and `useReaderRightChannel` to false!!!!
                mFormatReader->read(&fileBuffer, 0, numSamples, 0, false, false);
                
                audioBuffers[file.getFileNameWithoutExtension().getTrailingIntValue()] = std::move(fileBuffer);
                // if files are named like ****_C4_60.wav that would be very helpful!!
     
            }
        }
        
        
        
    }
    
    /*
    if (chooser.browseForFileToOpen())
    {
        auto file = chooser.getResult(); // get and store file?
        mFormatReader = mFormatManager.createReaderFor(file); // create a pointer for file?
        
        if (mFormatReader != nullptr)
        {
            // Print number of channels
            int numChannels = mFormatReader->numChannels;
            int numSamples = static_cast<int>(mFormatReader->lengthInSamples); //int64 to int32
            std::cout << "File loaded! Number of channels: " << numChannels << std::endl;
            
            // Create a buffer for file
            //juce::AudioBuffer<float> fileBuffer(numChannels, numSamples);
            //float* const* channelData = fileBuffer.getArrayOfWritePointers();
            juce::AudioSampleBuffer fileBuffer(numChannels, numSamples);
            
            // Read audio data and separate channel
            // set the `useReaderLeftChannel` and `useReaderRightChannel` to false!!!!
            mFormatReader->read(&fileBuffer, 0, numSamples, 0, false, false);

            
            // DBG: Print stuff on each channel
            // Create pointer to each individual channel
            // the fileBuffer is working nicely; all 4 channels has actual data on it
            for (int channel = 0; channel < numChannels; ++channel)
            {
                float* channelData = fileBuffer.getWritePointer(channel);
                for (int sample = 0; sample < numSamples; ++sample)
                {
                    std::cout << "Channel#: " << channel << ", Sample#: " << sample << ", Data: " << channelData[sample] << std::endl;
                }
            }
            
            // Pass audio buffer back to a transport source
 
        }
        
    }
    */

}


//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SpheringerAudioProcessor();
}
