#pragma once
#include <cmath>
#include <vector>
#include <algorithm>

class ShelvingFilter {
public:
    enum Type { LOW_SHELF, HIGH_SHELF };

    void configure(Type type, float cutoffFreq, float sampleRate, float gaindB);

    float process(float x);

private:
    float _b0 = 1, _b1 = 0, _b2 = 0, _a1 = 0, _a2 = 0;
    float x1 = 0, x2 = 0, y1 = 0, y2 = 0;
};

class ToneControl {
public:
    ToneControl(int sampleRate);

    void setBass(float db);

    void setTreble(float db);

    void process(uint8_t* data, int size);

private:
    float _sampleRate;
    ShelvingFilter _bassL, _bassR;
    ShelvingFilter _trebleL, _trebleR;
};

