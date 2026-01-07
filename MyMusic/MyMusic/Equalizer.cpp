#include "Equalizer.h"

Equalizer10Band::Equalizer10Band(int sampleRate) : _sampleRate(sampleRate) {
    // Create 2 filters (Left/Right) for each of the 10 bands
    _filtersLeft.resize(10);
    _filtersRight.resize(10);

    // Initialize with 0dB gain
    for (int i = 0; i < 10; ++i) {
        updateBand(i, 0.0f);
    }
}

void Equalizer10Band::updateBand(int bandIndex, float gaindB) {
    if (bandIndex < 0 || bandIndex >= 10) return;

    _filtersLeft[bandIndex].configure(BAND_FREQUENCIES[bandIndex], _sampleRate, gaindB);
    _filtersRight[bandIndex].configure(BAND_FREQUENCIES[bandIndex], _sampleRate, gaindB);
}

void Equalizer10Band::process(uint8_t* data, int size) {
    int16_t* samples = reinterpret_cast<int16_t*>(data);
    int sampleCount = size / 2; // size in bytes / 2 bytes per sample

                                // Interleaved Stereo: L, R, L, R...
    for (int i = 0; i < sampleCount; i += 2) {
        float left = samples[i];
        float right = samples[i + 1];

        // Run through all 10 bands
        for (int b = 0; b < 10; ++b) {
            left = _filtersLeft[b].process(left);
            right = _filtersRight[b].process(right);
        }

        // Clipping protection (Clamp to int16 range)
        if (left > 32767.0f) left = 32767.0f;
        if (left < -32768.0f) left = -32768.0f;
        if (right > 32767.0f) right = 32767.0f;
        if (right < -32768.0f) right = -32768.0f;

        samples[i] = static_cast<int16_t>(left);
        samples[i + 1] = static_cast<int16_t>(right);
    }
}
