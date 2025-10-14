#pragma once

#ifndef USE_VLC_LIBRARY

#include <QVideoWidget>
#include <QMediaPlayer>
#include "./qascmediaplayer.h"

static QMediaPlayer_State getPlayerState(QMediaPlayer* player)
{
#ifdef QT_VERSION_6
	return player->playbackState();
#else
	return player->state();
#endif
}

static bool isVideoAvailable(QMediaPlayer* player)
{
#ifdef QT_VERSION_6
	return player->videoOutput() != nullptr;
#else
	return player->isVideoAvailable();
#endif
}

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
