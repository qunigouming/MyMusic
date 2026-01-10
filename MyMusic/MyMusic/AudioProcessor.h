#pragma once

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/efx.h>
#include <AL/efx-presets.h>
#include <memory>

enum class EnvironmentPreset {
    None,
    Bathroom,
    Church,
    ConcertHall,
    Room,
    VocalBoard,    // Custom: Very short, bright reverb
    Spring,        // Custom: Metallic, "boingy" sound
    Psychotic,
    Underground    // Custom: Long echo, metallic
};

class AudioProcessor
{
public:
    struct ALCdeviceDeleter {
        void operator()(ALCdevice* device) const {
            if (device) alcCloseDevice(device);
        }
    };

    struct ALCcontextDeleter {
        void operator()(ALCcontext* context) const {
            if (context) {
                // if has current context, make it none
                if (alcGetCurrentContext() == context)
                    alcMakeContextCurrent(nullptr);
                alcDestroyContext(context);
            }
        }
    };
    AudioProcessor() = default;
	~AudioProcessor() = default;

    bool initialize();

    ALuint getEffectSlot() { return _slot; }

    // EFX Function Pointers
    LPALGENEFFECTS alGenEffects = nullptr;
    LPALDELETEEFFECTS alDeleteEffects = nullptr;
    LPALISEFFECT alIsEffect = nullptr;
    LPALEFFECTI alEffecti = nullptr;
    LPALEFFECTF alEffectf = nullptr;

    LPALGENAUXILIARYEFFECTSLOTS alGenAuxiliaryEffectSlots = nullptr;
    LPALDELETEAUXILIARYEFFECTSLOTS alDeleteAuxiliaryEffectSlots = nullptr;
    LPALAUXILIARYEFFECTSLOTI alAuxiliaryEffectSloti = nullptr;

    void setEnvironment(EnvironmentPreset preset);
    void setSurroundDepth(int value);    // -10 to 10 (Slider)
    void setSurroundStrength(int value); // 0 to 10 (Slider)
    
private:
    bool loadEFXFunctions();

private:
    using ALCdevicePtr = std::unique_ptr<ALCdevice, ALCdeviceDeleter>;
    using ALCcontextPtr = std::unique_ptr<ALCcontext, ALCcontextDeleter>;
    ALCdevicePtr _device;
    ALCcontextPtr _context;
    ALuint _effect = 0;
    ALuint _filter = 0;
    ALuint _slot = 0;

    float _currentDecayTime = 1.49f;
    float _currentGain = 0.32f;
};

