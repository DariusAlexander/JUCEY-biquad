/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class NewProjectAudioProcessorEditor  : public AudioProcessorEditor
{
public:
    NewProjectAudioProcessorEditor (NewProjectAudioProcessor&);
    ~NewProjectAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    NewProjectAudioProcessor& processor;
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NewProjectAudioProcessorEditor)
};



class GenericEditor : public AudioProcessorEditor,
private Timer
{
public:
    enum
    {
        paramControlHeight = 40,
        paramLabelWidth    = 80,
        paramSliderWidth   = 300
    };
    
    GenericEditor (AudioProcessor& parent)
    : AudioProcessorEditor (parent)
    {
        auto& params = parent.getParameters();
        
        for (auto p : params)
        {
            if (auto* param = dynamic_cast<AudioParameterFloat*> (p))
            {
                Slider* aSlider;
                
                paramSliders.add (aSlider = new Slider (param->name));
                aSlider->setRange (param->range.start, param->range.end);
                aSlider->setSliderStyle (Slider::LinearHorizontal);
                aSlider->setValue (*param);
                
                aSlider->onValueChange = [this, aSlider] { changeSliderValue (aSlider); };
                aSlider->onDragStart   = [this, aSlider] { startDragChange (aSlider); };
                aSlider->onDragEnd     = [this, aSlider] { endDragChange (aSlider); };
                addAndMakeVisible (aSlider);
                
                Label* aLabel;
                paramLabels.add (aLabel = new Label (param->name, param->name));
                addAndMakeVisible (aLabel);
            }
        }
        
        noParameterLabel.setJustificationType (Justification::horizontallyCentred | Justification::verticallyCentred);
        noParameterLabel.setFont (noParameterLabel.getFont().withStyle (Font::italic));
        
        if (paramSliders.size() == 0)
        {
            addAndMakeVisible (noParameterLabel);
            setSize (300, 100);
        }
        else
        {
            setSize (paramSliderWidth + paramLabelWidth,
                     jmax (100, paramControlHeight * paramSliders.size()));
            
            startTimer (100);
        }
    }
    
    ~GenericEditor() {}
    
    void resized() override
    {
        auto r = getLocalBounds();
        noParameterLabel.setBounds (r);
        
        for (auto i = 0; i < paramSliders.size(); ++i)
        {
            auto paramBounds = r.removeFromTop (paramControlHeight);
            auto labelBounds = paramBounds.removeFromLeft (paramLabelWidth);
            
            paramLabels[i]->setBounds (labelBounds);
            paramSliders[i]->setBounds (paramBounds);
        }
    }
    
    void paint (Graphics& g) override
    {
        g.setColour (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
        g.fillAll();
    }
    
    //==============================================================================
    void changeSliderValue (Slider* slider)
    {
        if (auto* param = getParameterForSlider (slider))
            *param = (float) slider->getValue();
    }
    
    void startDragChange (Slider* slider)
    {
        if (auto* param = getParameterForSlider (slider))
            param->beginChangeGesture();
    }
    
    void endDragChange (Slider* slider)
    {
        if (auto* param = getParameterForSlider (slider))
            param->endChangeGesture();
    }
    
private:
    void timerCallback() override
    {
        auto& params = getAudioProcessor()->getParameters();
        
        for (auto i = 0; i < params.size(); ++i)
        {
            if (auto* param = dynamic_cast<AudioParameterFloat*> (params[i]))
            {
                if (i < paramSliders.size())
                    paramSliders[i]->setValue (*param);
            }
        }
    }
    
    AudioParameterFloat* getParameterForSlider (Slider* slider)
    {
        auto& params = getAudioProcessor()->getParameters();
        return dynamic_cast<AudioParameterFloat*> (params[paramSliders.indexOf (slider)]);
    }
    
    Label noParameterLabel { "noparam", "No parameters available" };
    OwnedArray<Slider> paramSliders;
    OwnedArray<Label> paramLabels;
};
