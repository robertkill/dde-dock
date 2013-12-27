/**
 * Copyright (c) 2011 ~ 2013 Deepin, Inc.
 *               2011 ~ 2013 jouyouyun
 *
 * Author:      jouyouyun <jouyouwen717@gmail.com>
 * Maintainer:  jouyouyun <jouyouwen717@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 **/

package main

// #cgo pkg-config: x11 xtst glib-2.0
// #include "grab-xrecord.h"
// #include <stdlib.h>
import "C"

import (
	/*"dlib"*/
	"dlib/dbus"
	"fmt"
	"github.com/BurntSushi/xgbutil/xevent"
)

func NewKeyBinding() *Manager {
	m := &Manager{}
	m.CustomBindList = GetCustomIdList()
	m.gsdAccelMap = GetGSDPairs()
	m.customAccelMap = GetCustomPairs()

	ListenKeyList(m)
	ListenCustomKey(m)
	ListenGSDKeyChanged(m)

	return m
}

func main() {
	binding := NewKeyBinding()
	err := dbus.InstallOnSession(binding)
	if err != nil {
		fmt.Println("Binding Get Session Bus Connect Failed:", err)
		return
	}

	kbd := &GrabManager{}
	err = dbus.InstallOnSession(kbd)
	if err != nil {
		fmt.Println("kbd Get Session Bus Connect Failed:", err)
		return
	}

	C.grab_xrecord_init()
	defer C.grab_xrecord_finalize()
	InitGrabKey()

	xevent.Main(X)
}
