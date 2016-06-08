/**
 * Copyright (C) 2014 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

package dock

import (
	"dbus/com/deepin/daemon/display"
	"errors"
	"fmt"
	"gir/gio-2.0"
	"github.com/BurntSushi/xgb/xproto"
	"github.com/BurntSushi/xgbutil/ewmh"
	"github.com/BurntSushi/xgbutil/xrect"
	"pkg.deepin.io/lib/dbus"
	"pkg.deepin.io/lib/dbus/property"
)

type DockManager struct {
	hideStateManager *HideStateManager
	dockProperty     *DockProperty
	dockedAppManager *DockedAppManager

	clientList                     windowSlice
	appIdFilterGroup               *AppIdFilterGroup
	desktopWindowsMapCacheManager  *desktopWindowsMapCacheManager
	desktopHashFileMapCacheManager *desktopHashFileMapCacheManager

	Entries []*AppEntry

	hideMode    HideModeType
	displayMode DisplayModeType

	settings    *gio.Settings
	HideMode    *property.GSettingsEnumProperty `access:"readwrite"`
	DisplayMode *property.GSettingsEnumProperty `access:"readwrite"`
	Position    *property.GSettingsEnumProperty `access:"readwrite"`

	ActiveWindow       xproto.Window
	dpy                *display.Display
	displayPrimaryRect *xrect.XRect
	dockRect           *xrect.XRect

	dockHeight int
	dockWidth  int
	entryCount uint
	// Signals
	EntryAdded   func(dbus.ObjectPath)
	EntryRemoved func(string)
}

const (
	settingKeyHideMode    = "hide-mode"
	settingKeyDisplayMode = "display-mode"
	settingKeyPosition    = "position"
)

func NewDockManager() (*DockManager, error) {
	m := new(DockManager)
	err := m.init()
	if err != nil {
		return nil, err
	}
	return m, nil
}

func (m *DockManager) destroy() {
	if m.dockProperty != nil {
		m.dockProperty.destroy()
		m.dockProperty = nil
	}

	if m.hideStateManager != nil {
		m.hideStateManager.destroy()
		m.hideStateManager = nil
	}

	if m.dpy != nil {
		display.DestroyDisplay(m.dpy)
		m.dpy = nil
	}

}

func getDockHeightByDisplayMode(mode DisplayModeType) int {
	switch mode {
	case DisplayModeModernMode:
		return 68
	case DisplayModeEfficientMode:
		return 48
	case DisplayModeClassicMode:
		return 32
	default:
		return 0
	}
}

func (m *DockManager) updateDockRect() {
	// calc dock rect
	primaryX, primaryY, primaryW, primaryH := m.displayPrimaryRect.Pieces()
	dockX := primaryX + (primaryW-m.dockWidth)/2
	dockY := primaryY + primaryH - m.dockHeight

	if m.dockRect == nil {
		m.dockRect = xrect.New(dockX, dockY, m.dockWidth, m.dockHeight)
	} else {
		m.dockRect.XSet(dockX)
		m.dockRect.YSet(dockY)
		m.dockRect.WidthSet(m.dockWidth)
		m.dockRect.HeightSet(m.dockHeight)
	}

	logger.Debug("primary rect:", m.displayPrimaryRect)
	logger.Debug("dock width:", m.dockWidth)
	logger.Debug("dock height:", m.dockHeight)
	logger.Debug("updateDockRect dock rect:", m.dockRect)

	if m.hideStateManager != nil {
		m.hideStateManager.updateStateWithoutDelay()
	} else {
		logger.Debug("m.hideStateManager is nil")
	}

	if m.dockProperty != nil {
		m.dockProperty.updateDockRect()
	} else {
		logger.Debug("m.dockProperty is nil")
	}
}

// ActivateWindow会激活给定id的窗口，被激活的窗口通常会成为焦点窗口。
func (m *DockManager) ActivateWindow(win uint32) error {
	err := activateWindow(xproto.Window(win))
	if err != nil {
		logger.Warning("Activate window failed:", err)
		return err
	}
	return nil
}

// CloseWindow会将传入id的窗口关闭。
func (m *DockManager) CloseWindow(win uint32) error {
	err := ewmh.CloseWindow(XU, xproto.Window(win))
	if err != nil {
		logger.Warning("Close window failed:", err)
		return err
	}
	return nil
}

// ReorderEntries 重排序dock上的app项目
// 参数entryIDs为dock上app项目的新顺序id列表，要求与当前app项目是同一个集合，只是顺序不同。
func (m *DockManager) ReorderEntries(entryIDs []string) error {
	logger.Debugf("Reorder entryIDs %#v", entryIDs)
	if len(entryIDs) != len(m.Entries) {
		logger.Warning("Reorder: len(entryIDs) != len(m.Entries)")
		return errors.New("length of incomming entryIDs not equal length of m.Entries")
	}
	var orderedEntries []*AppEntry
	for _, id := range entryIDs {
		// TODO: 优化
		entry := m.getAppEntryByEntryId(id)
		if entry != nil {
			orderedEntries = append(orderedEntries, entry)
		} else {
			logger.Warningf("Reorder: invaild entry id %q", id)
			return fmt.Errorf("Invaild entry id %q", id)
		}
	}
	m.Entries = orderedEntries
	m.dockedAppManager.saveDockedAppList()
	return nil
}

// for debug
func (m *DockManager) GetEntryIDs() []string {
	list := make([]string, 0, len(m.Entries))
	for _, entry := range m.Entries {
		var appId string
		if entry.appInfo != nil {
			appId = entry.appInfo.GetId()
		} else {
			appId = entry.innerId
		}
		list = append(list, appId)
	}
	return list
}
