// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SNITRAYWIDGET_H
#define SNITRAYWIDGET_H

#include "constants.h"
#include "abstracttraywidget.h"
#include "util/dockpopupwindow.h"

#include <org_kde_statusnotifieritem.h>

#include <QMenu>
#include <QDBusObjectPath>
DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE
class DBusMenuImporter;
namespace Dock {
class TipsWidget;
}
//using namespace com::deepin::dde;
using namespace org::kde;

/**
 * @brief The SNITrayWidget class
 * @note 系统托盘第三方程序窗口
 */
class SNITrayWidget : public AbstractTrayWidget
{
    Q_OBJECT

public:
    enum ItemCategory {UnknownCategory = -1, ApplicationStatus, Communications, SystemServices, Hardware};
    enum ItemStatus {Passive, Active, NeedsAttention};
    enum IconType {UnknownIconType = -1, Icon, OverlayIcon, AttentionIcon, AttentionMovieIcon};

public:
    SNITrayWidget(const QString &sniServicePath, QWidget *parent = Q_NULLPTR);

    QString itemKeyForConfig() override;
    void updateIcon() override;
    void sendClick(uint8_t mouseButton, int x, int y) override;

    bool isValid() override;
    SNITrayWidget::ItemStatus status();
    SNITrayWidget::ItemCategory category();

    static QString toSNIKey(const QString &sniServicePath);
    static bool isSNIKey(const QString &itemKey);
    static QPair<QString, QString> serviceAndPath(const QString &servicePath);
    static uint servicePID(const QString &servicePath);

    void showHoverTips();
    const QPoint topleftPoint() const;
    void showPopupWindow(QWidget *const content, const bool model = false);
    const QPoint popupMarkPoint() const;

    static void setDockPostion(const Dock::Position pos) { DockPosition = pos; }

Q_SIGNALS:
    void statusChanged(SNITrayWidget::ItemStatus status);

private Q_SLOTS:
    void initSNIPropertys();
    void initMenu();
    void refreshIcon();
    void refreshOverlayIcon();
    void refreshAttentionIcon();
    void showContextMenu(int x, int y);
    // SNI property change slot
    void onSNIAttentionIconNameChanged(const QString &value);
    void onSNIAttentionIconPixmapChanged(DBusImageList  value);
    void onSNIAttentionMovieNameChanged(const QString &value);
    void onSNICategoryChanged(const QString &value);
    void onSNIIconNameChanged(const QString &value);
    void onSNIIconPixmapChanged(DBusImageList  value);
    void onSNIIconThemePathChanged(const QString &value);
    void onSNIIdChanged(const QString &value);
    void onSNIMenuChanged(const QDBusObjectPath &value);
    void onSNIOverlayIconNameChanged(const QString &value);
    void onSNIOverlayIconPixmapChanged(DBusImageList  value);
    void onSNIStatusChanged(const QString &status);
    void hidePopup();
    void hideNonModel();
    void popupWindowAccept();
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *e) override;

private:
    void paintEvent(QPaintEvent *e) override;
    QPixmap newIconPixmap(IconType iconType);
    void setMouseData(QMouseEvent *e);
    void handleMouseRelease();

private:
    StatusNotifierItem *m_sniInter;

    DBusMenuImporter *m_dbusMenuImporter;

    QMenu *m_menu;
    QTimer *m_updateIconTimer;
    QTimer *m_updateOverlayIconTimer;
    QTimer *m_updateAttentionIconTimer;

    QString m_sniServicePath;
    QString m_dbusService;
    QString m_dbusPath;

    QPixmap m_pixmap;
    QPixmap m_overlayPixmap;

    // SNI propertys
    QString m_sniAttentionIconName;
    DBusImageList m_sniAttentionIconPixmap;
    QString m_sniAttentionMovieName;
    QString m_sniCategory;
    QString m_sniIconName;
    DBusImageList m_sniIconPixmap;
    QString m_sniIconThemePath;
    QString m_sniId;
    QDBusObjectPath m_sniMenuPath;
    QString m_sniOverlayIconName;
    DBusImageList m_sniOverlayIconPixmap;
    QString m_sniStatus;
    QTimer *m_popupTipsDelayTimer;
    QTimer *m_handleMouseReleaseTimer;
    QPair<QPoint, Qt::MouseButton> m_lastMouseReleaseData;
    static Dock::Position DockPosition;
    static QPointer<DockPopupWindow> PopupWindow;
    Dock::TipsWidget *m_tipsLabel;
    bool m_popupShown;
};

#endif /* SNIWIDGET_H */
