#pragma once
#include "Singleton.h"

enum class PlayerState {
	STOP = -1,
	PLAYING,
	PAUSE
};

class PlayStatePrivate : public Singleton<PlayStatePrivate>
{ 
	friend class Singleton<PlayStatePrivate>;
public:
	~PlayStatePrivate() = default;

	void setState(PlayerState new_state) {
		std::lock_guard<std::mutex> lock(_mutex);
        _state = new_state;
	}

	PlayerState getState() {
		std::lock_guard<std::mutex> lock(_mutex);
        return _state;
	}

private:
    PlayStatePrivate() = default;
	PlayerState _state = PlayerState::STOP;
	std::mutex _mutex;
};