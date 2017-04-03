/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#ifndef PLUGINEDITOR_H_INCLUDED
#define PLUGINEDITOR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"


//==============================================================================
/**
*/
class VocoderAudioProcessorEditor  : public AudioProcessorEditor,
                                     private Slider::Listener, Timer
{
public:
    VocoderAudioProcessorEditor (VocoderAudioProcessor&);
    ~VocoderAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    //==============================================================================
    
    enum Sliders_Enum {
        Slider_Frequncy_Bands,
        Slider_Q,
        Slider_Count
    };
    
    
    
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    VocoderAudioProcessor& processor;
    
    void timerCallback() override;
    AudioVisualiserComponent vis;
    
    Label label;
    
    // From Slider::Listener
    void sliderValueChanged(Slider* slider) override;
    
    Slider Sliders[Sliders_Enum::Slider_Count];
    void initaliseAllSliders();
    Sliders_Enum getSliderFromPointer(Slider * slider);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VocoderAudioProcessorEditor)
};


#endif  // PLUGINEDITOR_H_INCLUDED
