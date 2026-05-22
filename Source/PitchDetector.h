/*
  ==============================================================================

    PitchDetector.h
    Created: 19 May 2026 10:28:36pm
    Author:  Timothy

  ==============================================================================
*/

#pragma once

/*
  ==============================================================================
   PitchDetector - YIN Algorithm
  ==============================================================================
   Implements the YIN pitch detection algorithm (de Cheveigné & Kawahara, 2002).

   threshold  default: 0.15f
       Aperiodicity threshold. Lower values are stricter — fewer detections
       but more accurate. Raise if too many -1 (no pitch) returns.

   Recommended buffer sizes by frequency:
       E2  (82  Hz) @ 48kHz -> min 1170 samples -> use 2048
       A2  (110 Hz) @ 48kHz -> min  873 samples -> use 2048
       E4  (330 Hz) @ 48kHz -> min  291 samples -> use 512 or 1024

   Returns -1.0f when no pitch is detected or confidence is below threshold.
  ==============================================================================
*/

#include <JuceHeader.h>

class PitchDetector
{
public:
    PitchDetector();

    // Call this before processing
    void prepare(double sampleRate);

    // Returns detected pitch in Hz, or -1 if no pitch detected
    float detectPitch(const float* buffer, int numSamples);

private:
    double sampleRate = 44800.0;
    float  threshold = 0.15f;   // YIN confidence threshold

    std::vector<float> yinBuffer;

    float yinDetect(const float* buffer, int numSamples);
    void  difference(const float* buffer, int numSamples);
    void  cumulativeMeanNormalizedDifference(int numSamples);
    int   absoluteThreshold(int numSamples);
    float parabolicInterpolation(int tauEstimate, int numSamples);
};
