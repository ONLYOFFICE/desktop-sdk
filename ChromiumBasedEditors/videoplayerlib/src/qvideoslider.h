#pragma once

#include <QWidget>
#include <QPaintEvent>
#include <QSlider>
#include <QMouseEvent>

class QVideoSlider : public QSlider
{
    Q_OBJECT

    bool m_bIsSeekOnClick;
    double m_dDpi;

public:
    explicit QVideoSlider(QWidget *parent = 0);

    virtual void mousePressEvent(QMouseEvent* e);
    virtual void resizeEvent(QResizeEvent* e);

public:
    void SetSeekOnClick(bool bValue);
};
