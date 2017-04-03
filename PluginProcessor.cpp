/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"



//==============================================================================
VocoderAudioProcessor::VocoderAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                       .withInput  ("Sidechain",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

VocoderAudioProcessor::~VocoderAudioProcessor()
{
}

//==============================================================================
const String VocoderAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool VocoderAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool VocoderAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

double VocoderAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int VocoderAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int VocoderAudioProcessor::getCurrentProgram()
{
    return 0;
}

void VocoderAudioProcessor::setCurrentProgram (int index)
{
}

const String VocoderAudioProcessor::getProgramName (int index)
{
    return String();
}

void VocoderAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void VocoderAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    Q = 0.707;
    
    NumberOfBands = MAX_BANDS;
    calculateBands();
    
    visBuffer.setSize(2, samplesPerBlock);
    
    for (int channel = 0; channel < 2; channel++)
    {
        for (int i = 0; i < MAX_BANDS; i++)
        {
            EnvelopeFilter[channel][i].setCoefficients(IIRCoefficients::makeLowPass(sampleRate, 100));
        }
    }
    
    for (int i = 0; i < MAX_BANDS; i++)
    {
        normalBuffers[i].setSize(2, samplesPerBlock);
        normalBuffers[i].clear();
        sidechainBuffers[i].setSize(2, samplesPerBlock);
        sidechainBuffers[i].clear();
    }
}

void VocoderAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool VocoderAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void VocoderAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    const int totalNumInputChannels   = getTotalNumInputChannels();
    const int totalNumOutputChannels  = getTotalNumOutputChannels();

    AudioSampleBuffer normalBufferProcess = getBusBuffer(buffer, true, 0);
    AudioSampleBuffer sideChainBufferProcess  = getBusBuffer(buffer, true, 1);
    
    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    {
        buffer.clear (i, 0, buffer.getNumSamples());
    }
    
    // Copy inputs from host to all Vocoder Buffers
    for (int ch = 0; ch < getTotalNumInputChannels(); ch++)
    {
        for (int i = 0; i < NumberOfBands; i++)
        {
            if (ch < 2) {
                normalBuffers[i].copyFrom(ch, 0, normalBufferProcess, ch, 0, normalBufferProcess.getNumSamples());
                float *ptr = normalBuffers[i].getWritePointer(ch);
                
                // Apply respective Vocoder Filter to input1&2's respective buffers
                Filters[ch][i].processSamples(ptr, normalBuffers[i].getNumSamples());
            }
            if (ch > 1) {
                sidechainBuffers[i].copyFrom(ch-2, 0, sideChainBufferProcess, ch-2, 0, sideChainBufferProcess.getNumSamples());
                float *ptr = sidechainBuffers[i].getWritePointer(ch-2);
                
                // Apply respective Vocoder Filter to input3&4's respective buffers
                Filters[ch][i].processSamples(ptr, sidechainBuffers[i].getNumSamples());
            }
        }
    }
    
    // Convert the sidechain buffers to windows/ envelopes
    for (int channel = 0; channel < totalNumOutputChannels; channel++)
    {
        for (int band = 0; band < NumberOfBands; band++)
        {
            const float *readPtr  = sidechainBuffers[band].getReadPointer(channel);
            float       *writePtr = sidechainBuffers[band].getWritePointer(channel);
            
            FloatVectorOperations::abs(writePtr, readPtr, sidechainBuffers[band].getNumSamples());
        
            EnvelopeFilter[channel][band].processSamples(writePtr, sidechainBuffers[band].getNumSamples());
            
            sidechainBuffers[band].applyGain(4.0);
        }
    }
    
    visBuffer.clear();
    
    // Apply envelopes to 1&2's buffers
    for (int channel = 0; channel < normalBuffers[0].getNumChannels(); channel++)
    {
        for (int band = 0; band < NumberOfBands; band++)
        {
            const float *sidechainReadPtr = sidechainBuffers[band].getReadPointer(channel);
            
            float *normalWritePtr = normalBuffers[band].getWritePointer(channel);
            
            FloatVectorOperations::multiply(normalWritePtr, sidechainReadPtr, normalBuffers[band].getNumSamples());
            
            visBuffer.addFrom(channel, 0, normalBuffers[band], channel, 0, normalBuffers[band].getNumSamples());
        }
    }
    
    for (int channel = 0; channel < totalNumOutputChannels; channel++)
    {
        buffer.copyFrom(channel, 0, visBuffer, channel, 0, visBuffer.getNumSamples());
    }
    
}

//==============================================================================
bool VocoderAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* VocoderAudioProcessor::createEditor()
{
    return new VocoderAudioProcessorEditor (*this);
}

//==============================================================================
void VocoderAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void VocoderAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

void VocoderAudioProcessor::calculateBands()
{
    double Step = log2((double)(20000.0 / NumberOfBands));
    
    for (int ch = 0; ch < getTotalNumInputChannels(); ch++)
        for (int i = 0; i < NumberOfBands; i++) {
            double centerFreq = (pow(2, Step) * i) + 20;
        
            Filters[ch][i].setCoefficients(IIRCoefficients::makeBandPass(getSampleRate(), centerFreq, Q));
    }
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VocoderAudioProcessor();
}
