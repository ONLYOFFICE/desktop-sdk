#pragma once

#include <QWidget>
#include <QMediaPlayer>

#ifdef QT_VERSION_6
#include <QAudioOutput>

typedef QMediaPlayer::PlaybackState QMediaPlayer_State;
typedef QUrl QMediaContent;

static void QMediaPlayer_setMedia(QMediaPlayer* pPlayer, const QString& sFile)
{
    pPlayer->setSource(QUrl::fromLocalFile(sFile));
}
static void QMediaPlayer_setVolume(QMediaPlayer* pPlayer, const int& nVolume)
{
    if (pPlayer->audioOutput())
        pPlayer->audioOutput()->setVolume(nVolume);
}
static bool QMediaPlayer_isAudio(QMediaPlayer* pPlayer)
{
    if (pPlayer->hasVideo())
        return false;
    return true;
}
static bool QMediaContent_isEmpty(QMediaContent& content)
{
    return content.isEmpty();
}
#else
typedef QMediaPlayer::State QMediaPlayer_State;

static void QMediaPlayer_setMedia(QMediaPlayer* pPlayer, const QString& sFile)
{
    if (sFile.isEmpty())
        pPlayer->setMedia(QMediaContent());
    else
        pPlayer->setMedia(QMediaContent(QUrl::fromLocalFile(sFile)));
}
static void QMediaPlayer_setVolume(QMediaPlayer* pPlayer, const int& nVolume)
{
    pPlayer->setVolume(nVolume);
}
static bool QMediaPlayer_isAudio(QMediaPlayer* pPlayer)
{
    if (pPlayer->isVideoAvailable())
        return false;
    return true;
}
static bool QMediaContent_isEmpty(QMediaContent& content)
{
    return content.isNull();
}
#endif
