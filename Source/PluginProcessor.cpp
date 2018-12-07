/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <math.h>
#include <algorithm>

//
const int maxNCh = 8;
double fgain = 0; //fade-in gain multipler
double sr = 48000.0;

double K = 1;
double norm = 1;

double *atemp; //temp swapping buffers for Z
double *btemp;

//Hi Pass variables
//double hpFreq = 300;
//double hpQ = 0.71;
const int hpOrder = 24;
//coeff
double hpa0;
double hpa1;
double hpa2;
double hpb1;
double hpb2;
//memory as pointers
double *hpz [maxNCh][6]; //array of pointers to each channel Z data -> a,b & 0,1,2

//Lo Pass variables
double lpFreq = 1000;
double lpQ = 1;
int lpOrder = 24;
//coeff
double lpa0;
double lpa1;
double lpa2;
double lpb1;
double lpb2;
//memory?
double *lpz [maxNCh][6]; //array of pointers to each channel Z data -> a,b & 0,1,2

//==============================================================================
NewProjectAudioProcessor::NewProjectAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    //auto nrQ = NormalisableRange<float>::NormalisableRange(0.1f, 10.0f, 0.1f, 0.1f, true);
    //auto nrF = NormalisableRange<float>::NormalisableRange(20.0f, 20000.0f, 0.1f, 0.1f, true);
    addParameter(hpFParam = new AudioParameterFloat ("HP Freq", // parameter ID
                                                     "HP Freq", // parameter name
                                                     NormalisableRange<float>(20.0f, 20000.0f, 0.1f, 0.1f, true),
                                                     20.0f)); // default value
    addParameter(hpQParam = new AudioParameterFloat ("HP Q", // parameter ID
                                                     "HP Q", // parameter name
                                                     NormalisableRange<float>(0.1f, 1.5f, 0.1f, 0.1f, true),
                                                     0.71f)); // default value
    addParameter(lpFParam = new AudioParameterFloat ("LP Freq", // parameter ID
                                                     "LP Freq", // parameter name
                                                     NormalisableRange<float>(20.0f, 20000.0f, 0.1f, 0.1f, true),
                                                     18000.0f)); // default value
    addParameter(lpQParam = new AudioParameterFloat ("LP Q", // parameter ID
                                                     "LP Q", // parameter name
                                                     NormalisableRange<float>(0.1f, 1.5f, 0.1f, 0.1f, true),
                                                     0.71f)); // default value
    
    
}

NewProjectAudioProcessor::~NewProjectAudioProcessor()
{
}

//==============================================================================
const String NewProjectAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NewProjectAudioProcessor::acceptsMidi() const
{
    return false;
}

bool NewProjectAudioProcessor::producesMidi() const
{
    return false;
}

bool NewProjectAudioProcessor::isMidiEffect() const
{
    return false;
}

double NewProjectAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NewProjectAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int NewProjectAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NewProjectAudioProcessor::setCurrentProgram (int index)
{
}

const String NewProjectAudioProcessor::getProgramName (int index)
{
    return {};
}

void NewProjectAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void NewProjectAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    sr = sampleRate;
    
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    //int nCh = getTotalNumInputChannels();
    fgain = 0;
    
    //Allocate memory for delayed samples
    for(int c=0; c<maxNCh; ++c) {
        for(int z=0; z<6; ++z) {
            //hpz[c][z] = (double*) malloc(sizeof(double)*hpOrder+1);
            //lpz[c][z] = (double*) malloc(sizeof(double)*hpOrder+1);
            hpz[c][z] = (double*) calloc(hpOrder+1, sizeof(double));
            lpz[c][z] = (double*) calloc(lpOrder+1, sizeof(double));
        }
    }
    
    //- - calc coefficients - -
    calcBiquadCoeffs();
    
}

void NewProjectAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    
    for(int c=0; c<maxNCh; ++c) {
        for(int z=0; z<6; ++z) {
            //free(hpz[c][z]);
            //free(lpz[c][z]);
        }
    }
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NewProjectAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
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

void NewProjectAudioProcessor::calcBiquadCoeffs()
{
    //- - calc coefficients - -
    //hipass coeffs
    K = tan(M_PI * (double)*hpFParam / sr);
    norm = 1.0 / (1.0 + K / (double)*hpQParam + K * K);
    hpa0 = 1.0 * norm;
    hpa1 = -2.0 * hpa0;
    hpa2 = hpa0;
    hpb1 = 2.0 * (K * K - 1.0) * norm;
    hpb2 = (1.0 - K / (double)*hpQParam + K * K) * norm;
    
    //lopass coeffs
    K = tan(M_PI * (double)*lpFParam / sr);
    norm = 1.0 / (1.0 + K / (double)*lpQParam + K * K);
    lpa0 = K * K * norm;
    lpa1 = 2.0 * lpa0;
    lpa2 = lpa0;
    lpb1 = 2.0 * (K * K - 1.0) * norm;
    lpb2 = (1.0 - K / (double)*lpQParam + K * K) * norm;
}


void NewProjectAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    calcBiquadCoeffs();
    
    ScopedNoDenormals noDenormals;
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

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    
    for (int channel=0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
        int len = buffer.getNumSamples();
        
        
        // ..do something to the data...
        //HP Filter
        for(int i=0; i<len; ++i) { //for each sample
            //gain
            fgain = (1<fgain) ? 1:fgain+0.0001;
            
            //hp delay (pointer reassignment)
            atemp = hpz[channel][0];
            hpz[channel][0] = hpz[channel][2];
            hpz[channel][2] = hpz[channel][1];
            hpz[channel][1] = atemp;
            btemp = hpz[channel][3];
            hpz[channel][3] = hpz[channel][5];
            hpz[channel][5] = hpz[channel][4];
            hpz[channel][4] = btemp;
            
            //lp delay (pointer reassignment)
            atemp = lpz[channel][0];
            lpz[channel][0] = lpz[channel][2];
            lpz[channel][2] = lpz[channel][1];
            lpz[channel][1] = atemp;
            btemp = lpz[channel][3];
            lpz[channel][3] = lpz[channel][5];
            lpz[channel][5] = lpz[channel][4];
            lpz[channel][4] = btemp;
            
            double fb = (double) *(channelData+i) * fgain; //feedback a0->b0
            for(int o=0; o<hpOrder; ++o) { //for each filter
                hpz[channel][0][o] = fb;
                //Transposed direct form II
                fb = (hpz[channel][2][o]*hpa2 - hpz[channel][5][o]*hpb2) + (hpz[channel][1][o]*hpa1 - hpz[channel][4][o]*hpb1) + fb*hpa0;
                hpz[channel][3][o] = fb;
                lpz[channel][0][o] = fb;
                //Transposed direct form II
                fb = (lpz[channel][2][o]*lpa2 - lpz[channel][5][o]*lpb2) + (lpz[channel][1][o]*lpa1 - lpz[channel][4][o]*lpb1) + fb*lpa0;
                lpz[channel][3][o] = fb;
            }
            //pull data from fb and write back to channelData[i]
            *(channelData+i) = (float)fb * fgain;
        }
    }
}

//==============================================================================
bool NewProjectAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}


AudioProcessorEditor* NewProjectAudioProcessor::createEditor()
{
    //return new NewProjectAudioProcessorEditor (*this);
    return new GenericEditor (*this);
}

//==============================================================================
void NewProjectAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void NewProjectAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NewProjectAudioProcessor();
}
