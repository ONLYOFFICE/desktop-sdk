#ifndef Q_FOOTER_PANEL_PRIVATE_H
#define Q_FOOTER_PANEL_PRIVATE_H

#include <QWidget>

#include "qiconpushbutton.h"
#include "qvideoslider.h"
#include "qtimelabel.h"
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
	QIconPushButton*	m_pRewindBack;
	QIconPushButton*	m_pRewindForward;
	QVideoSlider*		m_pSlider;
	QTimeLabel*			m_pTimeLabel;


	// volume control elements
	QWidget*		m_pVolumeControl;
	QVideoSlider*	m_pVolumeControlV;

	// style options
	CFooterStyleOptions	m_oStyleOpt;

	// flags
	bool m_bIsEnabledPlayList;
	bool m_bIsEnabledFullscreen;
	bool m_bIsRoundedCorners;

	// constants
	const int c_nHeight = 40;
	const int c_nMinWidth = 320;
	const int c_nMaxWidth = 1000;

	const int c_nButtonsWidth = 30;
	const int c_nButtonsY = 5;
	const int c_nButtonsBetween = 8;

	const int c_nVolumeControlWidth = 30;
	const int c_nVolumeControlHeight = 140;

	const int c_nVolumeSliderWidth = 20;
	const int c_nVolumeSliderHeight = 120;

	const int c_nTimeLabelWidth = 60;

	const int c_nBorderRadius = 5;
};

#endif // Q_FOOTER_PANEL_PRIVATE_H
