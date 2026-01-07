#include "AudioStreamProcessor.h"

#include <algorithm>

#include <AL/efx-presets.h>

#include "LogManager.h"

#define  NUM_BUFFERS 4

AudioStreamProcessor::AudioStreamProcessor()
{
}

AudioStreamProcessor::~AudioStreamProcessor()
{
    stop();
    alDeleteBuffers(_buffers.size(), _buffers.data());
}

bool AudioStreamProcessor::initialize()
{
    _processor = std::make_unique<AudioProcessor>();
    if (!_processor->initialize()) {
        return false;
    }
    createOpenALResources();

    return true;
}

void AudioStreamProcessor::createOpenALResources()
{
    // create audio source
    alGenSources(1, &_source);

    _buffers.resize(NUM_BUFFERS);
    // create buffers
    alGenBuffers(_buffers.size(), _buffers.data());

    // set attributes
    alSourcef(_source, AL_PITCH, 1.0f);
    alSourcef(_source, AL_GAIN, 1.0f);
    alSource3f(_source, AL_POSITION, 0.0f, 0.0f, 0.0f);
    alSource3f(_source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
    alSourcei(_source, AL_LOOPING, AL_FALSE);
}

void AudioStreamProcessor::setVolume(float volume)
{
    volume = std::clamp(volume, 0.0f, 1.0f);
    if (_source) {
        alSourcef(_source, AL_GAIN, volume);
    }
}

bool AudioStreamProcessor::hasBuffer()
{
    ALint processed;
    alGetSourcei(_source, AL_BUFFERS_PROCESSED, &processed);

    ALint state = 0;
    alGetSourcei(_source, AL_SOURCE_STATE, &state);
    if (state != AL_PLAYING && state != AL_PAUSED) {
        // check if there is enough space in the queue
        ALint queued;
        alGetSourcei(_source, AL_BUFFERS_QUEUED, &queued);
        return queued < NUM_BUFFERS;        //  enough space
    }
    return processed > 0;
}

void AudioStreamProcessor::start()
{
    ALint state;
    alGetSourcei(_source, AL_SOURCE_STATE, &state);
    if (state != AL_PLAYING) {
        alSourcePlay(_source);
    }
}

void AudioStreamProcessor::stop()
{
    alSourceStop(_source);
}

bool AudioStreamProcessor::write(uint8_t* data, int size)
{
    ALint processed = 0;
    
    alGetSourcei(_source, AL_BUFFERS_PROCESSED, &processed);
    ALuint bufferID = 0;
    // process has been played buffer
    if (processed > 0) {
        alSourceUnqueueBuffers(_source, 1, &bufferID);

        ALenum error = alGetError();
        if (error != AL_NO_ERROR) {
            LOG(ERROR) << "Failed to unqueue buffer, error: " << error;
            return false;
        }
    }
    else {
        ALint queued = 0;
        alGetSourcei(_source, AL_BUFFERS_QUEUED, &queued);

        if (queued < NUM_BUFFERS) {
            // 使用未使用的缓冲区
            bufferID = _buffers[queued];

            LOG(INFO) << "Filled new buffer, total queued: " << (queued + 1);
        }
        else {
            // 所有缓冲区都在使用中，且没有已播放完的缓冲区
            LOG(WARNING) << "No free buffers available, dropping audio data";
            return false;
        }
    }

    // fill data to the buffer
    alBufferData(bufferID, AL_FORMAT_STEREO16, data, size, 44100);

    ALenum error = alGetError();
    if (error != AL_NO_ERROR) {
        LOG(ERROR) << "alBufferData failed: , error: " << error;
        return false;
    }

    alSourceQueueBuffers(_source, 1, &bufferID);

    if ((error = alGetError()) != AL_NO_ERROR) {
        LOG(ERROR) << "alSourceQueueBuffers failed: , error: " << error;
        return false;
    }

    // Auto-start if it stopped due to starvation
    ALint state = 0;
    alGetSourcei(_source, AL_SOURCE_STATE, &state);

    if (state != AL_PLAYING && state != AL_PAUSED) {
        ALint queuedCount = 0;
        alGetSourcei(_source, AL_BUFFERS_QUEUED, &queuedCount);

        // 至少有2个缓冲区才开始播放
        if (queuedCount >= 2) {
            LOG(INFO) << "Starting playback with " << queuedCount << " buffered frames";
            alSourcePlay(_source);
        }
    }
    // std::this_thread::sleep_for(std::chrono::milliseconds(10));
    return true;
}

void AudioStreamProcessor::setEnvironment(int index)
{
    _processor->setEnvironment(static_cast<EnvironmentPreset>(index));
    LOG(INFO) << "Set new Environment is: " << index;
}
