#pragma once

#include "AudioProcessor.h"
#include <vector>

class AudioStreamProcessor
{
public:
	AudioStreamProcessor();
	~AudioStreamProcessor();

	bool initialize();

	void createOpenALResources();

	void setVolume(float volume);

	bool hasBuffer();		// whether has unoccupied buffer
	void start();
	void stop();

	bool write(uint8_t* data, int size);

	void setEnvironment(int index);
	void setEnvDepthValue(int value);
    void setEnvIntensityValue(int value);

private:
	ALuint _source = 0;
	std::vector<ALuint> _buffers;
	std::unique_ptr<AudioProcessor> _processor;
};

