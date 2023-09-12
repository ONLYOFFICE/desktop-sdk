#pragma once

#ifndef USE_VLC_LIBRARY

#include <QVideoWidget>
#include <QMediaPlayer>
#include "./qascmediaplayer.h"

#else

namespace QMediaPlayer
{
	enum State
	{
		StoppedState,
		PlayingState,
		PausedState
	};
}

typedef QMediaPlayer::State QMediaPlayer_State;

#endif
