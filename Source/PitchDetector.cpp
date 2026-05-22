/*
  ==============================================================================

    PitchDetector.cpp
    Created: 19 May 2026 10:28:25pm
    Author:  Timothy

  ==============================================================================
*/

#include "PitchDetector.h"

PitchDetector::PitchDetector() {}

void PitchDetector::prepare(double sr)
{
    sampleRate = sr;
}

float PitchDetector::detectPitch(const float* buffer, int numSamples)
{
    if (numSamples < 2) return -1.0f;
    return yinDetect(buffer, numSamples);
}

float PitchDetector::yinDetect(const float* buffer, int numSamples)
{
    int halfSize = numSamples / 2;
    yinBuffer.assign(halfSize, 0.0f);

    difference(buffer, numSamples);
    cumulativeMeanNormalizedDifference(halfSize);

    int tauEstimate = absoluteThreshold(halfSize);
    if (tauEstimate == -1) return -1.0f;

    float pitch = parabolicInterpolation(tauEstimate, halfSize);
    if (pitch <= 0.0f) return -1.0f;

    return (float)(sampleRate / pitch);
}

void PitchDetector::difference(const float* buffer, int numSamples)
{
    int halfSize = numSamples / 2;
    for (int tau = 0; tau < halfSize; ++tau)
    {
        float sum = 0.0f;
        for (int i = 0; i < halfSize; ++i)
        {
            float delta = buffer[i] - buffer[i + tau];
            sum += delta * delta;
        }
        yinBuffer[tau] = sum;
    }
}

void PitchDetector::cumulativeMeanNormalizedDifference(int halfSize)
{
    yinBuffer[0] = 1.0f;
    float runningSum = 0.0f;
    for (int tau = 1; tau < halfSize; ++tau)
    {
        runningSum += yinBuffer[tau];
        yinBuffer[tau] *= tau / runningSum;
    }
}

int PitchDetector::absoluteThreshold(int halfSize)
{
    for (int tau = 2; tau < halfSize; ++tau)
    {
        if (yinBuffer[tau] < threshold)
        {
            // Find local minimum
            while (tau + 1 < halfSize && yinBuffer[tau + 1] < yinBuffer[tau])
                ++tau;
            return tau;
        }
    }
    return -1;
}

float PitchDetector::parabolicInterpolation(int tauEstimate, int halfSize)
{
    int x0 = (tauEstimate < 1) ? tauEstimate : tauEstimate - 1;
    int x2 = (tauEstimate + 1 < halfSize) ? tauEstimate + 1 : tauEstimate;

    if (x0 == tauEstimate)
        return (yinBuffer[tauEstimate] <= yinBuffer[x2]) ? (float)tauEstimate : (float)x2;

    if (x2 == tauEstimate)
        return (yinBuffer[tauEstimate] <= yinBuffer[x0]) ? (float)tauEstimate : (float)x0;

    float s0 = yinBuffer[x0];
    float s1 = yinBuffer[tauEstimate];
    float s2 = yinBuffer[x2];

    return tauEstimate + (s2 - s0) / (2.0f * (2.0f * s1 - s2 - s0));
}
