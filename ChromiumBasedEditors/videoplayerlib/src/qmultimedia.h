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

	enum MediaStatus
	{
		UnknownMediaStatus,
		NoMedia,
		LoadingMedia,
		LoadedMedia,
		StalledMedia,
		BufferingMedia,
		BufferedMedia,
		EndOfMedia,
		InvalidMedia
	};
}

typedef QMediaPlayer::State QMediaPlayer_State;

#endif
