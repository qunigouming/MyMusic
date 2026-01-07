#include "AudioProcessor.h"
#include <stdexcept>
#include "LogManager.h"

void AudioProcessor::setEnvironment(EnvironmentPreset preset)
{
    EFXEAXREVERBPROPERTIES props;

    // Load base values based on selection
    switch (preset) {
    case EnvironmentPreset::None:
        // Turn off
        alEffecti(_effect, AL_EFFECT_TYPE, AL_EFFECT_NULL);
        // Important: Update slot to use null effect
        alAuxiliaryEffectSloti(_slot, AL_EFFECTSLOT_EFFECT, _effect);
        return;

    case EnvironmentPreset::Bathroom:
        props = EFX_REVERB_PRESET_BATHROOM;
        break;
    case EnvironmentPreset::Church:
        props = EFX_REVERB_PRESET_STONECORRIDOR; // Good approximation
        break;
    case EnvironmentPreset::ConcertHall:
        props = EFX_REVERB_PRESET_CONCERTHALL;
        break;
    case EnvironmentPreset::Room:
        props = EFX_REVERB_PRESET_LIVINGROOM;
        break;
    case EnvironmentPreset::VocalBoard:
        // Custom: Short decay, high clarity
        props = EFX_REVERB_PRESET_GENERIC;
        props.flDecayTime = 0.4f;
        props.flGain = 0.5f;
        break;
    case EnvironmentPreset::Spring:
        // Custom: Simulate spring reverb (high density, metallic)
        props = EFX_REVERB_PRESET_GENERIC;
        props.flDecayTime = 2.5f;
        props.flDensity = 1.0f;
        props.flDiffusion = 0.0f; // Low diffusion sounds metallic
        break;
    case EnvironmentPreset::Psychotic:
        props = EFX_REVERB_PRESET_PSYCHOTIC; // Or similar
        break;
    case EnvironmentPreset::Underground:
        props = EFX_REVERB_PRESET_CITY_UNDERPASS;
        break;
    default:
        props = EFX_REVERB_PRESET_GENERIC;
        break;
    }

    // Activate Reverb Type
    alEffecti(_effect, AL_EFFECT_TYPE, AL_EFFECT_REVERB);

    // Apply all parameters
    alEffectf(_effect, AL_REVERB_DENSITY, props.flDensity);
    alEffectf(_effect, AL_REVERB_DIFFUSION, props.flDiffusion);
    alEffectf(_effect, AL_REVERB_GAIN, props.flGain);
    alEffectf(_effect, AL_REVERB_GAINHF, props.flGainHF);
    alEffectf(_effect, AL_REVERB_DECAY_TIME, props.flDecayTime);
    alEffectf(_effect, AL_REVERB_DECAY_HFRATIO, props.flDecayHFRatio);
    alEffectf(_effect, AL_REVERB_REFLECTIONS_GAIN, props.flReflectionsGain);
    alEffectf(_effect, AL_REVERB_REFLECTIONS_DELAY, props.flReflectionsDelay);
    alEffectf(_effect, AL_REVERB_LATE_REVERB_GAIN, props.flLateReverbGain);
    alEffectf(_effect, AL_REVERB_LATE_REVERB_DELAY, props.flLateReverbDelay);

    // Store these so sliders can modify them relatively
    _currentDecayTime = props.flDecayTime;
    _currentGain = props.flGain;

    // Update Slot
    alAuxiliaryEffectSloti(_slot, AL_EFFECTSLOT_EFFECT, _effect);
}

void AudioProcessor::setSurroundDepth(float percent)
{
    // Modulate the decay time based on the current preset baseline
    // Or just set strict limits (e.g., 0.1s to 10.0s)

    float newDecay = _currentDecayTime * (0.5f + percent * 1.5f); // Range: 50% to 200% of preset
    alEffectf(_effect, AL_REVERB_DECAY_TIME, newDecay);

    // You must update the slot for changes to take effect immediately
    alAuxiliaryEffectSloti(_slot, AL_EFFECTSLOT_EFFECT, _effect);
}

void AudioProcessor::setSurroundStrength(float percent)
{
    // Map slider to Gain (0.0 silent to 1.0 max)
    // Note: Reverb gain usually maxes at 1.0, but can go lower
    alEffectf(_effect, AL_REVERB_GAIN, percent); // Direct mapping

    alAuxiliaryEffectSloti(_slot, AL_EFFECTSLOT_EFFECT, _effect);
}

bool AudioProcessor::initialize()
{
    try {
        // 打开设备
        _device = ALCdevicePtr(alcOpenDevice(nullptr));
        if (!_device) {
            LOG(ERROR) << "Failed to open device";
            return false;
        }

        // 创建上下文
        _context = ALCcontextPtr(alcCreateContext(_device.get(), nullptr));
        if (!_context) {
            LOG(ERROR) << "Failed to create context";
            return false;
        }

        // 设为当前上下文
        if (!alcMakeContextCurrent(_context.get())) {
            LOG(ERROR) << "Failed to make context current";
            return false;
        }

        if (!loadEFXFunctions()) {
            LOG(ERROR) << "load EFX failed!!!";
            return false;
        }

        // 检查错误
        ALenum error = alGetError();
        return (error == AL_NO_ERROR);
    }
    catch (...) {
        return false;
    }
}

bool AudioProcessor::loadEFXFunctions()
{
    if (!alcIsExtensionPresent(alcGetContextsDevice(alcGetCurrentContext()), "ALC_EXT_EFX")) {
        return false;
    }
    // Load pointers
    alGenEffects = (LPALGENEFFECTS)alGetProcAddress("alGenEffects");
    alDeleteEffects = (LPALDELETEEFFECTS)alGetProcAddress("alDeleteEffects");
    alIsEffect = (LPALISEFFECT)alGetProcAddress("alIsEffect");
    alEffecti = (LPALEFFECTI)alGetProcAddress("alEffecti");
    alEffectf = (LPALEFFECTF)alGetProcAddress("alEffectf");

    alGenAuxiliaryEffectSlots = (LPALGENAUXILIARYEFFECTSLOTS)alGetProcAddress("alGenAuxiliaryEffectSlots");
    alDeleteAuxiliaryEffectSlots = (LPALDELETEAUXILIARYEFFECTSLOTS)alGetProcAddress("alDeleteAuxiliaryEffectSlots");
    alAuxiliaryEffectSloti = (LPALAUXILIARYEFFECTSLOTI)alGetProcAddress("alAuxiliaryEffectSloti");
    return (alGenEffects && alGenAuxiliaryEffectSlots);
}
