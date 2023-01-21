#ifndef MS_H
#define MS_H

#include <JuceHeader.h>
#include "../config.h"

namespace audio
{
    class MS
    {
    public:
        MS() {;}
        ~MS() {;}
        
        void prepare(float _sampleRate, int _blockSize)
        {
            sampleRate = _sampleRate;
            blockSize = _blockSize;
            combOrder = floor(msInSamples(FILTER_DELAY_MS, sampleRate));
            psDelayAmt = floor(msInSamples(PS_DELAY_MS, sampleRate));
            
            mBuffer.setSize(1, blockSize);
            sBuffer.setSize(1, blockSize);
            psBuffer.setSize(1, blockSize);
            
            juce::dsp::ProcessSpec spec;
            spec.sampleRate = sampleRate;
            spec.numChannels = 1;
            spec.maximumBlockSize = blockSize;
            
            combDelay.prepare(spec);
            combDelay.setMaximumDelayInSamples(combOrder);
            combDelay.setDelay(combOrder);
            
            psDelay.prepare(spec);
            psDelay.setMaximumDelayInSamples(psDelayAmt);
            psDelay.setDelay(psDelayAmt);
            
            gainFactorL.setCurrentAndTargetValue(1.f);
            gainFactorR.setCurrentAndTargetValue(1.f);
            gainFactorL.reset(sampleRate, SMOOTH_SEC);
            gainFactorR.reset(sampleRate, SMOOTH_SEC);
        }
        
        void reset()
        {
            mBuffer.clear();
            sBuffer.clear();
            psBuffer.clear();
            combDelay.reset();
            psDelay.reset();
        }
        
        void setParams(float _width, float _stereoize)
        {
            width = _width;
            stereoize = _stereoize;
        }
        
        // stereo processing
        void process(AudioBuffer<float>& buffer)
        {
            blockSize = buffer.getNumSamples();
            auto inSamps = buffer.getArrayOfWritePointers();
            auto midSamps = mBuffer.getArrayOfWritePointers();
            auto psSamps = psBuffer.getArrayOfWritePointers();
            auto sideSamps = sBuffer.getArrayOfWritePointers();
            
            const float inRMSLeft = buffer.getRMSLevel(0, 0, blockSize);
            const float inRMSRight = buffer.getRMSLevel(1, 0, blockSize);
            
            // fill mid buffer L + R
            SIMD::copy(midSamps[0], inSamps[0], blockSize);
            SIMD::add(midSamps[0], inSamps[1], blockSize);
            
            // create pseudo-side buffer
            SIMD::copy(psSamps[0], midSamps[0], blockSize);
            applyCombFilter(psBuffer);
            SIMD::multiply(psSamps[0], stereoize, blockSize);
            
            // fill side buffer (L - R)
            SIMD::copy(sideSamps[0], inSamps[0], blockSize);
            SIMD::subtract(sideSamps[0], inSamps[1], blockSize);
            SIMD::add(sideSamps[0], psSamps[0], blockSize);
            SIMD::multiply(sideSamps[0], width, blockSize);
            
            // write to buffer
            // LEFT (M + S)
            SIMD::copy(inSamps[0], midSamps[0], blockSize);
            SIMD::add(inSamps[0], sideSamps[0], blockSize);
            SIMD::multiply(inSamps[0], 0.5f, blockSize);

            // RIGHT (M - S)
            SIMD::copy(inSamps[1], midSamps[0], blockSize);
            SIMD::subtract(inSamps[1], sideSamps[0], blockSize);
            SIMD::multiply(inSamps[1], 0.5f, blockSize);
        
            applyGainNorm(buffer, inRMSLeft, inRMSRight);
        }
        
        void processSample(float _width, float _stereoize, float* leftSamp, float* rightSamp)
        {
            float midSamp, sideSamp, psSamp;
            
            midSamp = *leftSamp + *rightSamp;
            sideSamp = *leftSamp - *rightSamp;
            
            combDelay.pushSample(0, midSamp);
            psSamp = midSamp - FILTER_FEEDBACK * combDelay.popSample(0, combOrder);
            
            psDelay.pushSample(0, psSamp);
            
            sideSamp += psDelay.popSample(0) * _stereoize;
            sideSamp *= _width;
            
            *leftSamp = (midSamp + sideSamp) / 2.0f;
            *rightSamp = (midSamp - sideSamp) / 2.0f;
        }
        
        void applyGainNorm(AudioBuffer<float>& buffer, float inRMSLeft, float inRMSRight)
        {
            float outRMSLeft = buffer.getRMSLevel(0, 0, blockSize);
            float outRMSRight = buffer.getRMSLevel(1, 0, blockSize);
            
            if (outRMSRight)
            {
                float gainFactor = (inRMSRight - inRMSLeft + outRMSLeft) / outRMSRight;
                gainFactorR.setTargetValue(gainFactor);
                
                auto wp = buffer.getWritePointer(1);
                for (int s = 0; s < blockSize; ++s)
                    wp[s] *= gainFactorR.getNextValue();
            }
            else
                gainFactorR.skip(blockSize);
                
        }
        
    private:
        void applyCombFilter(AudioBuffer<float>& buffer)
        {
            float* outData = buffer.getWritePointer(0);
            const float* inData = buffer.getReadPointer(0);
            
            for (int s = 0; s < blockSize; ++s)
            {
                combDelay.pushSample(0, inData[s]);
                float delayData = inData[s] - FILTER_FEEDBACK * combDelay.popSample(0, combOrder);
                outData[s] = delayData;
            }
        }
        
        float width, stereoize, sampleRate;
        int blockSize, combOrder, psDelayAmt;
        
        AudioBuffer<float> mBuffer, sBuffer, psBuffer;
        juce::dsp::DelayLine<float> combDelay, psDelay;
        juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> gainFactorL, gainFactorR;
    };
}

#endif
