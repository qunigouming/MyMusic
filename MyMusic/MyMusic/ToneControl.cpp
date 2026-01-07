#include "ToneControl.h"

void ShelvingFilter::configure(Type type, float cutoffFreq, float sampleRate, float gaindB) {
    float A = std::pow(10.0f, gaindB / 40.0f);
    float w0 = 2.0f * 3.14159265f * cutoffFreq / sampleRate;
    float sinW = std::sin(w0);
    float cosW = std::cos(w0);
    float alpha = sinW / 2.0f * std::sqrt(2.0f); // Q = 0.707

    float b0, b1, b2, a0, a1, a2;

    if (type == LOW_SHELF) {
        b0 = A * ((A + 1.0f) - (A - 1.0f) * cosW + 2.0f * std::sqrt(A) * alpha);
        b1 = 2.0f * A * ((A - 1.0f) - (A + 1.0f) * cosW);
        b2 = A * ((A + 1.0f) - (A - 1.0f) * cosW - 2.0f * std::sqrt(A) * alpha);
        a0 = (A + 1.0f) + (A - 1.0f) * cosW + 2.0f * std::sqrt(A) * alpha;
        a1 = -2.0f * ((A - 1.0f) + (A + 1.0f) * cosW);
        a2 = (A + 1.0f) + (A - 1.0f) * cosW - 2.0f * std::sqrt(A) * alpha;
    }
    else { // HIGH_SHELF
        b0 = A * ((A + 1.0f) + (A - 1.0f) * cosW + 2.0f * std::sqrt(A) * alpha);
        b1 = -2.0f * A * ((A - 1.0f) + (A + 1.0f) * cosW);
        b2 = A * ((A + 1.0f) + (A - 1.0f) * cosW - 2.0f * std::sqrt(A) * alpha);
        a0 = (A + 1.0f) - (A - 1.0f) * cosW + 2.0f * std::sqrt(A) * alpha;
        a1 = 2.0f * ((A - 1.0f) - (A + 1.0f) * cosW);
        a2 = (A + 1.0f) - (A - 1.0f) * cosW - 2.0f * std::sqrt(A) * alpha;
    }

    _b0 = b0 / a0; _b1 = b1 / a0; _b2 = b2 / a0;
    _a1 = a1 / a0; _a2 = a2 / a0;
}

float ShelvingFilter::process(float x) {
    float y = _b0 * x + _b1 * x1 + _b2 * x2 - _a1 * y1 - _a2 * y2;
    x2 = x1; x1 = x;
    y2 = y1; y1 = y;
    return y;
}

ToneControl::ToneControl(int sampleRate) {
    _sampleRate = sampleRate;
    setBass(0.0f);   // 0dB default
    setTreble(0.0f); // 0dB default
}

void ToneControl::setBass(float db) {
    // Cutoff around 200Hz for bass
    _bassL.configure(ShelvingFilter::LOW_SHELF, 200.0f, _sampleRate, db);
    _bassR.configure(ShelvingFilter::LOW_SHELF, 200.0f, _sampleRate, db);
}

void ToneControl::setTreble(float db) {
    // Cutoff around 3000Hz for treble
    _trebleL.configure(ShelvingFilter::HIGH_SHELF, 3000.0f, _sampleRate, db);
    _trebleR.configure(ShelvingFilter::HIGH_SHELF, 3000.0f, _sampleRate, db);
}

void ToneControl::process(uint8_t* data, int size) {
    int16_t* samples = reinterpret_cast<int16_t*>(data);
    int count = size / 2;
    for (int i = 0; i < count; i += 2) {
        float l = samples[i];
        float r = samples[i + 1];

        // Apply Bass then Treble
        l = _trebleL.process(_bassL.process(l));
        r = _trebleR.process(_bassR.process(r));

        samples[i] = static_cast<int16_t>(std::clamp(l, -32768.0f, 32767.0f));
        samples[i + 1] = static_cast<int16_t>(std::clamp(r, -32768.0f, 32767.0f));
    }
}
