#include "qpushbutton_icons.h"

class QIconPushButtonPrivate
{
public:
    QString m_sIconR;
    QString m_sIconH;
    QString m_sIconP;

    bool m_bIsSvg;
    bool m_bIsSvgSupport;
    double m_dDpi;

    QPushButton* m_pButton;

public:
    QIconPushButtonPrivate(QPushButton* pButton)
    {
        m_pButton = pButton;

        m_bIsSvg = false;
        m_bIsSvgSupport = false;

        m_dDpi = 0.0;
    }

    QString getIconPostfix(QPushButton* p)
    {
        double dDpi = QWidget_GetDPI(p);
        QString sPostfix = m_bIsSvg ? ".svg" : ".png";

        if (fabs(1.0 - dDpi) < 0.1 || fabs(0.0 - dDpi) < 0.1)
            sPostfix = ".png";
        else if (fabs(2.0 - dDpi) < 0.1)
            sPostfix = "-2x.png";

        return sPostfix;
    }

    void checkDpi(const double dDpi, const bool isAttack = false)
    {
        if (!isAttack && fabs(dDpi - m_dDpi) < 0.05)
            return;

        m_dDpi = dDpi;
        if (fabs(0.0 - m_dDpi) > 0.05 && fabs(1.0 - m_dDpi) > 0.05 && fabs(2.0 - m_dDpi) > 0.05)
        {
            if (m_bIsSvgSupport)
            {
                m_bIsSvg = true;
                this->m_pButton->setStyleSheet("QPushButton { background-color: transparent; border: none; margin:0; padding:0; outline:none; }");
                this->setIcon(":/icons/" + m_sIconR + ".svg");
                this->m_pButton->update();
                return;
            }
        }
        m_bIsSvg = false;

        QString sPostfix = ".png";
        if (fabs(2.0 - m_dDpi) < 0.1)
            sPostfix = "-2x.png";

        QString sStyle = "QPushButton {border-image:url(:/icons/" + m_sIconR + sPostfix + ") 0 0 0 0 stretch stretch;margin:0;padding:0;outline:none;}";
        sStyle += "QPushButton::hover {border-image:url(:/icons/" + m_sIconH + sPostfix + ") 0 0 0 0 stretch stretch;margin:0;padding:0;outline:none;}";
        sStyle += "QPushButton::pressed {border-image:url(:/icons/" + m_sIconP + sPostfix + ") 0 0 0 0 stretch stretch;margin:0;padding:0;outline:none;}";

        m_pButton->setStyleSheet(sStyle);
    }

    void setIcon(QString s)
    {
        if (m_bIsSvg)
        {
            QIcon icon;
            QSize size = m_pButton->size();
            icon.addFile(s, size);
            m_pButton->setIcon(icon);
            m_pButton->setIconSize(size);
        }
    }
};

QIconPushButton::QIconPushButton(QWidget *parent, bool bIsSvgSupport, QString sIconR, QString sIconH, QString sIconP) : QPushButton(parent)
{
    m_internal = new QIconPushButtonPrivate(this);
    m_internal->m_bIsSvgSupport = bIsSvgSupport;

    changeIcons(sIconR, sIconH, sIconP);
}

void QIconPushButton::changeIcons(QString sIconR, QString sIconH, QString sIconP)
{
    m_internal->m_sIconR = sIconR;

    if (sIconH.isEmpty() && sIconP.isEmpty())
    {
        m_internal->m_sIconH = m_internal->m_sIconR + "-hover";
        m_internal->m_sIconP = m_internal->m_sIconR + "-active";
    }
    else if (sIconP.isEmpty())
    {
        m_internal->m_sIconH = sIconH;
        m_internal->m_sIconP = m_internal->m_sIconH;
    }
    else
    {
        m_internal->m_sIconH = sIconH;
        m_internal->m_sIconP = sIconP;
    }

    this->m_internal->checkDpi(m_internal->m_dDpi, true);
}

bool QIconPushButton::event(QEvent *e)
{
    if (!m_internal->m_bIsSvg)
        return QPushButton::event(e);

    QPushButton::event(e);
    if (e->type() == QEvent::Enter)
    {
        this->m_internal->setIcon(":/icons/" + m_internal->m_sIconH + ".svg");
    }
    if (e->type() == QEvent::Leave)
    {
        this->m_internal->setIcon(":/icons/" + m_internal->m_sIconR + ".svg");
    }
    return true;
}
void QIconPushButton::resizeEvent(QResizeEvent* e)
{
    QPushButton::resizeEvent(e);
    m_internal->checkDpi(QWidget_GetDPI(this));
}

QIconPushButton::~QIconPushButton()
{
    delete m_internal;
}
