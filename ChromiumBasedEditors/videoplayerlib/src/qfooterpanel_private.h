#ifndef Q_FOOTER_PANEL_PRIVATE_H
#define Q_FOOTER_PANEL_PRIVATE_H

#include <QWidget>
#include <QPushButton>
#include <QResizeEvent>
#include <QPaintEvent>

#include "qvideoslider.h"

class QAscVideoView;
class QFooterPanel;
class QFooterPanel_Private
{
public:
	QAscVideoView*	m_pView;

	// footer elements
	QPushButton*	m_pPlayPause;
	QPushButton*	m_pVolume;
	QPushButton*	m_pFullscreen;
	QPushButton*	m_pPlaylist;
	QVideoSlider*	m_pSlider;

	// volume control elements
	QWidget*		m_pVolumeControl;
	QVideoSlider*	m_pVolumeControlV;

	// flags
	bool m_bIsEnabledPlayList;
	bool m_bIsEnabledFullscreen;

	// constants
	const int c_nHeight = 40;
	const int c_nMinWidth = 320;
	const int c_nMaxWidth = 1000;

	const int c_nButtonsWidth = 30;
	const int c_nButtonsY = 5;
	const int c_nButtonsBetween = 10;

	const int c_nVolumeControlWidth = 60;
	const int c_nVolumeControlHeight = 160;

	const int c_nVolumeSliderX = 15;
	const int c_nVolumeSliderY = 20;
	const int c_nVolumeSliderWidth = 30;
	const int c_nVolumeSliderHeight = 120;
};

#endif // Q_FOOTER_PANEL_PRIVATE_H
