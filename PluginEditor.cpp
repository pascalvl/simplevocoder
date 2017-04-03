/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
VocoderAudioProcessorEditor::VocoderAudioProcessorEditor (VocoderAudioProcessor& p)
: AudioProcessorEditor (&p), Timer(),  processor(p), vis(2)
{
    startTimerHz(60);
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 330);
    
    initaliseAllSliders();
    
    vis.setBounds(0, 20, 400, 280);
    vis.setBufferSize(processor.sidechainBuffers[processor.NumberOfBands-1].getNumSamples());
    addAndMakeVisible(vis);
}

VocoderAudioProcessorEditor::~VocoderAudioProcessorEditor()
{
    
}

//==============================================================================
void VocoderAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (Colours::white);

}

void VocoderAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}

void VocoderAudioProcessorEditor::sliderValueChanged(Slider* slider)
{
    switch (getSliderFromPointer(slider)) {
        case Slider_Frequncy_Bands:
        {
            int sliderValue = slider->getValueObject().getValue();
            processor.NumberOfBands = sliderValue;
            slider->setValue(sliderValue);
            processor.calculateBands();
        } break;
        
        case Slider_Q:
        {
            float sliderValue = slider->getValueObject().getValue();
            processor.Q = sliderValue;
            slider->setValue(sliderValue);
            processor.calculateBands();
        } break;
            
        default: break;
    }
}

VocoderAudioProcessorEditor::Sliders_Enum VocoderAudioProcessorEditor::getSliderFromPointer(Slider* slider)
{
    Sliders_Enum Result;
    for (int s = 0; s < Slider_Count; s++)
    {
        if (slider == &Sliders[s])
        {
            Result = (Sliders_Enum) s;
        }
    }
    return Result;
}

// Called from within VocoderAudioProcessorEditor's constructor
void VocoderAudioProcessorEditor::initaliseAllSliders()
{
    Sliders_Enum CurrentSlider;
    
    for (int s = 0; s < Slider_Count; s++)
    {
        CurrentSlider = (Sliders_Enum) s;
        
        switch (CurrentSlider)
        {
            case Slider_Frequncy_Bands:
            {
                Sliders[Slider_Frequncy_Bands].addListener(this);
                Sliders[Slider_Frequncy_Bands].setBounds(0, 0, 400, 20);
                Sliders[Slider_Frequncy_Bands].setRange(1.0, MAX_BANDS);
                Sliders[Slider_Frequncy_Bands].setValue(processor.NumberOfBands);
                processor.calculateBands();
                addAndMakeVisible(Sliders[Slider_Frequncy_Bands]);
            } break;
                
            case Slider_Q:
            {
                Sliders[Slider_Q].addListener(this);
                Sliders[Slider_Q].setBounds(0, 300, 400, 30);
                Sliders[Slider_Q].setRange(0.1, 10);
                Sliders[Slider_Q].setValue(processor.Q);
                addAndMakeVisible(Sliders[Slider_Q]);
            } break;
                
            default: break;
        }
    }
}

void VocoderAudioProcessorEditor::timerCallback()
{
    vis.pushBuffer(processor.visBuffer);
}
