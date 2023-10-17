#include "qvideoplaylist.h"
#include <QStyleOption>
#include <QPainter>
#include <QStandardItemModel>
#include <QScrollBar>
#include <QHeaderView>
#include <QAbstractItemView>
#include <QAbstractItemModel>
#include <QFileDialog>
#include <QStandardPaths>

#include <QShortcut>
#include <QDebug>
#include "../../../../core/DesktopEditor/common/Directory.h"
#include "../../../../core/DesktopEditor/common/StringBuilder.h"
#include "../../../../core/DesktopEditor/xml/include/xmlutils.h"
#include "qpushbutton_icons.h"
#include "../qascvideoview.h"

QStandardItem* CreateFileItem(const QString& text, const QString& file)
{
	QStandardItem* item = new QStandardItem(text);
	item->setAccessibleText(file);
	item->setDropEnabled(false);
	item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	return item;
}

QStandardItem* CreateFileDurationItem(const QString& text)
{
	QStandardItem* item = new QStandardItem(text);
	item->setDropEnabled(false);
	item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
	return item;
}

QVideoPlaylist::QVideoPlaylist(QWidget *parent) : QWidget(parent)
{
	m_oCS.InitializeCriticalSection();

	m_pDialogParent = NULL;

	m_sSavePlayListAddon = "/ONLYOFFICE/VideoPlayer";
	QWidget_setBackground(this, 0x22, 0x22, 0x22);

	m_pAdd = new QIconPushButton(this, true, "add-files", "add-files-active");
	m_pClear = new QIconPushButton(this, true, "drop-playlist", "drop-playlist-active");

	m_pListView = new QTableView(this);
	QStandardItemModel* pModel = new QStandardItemModel(this);
	m_pListView->setModel(pModel);

	pModel->setColumnCount(2);
	m_pListView->setColumnWidth(0, 150);
	m_pListView->setColumnWidth(1, 50);

	QHeaderView* pVerticalHeader = m_pListView->verticalHeader();
	QHeaderView* pHorizontalHeader = m_pListView->horizontalHeader();

	pHorizontalHeader->setHidden(true);
	pHorizontalHeader->setStretchLastSection(true);

	pVerticalHeader->setHidden(true);

	m_pListView->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_pListView->setSelectionMode(QAbstractItemView::SingleSelection);
	m_pListView->setDragDropMode(QAbstractItemView::DropOnly);
	m_pListView->setDefaultDropAction(Qt::IgnoreAction);
	m_pListView->setDragDropOverwriteMode(false);
	m_pListView->setDragEnabled(true);
	m_pListView->setDropIndicatorShown(false);

	QObject::connect(m_pAdd, SIGNAL(clicked(bool)), this, SLOT(slotButtonAdd()));

	QObject::connect(m_pClear, SIGNAL(clicked(bool)), this, SLOT(slotButtonClear()));

	QObject::connect(m_pListView, SIGNAL(clicked(const QModelIndex &)),
					 this, SLOT(slotClick(const QModelIndex &)));

	QObject::connect(m_pListView, SIGNAL(activated(const QModelIndex &)),
					 this, SLOT(slotActivated(const QModelIndex &)));

	m_pListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_pListView->setShowGrid(false);

	m_dDpi = 0;
	private_Style(1.0);

#ifdef _MAC
	QShortcut* shortcut1 = new QShortcut(QKeySequence(Qt::Key_Return), m_pListView);
	connect(shortcut1, SIGNAL(activated()), this, SLOT(slotActivatedShortcut()));

	QShortcut* shortcut2 = new QShortcut(QKeySequence(Qt::Key_Enter), m_pListView);
	connect(shortcut2, SIGNAL(activated()), this, SLOT(slotActivatedShortcut()));
#endif

	QShortcut* shortcutDelete = new QShortcut(QKeySequence(QKeySequence::Delete), m_pListView);
	connect(shortcutDelete, SIGNAL(activated()), this, SLOT(slotDeletedShortcut()));

	QObject::connect(this, SIGNAL(_onCheckDuration(QString,QString)), this, SLOT(_onSlotCheckDuration(QString,QString)), Qt::QueuedConnection);

	QObject::connect(&m_oTimer, SIGNAL(timeout()), this, SLOT(_onThreadFunc()));
	m_oTimer.start(100);

#ifndef USE_VLC_LIBRARY
	m_pCheckPlayer = new QMediaPlayer();
	QObject::connect(m_pCheckPlayer, SIGNAL(durationChanged(qint64)), this, SLOT(_onSlotDurationChanged(qint64)));
	QObject::connect(m_pCheckPlayer, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(_onSlotMediaStatusChanged(QMediaPlayer::MediaStatus)));

#ifndef QT_VERSION_6
	QObject::connect(m_pCheckPlayer, SIGNAL(mediaChanged(QMediaContent)), this, SLOT(_onSlotMediaChanged(QMediaContent)));
	QObject::connect(m_pCheckPlayer, SIGNAL(error(QMediaPlayer::Error)), this, SLOT(_onSlotError(QMediaPlayer::Error)));
#else
	QObject::connect(m_pCheckPlayer, SIGNAL(sourceChanged(QMediaContent)), this, SLOT(_onSlotMediaChanged(QMediaContent)));
	//QObject::connect(m_pCheckPlayer, SIGNAL(error(QMediaPlayer::Error)), this, SLOT(_onSlotError(QMediaPlayer::Error)));
#endif

#endif
}

QVideoPlaylist::~QVideoPlaylist()
{
	m_oCS.DeleteCriticalSection();
}

void QVideoPlaylist::private_Style(double dDpi)
{
	if (fabs(dDpi - m_dDpi) < 0.1)
		return;
	m_dDpi = dDpi;

	QScrollBar* pScrollBar = m_pListView->verticalScrollBar();

	int n1 = (int)(dDpi * 1);
	int n2 = (int)(dDpi * 2);
	int n4 = (int)(dDpi * 4);
	int n5 = (int)(dDpi * 5);
	int n6 = (int)(dDpi * 6);
	int n10 = (int)(dDpi * 10);
	int n20 = (int)(dDpi * 20);

	QString sStyle = QString("\
							 QScrollBar:vertical {\
								 border: none;\
								 background: transparent;\
								 width: %6px;\
								 margin-left: 0px;\
								 margin-top: %4px;\
								 margin-right: %2px;\
								 margin-bottom: %4px;\
							 }\
							 QScrollBar::handle:vertical {\
								 background: #545454;\
								 min-height: %7px;\
								 border: %1px solid #545454;\
								 width: %5px;\
								 border-radius: %3px;\
							 }\
							 QScrollBar::handle:vertical:hover {\
								 background: #9B9B9B;\
								 min-height: %7px;\
								 border: %1px solid #9B9B9B;\
								 width: %5px;\
								 border-radius: %3px;\
							 }\
							 QScrollBar::add-line:vertical {\
								 background: transparent;\
								 height: 0px;\
							 }\
							 QScrollBar::sub-line:vertical {\
								 background: transparent;\
								 height: 0px;\
							 }\
							 QScrollBar::up-arrow:vertical, QScrollBar::down-arrow:vertical {\
								 height: %6px;\
								 background: transparent;\
							 }\
							 QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {\
								 background: none;\
								 height: %6px;\
							 }").arg(QString::number(n1), QString::number(n2), QString::number(n4), QString::number(n5), QString::number(n6), QString::number(n10), QString::number(n20));
							 pScrollBar->setStyleSheet(sStyle);

			QString sStyleL = QString("\
									  QTableView {\
										  background: transparent;\
										  border: none;\
										  color: #BBBBBB;\
										  margin-left: %2px;\
										  margin-top: %2px;\
										  margin-right: %1px;\
										  margin-bottom: %2px;\
										  border: none;\
										  outline: none;\
										  selection-background-color: transparent;\
										  selection-color: #FFFFFF; \
										  vertical-align:middle;\
										  font-family: 'Open Sans'; font-style: normal; font-size: %3px;\
									  }\
									  QTableView::item { height: 20px; border: none; outline: none; }\
									  QTableView::item:selected { border: none; outline: none; background-color: #353535; }\
									  ").arg(QString::number(n5), QString::number(n10), QString::number((int)(dDpi * 12)));
									  m_pListView->setStyleSheet(sStyleL);

			m_pListView->setColumnWidth(0, QWidget_ScaleDPI(150, m_dDpi));
	m_pListView->setColumnWidth(1, QWidget_ScaleDPI(50, m_dDpi));

	m_pListView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	m_pListView->verticalHeader()->setDefaultSectionSize(QWidget_ScaleDPI(30, m_dDpi));

	CheckStyles();
}

void QVideoPlaylist::resizeEvent(QResizeEvent* e)
{
	double dDpi = QWidget_GetDPI(this);

	int nButW = QWidget_ScaleDPI(30, dDpi);
	int n5 = QWidget_ScaleDPI(5, dDpi);
	int n10 = QWidget_ScaleDPI(10, dDpi);

	int nTopB = height() - nButW;

	m_pListView->setGeometry(0, 0, width(), nTopB - n10);
	m_pAdd->setGeometry(n10, nTopB - n5, nButW, nButW);
	m_pClear->setGeometry(width() - nButW - n10, nTopB - n5, nButW, nButW);

	private_Style(dDpi);
}

void QVideoPlaylist::slotClick(const QModelIndex &index)
{

}


void QVideoPlaylist::slotActivated(const QModelIndex &index)
{
	QStandardItemModel* pModel = (QStandardItemModel*)m_pListView->model();
	int nRows = pModel->rowCount();
	int nCurrentRow = index.row();

	QFont fontAll("Open Sans", 10, QFont::Normal, false);
	QFont fontSelected("Open Sans", 10, QFont::Bold, false);
	fontAll.setUnderline(false);
	fontAll.setStrikeOut(false);
	fontSelected.setUnderline(false);
	fontSelected.setStrikeOut(false);
	fontAll.setPixelSize(QWidget_ScaleDPI(12, m_dDpi));
	fontSelected.setPixelSize(QWidget_ScaleDPI(12, m_dDpi));

	QBrush brushAll(QColor(0xBB, 0xBB, 0xBB));
	QBrush brushSelected(QColor(0xFF, 0xFF, 0xFF));

	for (int i = 0; i < nRows; i++)
	{
		QStandardItem* item1 = pModel->item(i, 0);
		QStandardItem* item2 = pModel->item(i, 1);

		if (i != nCurrentRow)
		{
			item1->setFont(fontAll);
			item1->setForeground(brushAll);

			item2->setFont(fontAll);
			item2->setForeground(brushAll);
		}
		else
		{
			item1->setFont(fontSelected);
			item1->setForeground(brushSelected);

			item2->setFont(fontSelected);
			item2->setForeground(brushSelected);

			emit fileChanged(item1->accessibleText());
		}
	}
}

void QVideoPlaylist::CheckStyles()
{
	QStandardItemModel* pModel = (QStandardItemModel*)m_pListView->model();
	int nRows = pModel->rowCount();

	int nRow = -1;
	for (int i = 0; i < nRows; i++)
	{
		QStandardItem* item1 = pModel->item(i, 0);

		if (item1->font().bold())
		{
			nRow = i;
			break;
		}
	}

	int nCurrentRow = nRow;

	QFont fontAll("Open Sans", 10, QFont::Normal, false);
	QFont fontSelected("Open Sans", 10, QFont::Bold, false);
	fontAll.setUnderline(false);
	fontAll.setStrikeOut(false);
	fontSelected.setUnderline(false);
	fontSelected.setStrikeOut(false);
	fontAll.setPixelSize(QWidget_ScaleDPI(12, m_dDpi));
	fontSelected.setPixelSize(QWidget_ScaleDPI(12, m_dDpi));

	QBrush brushAll(QColor(0xBB, 0xBB, 0xBB));
	QBrush brushSelected(QColor(0xFF, 0xFF, 0xFF));

	for (int i = 0; i < nRows; i++)
	{
		QStandardItem* item1 = pModel->item(i, 0);
		QStandardItem* item2 = pModel->item(i, 1);

		if (i != nCurrentRow)
		{
			item1->setFont(fontAll);
			item1->setForeground(brushAll);

			item2->setFont(fontAll);
			item2->setForeground(brushAll);
		}
		else
		{
			item1->setFont(fontSelected);
			item1->setForeground(brushSelected);

			item2->setFont(fontSelected);
			item2->setForeground(brushSelected);
		}
	}
}

bool QVideoPlaylist::isScrollBarVisible()
{
	return m_pListView->verticalScrollBar()->isVisible();
}

#ifdef _MAC
void QVideoPlaylist::slotActivatedShortcut()
{
	this->slotActivated(m_pListView->selectionModel()->currentIndex());
}
#endif

void QVideoPlaylist::slotDeletedShortcut()
{
	this->m_pListView->model()->removeRow(m_pListView->selectionModel()->currentIndex().row());
}

void QVideoPlaylist::slotButtonAdd()
{
	QString sDir = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);
	if (!m_sLastFileDialogFolder.isEmpty())
		sDir = m_sLastFileDialogFolder;

	QString sTitle = QString::fromUtf8("Добавить файлы в список воспроизведения");
	//sTitle = "Add files to playlist";
	CVideoExt ext;
	QStringList filenames = QFileDialog::getOpenFileNames(m_pDialogParent ? m_pDialogParent : this,
														  sTitle, sDir,
														  ext.getFilter(), NULL,
													  #ifdef QFILEDIALOG_DONTUSENATIVEDIALOG
														  QFileDialog::DontUseNativeDialog
													  #else
														  QFileDialog::Option()
													  #endif
														  );

	AddFiles(filenames);
	this->setFocus();
}

void QVideoPlaylist::slotButtonClear()
{
	QStandardItemModel* pModel = (QStandardItemModel*)m_pListView->model();
	pModel->clear();
}

void QVideoPlaylist::AddFiles(QStringList& filenames, const bool isStart)
{
	int nPrevCount = m_pListView->model()->rowCount();
	if (filenames.isEmpty())
		return;

	QStandardItemModel* pModel = (QStandardItemModel*)m_pListView->model();

	QStringList listCheck;

	QList<QStandardItem*> rowData;
	for (int i = 0; i < filenames.count(); ++i)
	{
		rowData.clear();

		QFileInfo file(filenames.at(i));

		if (0 == i)
			m_sLastFileDialogFolder = file.absolutePath();

		QStandardItem* item = CreateFileItem(file.fileName(), file.absoluteFilePath());
		rowData << item;

		listCheck.append(file.absoluteFilePath());

		item = CreateFileDurationItem("...");
		rowData << item;

		pModel->appendRow(rowData);
	}

	m_pListView->setColumnWidth(0, QWidget_ScaleDPI(150, m_dDpi));
	m_pListView->setColumnWidth(1, QWidget_ScaleDPI(50, m_dDpi));

	AddFilesToCheck(listCheck);

	if (isStart)
	{
		m_pListView->setCurrentIndex(pModel->index(nPrevCount, 0));
		m_pListView->activated(pModel->index(nPrevCount, 0));
	}
}

void QVideoPlaylist::Load()
{
	QString sSettings = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
	sSettings += m_sSavePlayListAddon;

#ifdef WIN32
	sSettings = sSettings.replace('/', '\\');
#endif

	std::wstring sDir = sSettings.toStdWString();
	NSDirectory::CreateDirectories(sDir);

	XmlUtils::CXmlNode oNode;
	if (!oNode.FromXmlFile(sDir + L"/settings.xml"))
		return;

	std::vector<XmlUtils::CXmlNode> arNodesFile = oNode.GetNodes(L"file");
	if (!arNodesFile.empty())
	{
		QStringList list;

		for (size_t i = 0, len = arNodesFile.size(); i < len; i++)
		{
			list.append(QString::fromStdWString(arNodesFile[i].GetText()));
		}

		this->AddFiles(list);
	}

	int nIndex = oNode.ReadValueInt(L"index", -1);
	if (-1 != nIndex)
	{
		QModelIndex index = m_pListView->model()->index(nIndex, 0);
		m_pListView->setCurrentIndex(index);
	}

	std::wstring sLastFolder = oNode.ReadValueString(L"lastdirectory");
	if (!sLastFolder.empty())
		m_sLastFileDialogFolder = QString::fromStdWString(sLastFolder);
}

void QVideoPlaylist::Save()
{
	m_oCS.DeleteCriticalSection();

	QString sSettings = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
	sSettings += m_sSavePlayListAddon;

	std::wstring sDir = sSettings.toStdWString();
	NSDirectory::CreateDirectories(sDir);

	std::wstring sFile = sDir + L"/settings.xml";

	NSStringUtils::CStringBuilder builder;
	builder.WriteString(L"<settings>");

	QStandardItemModel* pModel = (QStandardItemModel*)m_pListView->model();
	int nRows = pModel->rowCount();
	int nCurrentRow = m_pListView->currentIndex().row();

	for (int i = 0; i < nRows; i++)
	{
		QStandardItem* item = pModel->item(i, 0);

		builder.WriteString(L"<file>");
		builder.WriteEncodeXmlString(item->accessibleText().toStdWString());
		builder.WriteString(L"</file>");
	}

	builder.WriteString(L"<index>");
	builder.AddInt(nCurrentRow);
	builder.WriteString(L"</index>");

	builder.WriteString(L"<lastdirectory>");
	builder.WriteEncodeXmlString(m_sLastFileDialogFolder.toStdWString());
	builder.WriteString(L"</lastdirectory>");

	builder.WriteString(L"</settings>");

	std::wstring sXml = builder.GetData();
	NSFile::CFileBinary::SaveToFile(sFile, sXml);
}

// check duration methods
void QVideoPlaylist::AddFilesToCheck(QStringList& list)
{
	CTemporaryCS oCS(&m_oCS);

	for (int i = 0; i < list.count(); ++i)
	{
		if (!m_mapChecked.contains(list[i].toLower()))
			m_mapChecked.insert(list[i], true);
	}
}

void QVideoPlaylist::_onSlotCheckDuration(const QString& file, const QString& duration)
{
	QStandardItemModel* pModel = (QStandardItemModel*)m_pListView->model();
	int nRows = pModel->rowCount();

	for (int i = 0; i < nRows; i++)
	{
		QStandardItem* item = pModel->item(i, 0);
		if (item->accessibleText() == file)
			pModel->item(i, 1)->setText(duration);
	}
}

#ifndef USE_VLC_LIBRARY
void QVideoPlaylist::_onSlotError(QMediaPlayer::Error error)
{

}

void QVideoPlaylist::_onSlotMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
	if (status == QMediaPlayer::InvalidMedia)
		_onSlotDurationChanged(0);
}

void QVideoPlaylist::_onSlotDurationChanged(qint64 duration)
{
	if (m_sCheckFile.isEmpty())
		return;

	qint64 nH = (qint64)(duration / 3600000);
	duration -= (nH * 3600000);
	qint64 nM = (qint64)(duration / 60000);
	duration -= (nM * 60000);
	qint64 nS = (qint64)(duration / 1000);

	QString sDur = "";
	if (nH > 0)
	{
		sDur += QString::number((uint)nH);
		sDur += ":";
	}
	if (nM < 10)
		sDur += "0";
	sDur += QString::number((uint)nM);
	sDur += ":";
	if (nS < 10)
		sDur += "0";
	sDur += QString::number((uint)nS);

	emit _onCheckDuration(m_sCheckFile, sDur);

	m_pCheckPlayer->stop();
	QMediaPlayer_setMedia(m_pCheckPlayer, "");
}

void QVideoPlaylist::_onSlotMediaChanged(QMediaContent content)
{
	if (QMediaContent_isEmpty(content))
	{
		m_sCheckFile = "";
		return;
	}
}
#else
void QVideoPlaylist::_onVlcMediaParseChanged(bool isparsed)
{
	if (m_sCheckFile.isEmpty())
		return;

	qint64 duration = m_pCheckMedia->duration();

	QString sCheckFile = m_sCheckFile;

	m_sCheckFile = "";
	delete m_pCheckMedia;

	qint64 nH = (qint64)(duration / 3600000);
	duration -= (nH * 3600000);
	qint64 nM = (qint64)(duration / 60000);
	duration -= (nM * 60000);
	qint64 nS = (qint64)(duration / 1000);

	QString sDur = "";
	if (nH > 0)
	{
		sDur += QString::number((uint)nH);
		sDur += ":";
	}
	if (nM < 10)
		sDur += "0";
	sDur += QString::number((uint)nM);
	sDur += ":";
	if (nS < 10)
		sDur += "0";
	sDur += QString::number((uint)nS);

	emit _onCheckDuration(sCheckFile, sDur);
}
#endif

void QVideoPlaylist::_onThreadFunc()
{
	if (!m_sCheckFile.isEmpty())
		return;

	m_oCS.Enter();

	if (!m_mapChecked.isEmpty())
	{
		m_sCheckFile = m_mapChecked.firstKey();
		m_mapChecked.remove(m_sCheckFile);
	}
	else
	{
		m_sCheckFile = "";
		m_oCS.Leave();

		return;
	}

	m_oCS.Leave();

#ifndef USE_VLC_LIBRARY
	QMediaPlayer_setMedia(m_pCheckPlayer, m_sCheckFile);
#else
	m_pCheckMedia = new CVlcMedia(GetVlcInstance(), m_sCheckFile);
	QObject::connect(m_pCheckMedia, SIGNAL(parsedChanged(bool)), this, SLOT(_onVlcMediaParseChanged(bool)));
	m_pCheckMedia->parse();
#endif
	return;
}

void QVideoPlaylist::PlayCurrent()
{
	QStandardItemModel* pModel = (QStandardItemModel*)m_pListView->model();
	int nRows = pModel->rowCount();
	int nRow = -1;

	if (0 == nRows)
	{
		// файлов нет - показыаем файл диалог
		slotButtonAdd();
		nRows = pModel->rowCount();
	}
	if (0 == nRows)
		return;

	for (int i = 0; i < nRows; i++)
	{
		QStandardItem* item1 = pModel->item(i, 0);

		if (item1->font().bold())
		{
			nRow = i;
			break;
		}
	}

	if (-1 == nRow)
		nRow = m_pListView->selectionModel()->currentIndex().row();

	if (-1 == nRow)
		nRow = 0;

	m_pListView->setCurrentIndex(pModel->index(nRow, 0));
	m_pListView->activated(pModel->index(nRow, 0));
}

void QVideoPlaylist::Next()
{
	QStandardItemModel* pModel = (QStandardItemModel*)m_pListView->model();
	int nRows = pModel->rowCount();
	int nRow = -1;

	for (int i = 0; i < nRows; i++)
	{
		QStandardItem* item1 = pModel->item(i, 0);

		if (item1->font().bold())
		{
			nRow = i;
			break;
		}
	}

	if (nRow >= (nRows - 1))
	{
		emit fileChanged("");
		return;
	}

	++nRow;

	m_pListView->setCurrentIndex(pModel->index(nRow, 0));
	m_pListView->activated(pModel->index(nRow, 0));
	//m_pListView->selectionModel()->select(pModel->index(nRow, 0), QItemSelectionModel::Toggle | QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
}

void QVideoPlaylist::Prev()
{
	QStandardItemModel* pModel = (QStandardItemModel*)m_pListView->model();
	int nRows = pModel->rowCount();
	int nRow = -1;

	for (int i = 0; i < nRows; i++)
	{
		QStandardItem* item1 = pModel->item(i, 0);

		if (item1->font().bold())
		{
			nRow = i;
			break;
		}
	}

	if (nRow == 0)
		return;

	--nRow;

	m_pListView->setCurrentIndex(pModel->index(nRow, 0));
	m_pListView->activated(pModel->index(nRow, 0));
}
