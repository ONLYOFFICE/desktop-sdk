#ifndef Q_FOOTER_PANEL_PRIVATE_H
#define Q_FOOTER_PANEL_PRIVATE_H

#include <QWidget>
#include <QResizeEvent>
#include <QPaintEvent>

#include "qiconpushbutton.h"
#include "qvideoslider.h"
#include "style/style_options.h"

class QAscVideoView;
class QFooterPanel;
class QFooterPanel_Private
{
public:
	QAscVideoView*	m_pView;

	// footer elements
	QIconPushButton*	m_pPlayPause;
	QIconPushButton*	m_pVolume;
	QIconPushButton*	m_pFullscreen;
	QIconPushButton*	m_pPlaylist;
	QVideoSlider*		m_pSlider;

	// volume control elements
	QWidget*		m_pVolumeControl;
	QVideoSlider*	m_pVolumeControlV;

	// style options
	CFooterStyleOptions	m_oStyleOpt;

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

	const int c_nVolumeControlWidth = 30;
	const int c_nVolumeControlHeight = 140;

	const int c_nVolumeSliderWidth = 20;
	const int c_nVolumeSliderHeight = 120;
};

#endif // Q_FOOTER_PANEL_PRIVATE_H
