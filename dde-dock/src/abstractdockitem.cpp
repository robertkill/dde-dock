#include <QWidget>
#include <QFrame>
#include <QLabel>
#include <QtDBus>
#include <QJsonObject>
#include <QJsonValue>

#include "abstractdockitem.h"
#include "Widgets/arrowrectangle.h"
#include "DBus/dbusmenu.h"
#include "DBus/dbusmenumanager.h"

AbstractDockItem::AbstractDockItem(QWidget * parent) :
    QFrame(parent)
{

}

AbstractDockItem::~AbstractDockItem()
{

}

QString AbstractDockItem::getTitle()
{
    return "";
}

QWidget * AbstractDockItem::getApplet()
{
    return NULL;
}

bool AbstractDockItem::moveable()
{
    return m_moveable;
}

bool AbstractDockItem::actived()
{
    return m_isActived;
}

void AbstractDockItem::resize(int width,int height){
    QFrame::resize(width,height);

    emit widthChanged();
}

void AbstractDockItem::resize(const QSize &size){
    QFrame::resize(size);

    emit widthChanged();
}

QPoint AbstractDockItem::getNextPos()
{
    return m_itemNextPos;
}

void AbstractDockItem::setNextPos(const QPoint &value)
{
    m_itemNextPos = value;
}

void AbstractDockItem::setNextPos(int x, int y)
{
    m_itemNextPos.setX(x); m_itemNextPos.setY(y);
}

int AbstractDockItem::globalX()
{
    return mapToGlobal(QPoint(0,0)).x();
}

int AbstractDockItem::globalY()
{
    return mapToGlobal(QPoint(0,0)).y();
}

QPoint AbstractDockItem::globalPos()
{
    return mapToGlobal(QPoint(0,0));
}

void AbstractDockItem::showPreview()
{
    if (!m_previewAR->isHidden())
    {
        m_previewAR->resizeWithContent();
        return;
    }
    QWidget *tmpContent = getApplet();
    if (tmpContent == NULL) {
        QString title = getTitle();
        // TODO: memory management
        tmpContent = new QLabel(title);
        tmpContent->setStyleSheet("QLabel { color: white }");
        tmpContent->adjustSize();
    }

    m_previewAR->setArrorDirection(ArrowRectangle::ArrowBottom);
    m_previewAR->setContent(tmpContent);
    m_previewAR->showAtBottom(globalX() + width() / 2,globalY() - 5);
}

void AbstractDockItem::hidePreview(int interval)
{
    m_previewAR->delayHide(interval);
}

void AbstractDockItem::cancelHide()
{
    m_previewAR->cancelHide();
}

void AbstractDockItem::resizePreview()
{
    m_previewAR->resizeWithContent();
    if (!m_previewAR->isHidden())
    {
        m_previewAR->showAtBottom(globalX() + width() / 2,globalY() - 5);
        return;
    }
}

void AbstractDockItem::showMenu()
{
    if (m_dbusMenuManager == NULL) {
        m_dbusMenuManager = new DBusMenuManager(this);
    }

    QDBusPendingReply<QDBusObjectPath> pr = m_dbusMenuManager->RegisterMenu();
    pr.waitForFinished();

    if (pr.isValid()) {
        QDBusObjectPath op = pr.value();

        if (m_dbusMenu != NULL) {
            m_dbusMenu->deleteLater();
        }

        m_dbusMenu = new DBusMenu(op.path(), this);

        connect(m_dbusMenu, &DBusMenu::ItemInvoked, this, &AbstractDockItem::invokeMenuItem);

        QJsonObject targetObj;
        targetObj.insert("x", QJsonValue(globalX() + width() / 2));
        targetObj.insert("y", QJsonValue(globalY() - 5));
        targetObj.insert("isDockMenu", QJsonValue(true));
        targetObj.insert("menuJsonContent", QJsonValue(getMenuContent()));

        m_dbusMenu->ShowMenu(QString(QJsonDocument(targetObj).toJson()));
    }
}

QString AbstractDockItem::getMenuContent()
{
    return "";
}

void AbstractDockItem::invokeMenuItem(QString, bool)
{

}
