#pragma once

#include <vector>
#include <cmath>

static const float BAND_FREQUENCIES[] = {
    31.25f, 62.5f, 125.0f, 250.0f, 500.0f, 1000.0f, 2000.0f, 4000.0f, 8000.0f, 16000.0f
};

class BiquadFilter
{
public:
    BiquadFilter() { reset();  }

    void configure(float frequency, float sampleRate, float gaindB, float Q = 1.41f) 
    { 
        // Peaking EQ formula
        float w0 = 2.0f * 3.14159265f * frequency / sampleRate;
        float alpha = std::sin(w0) / (2.0f * Q);
        float A = std::pow(10.0f, gaindB / 40.0f);

        float b0 = 1.0f + alpha * A;
        float b1 = -2.0f * std::cos(w0);
        float b2 = 1.0f - alpha * A;
        float a0 = 1.0f + alpha / A;
        float a1 = -2.0f * std::cos(w0);
        float a2 = 1.0f - alpha / A;

        // Normalize
        _b0 = b0 / a0;
        _b1 = b1 / a0;
        _b2 = b2 / a0;
        _a1 = a1 / a0;
        _a2 = a2 / a0;
    }

    void reset() {
        x1 = x2 = y1 = y2 = 0.0f;
        _b0 = 1.0; _b1 = 0.0; _b2 = 0.0; _a1 = 0.0; _a2 = 0.0;
    }

    // Process one sample (Stereo safe if you maintain separate filters for L and R)
    inline float process(float x) {
        float y = _b0 * x + _b1 * x1 + _b2 * x2 - _a1 * y1 - _a2 * y2;
        x2 = x1;
        x1 = x;
        y2 = y1;
        y1 = y;
        return y;
    }

private:
    float _b0, _b1, _b2, _a1, _a2;
    float x1, x2, y1, y2;
};

class Equalizer10Band {
public:
    Equalizer10Band(int sampleRate);

    // Call this when UI slider moves. BandIndex 0-9, Gain in dB (-12 to +12)
    void updateBand(int bandIndex, float gaindB);

    // Process a buffer of Interleaved Stereo 16-bit PCM
    void process(uint8_t* data, int size);

private:
    int _sampleRate;
    std::vector<BiquadFilter> _filtersLeft;
    std::vector<BiquadFilter> _filtersRight;
};

