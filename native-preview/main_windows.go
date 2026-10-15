//go:build windows

// ChaseHQ-Native-Preview.exe: ROM-free offline synthetic native-code demonstration.
// Win32 GDI GUI, Go standard library only. No copyrighted artwork, ROM bytes,
// SDL binaries or original ChaseHQ-Native emulator sources are linked.
package main

import (
	"encoding/base64"
	"encoding/binary"
	"fmt"
	"syscall"
	"unsafe"
)

var (
	u32        = syscall.NewLazyDLL("user32.dll")
	g32        = syscall.NewLazyDLL("gdi32.dll")
	k32        = syscall.NewLazyDLL("kernel32.dll")
	pReg       = u32.NewProc("RegisterClassExW")
	pCreate    = u32.NewProc("CreateWindowExW")
	pDefault   = u32.NewProc("DefWindowProcW")
	pShow      = u32.NewProc("ShowWindow")
	pUpdate    = u32.NewProc("UpdateWindow")
	pGet       = u32.NewProc("GetMessageW")
	pTranslate = u32.NewProc("TranslateMessage")
	pDispatch  = u32.NewProc("DispatchMessageW")
	pPostQuit  = u32.NewProc("PostQuitMessage")
	pBegin     = u32.NewProc("BeginPaint")
	pEnd       = u32.NewProc("EndPaint")
	pInvalid   = u32.NewProc("InvalidateRect")
	pGetClient = u32.NewProc("GetClientRect")
	pCursor    = u32.NewProc("LoadCursorW")
	pDpi       = u32.NewProc("SetProcessDPIAware")
	pFill      = u32.NewProc("FillRect")
	pText      = g32.NewProc("TextOutW")
	pTextColor = g32.NewProc("SetTextColor")
	pBkMode    = g32.NewProc("SetBkMode")
	pBrush     = g32.NewProc("CreateSolidBrush")
	pFont      = g32.NewProc("CreateFontW")
	pSelect    = g32.NewProc("SelectObject")
	pDelete    = g32.NewProc("DeleteObject")
)

const (
	wsVisible          = 0x10000000
	wsChild            = 0x40000000
	wsOverlappedWindow = 0x00CF0000
	wmDestroy          = 0x0002
	wmPaint            = 0x000F
	wmCommand          = 0x0111
	wmSize             = 0x0005
	wmEraseBkgnd       = 0x0014
	swShow             = 5
	idStep             = 101
	idReset            = 102
	idFault            = 103
	idNext             = 104
)

type rect struct{ L, T, R, B int32 }
type point struct{ X, Y int32 }
type msg struct {
	Hwnd     uintptr
	Message  uint32
	WParam   uintptr
	LParam   uintptr
	Time     uint32
	Pt       point
	LPrivate uint32
}
type wc struct {
	Size        uint32
	Style       uint32
	Proc        uintptr
	ClassExtra  int32
	WindowExtra int32
	Instance    uintptr
	Icon        uintptr
	Cursor      uintptr
	Background  uintptr
	Menu        *uint16
	Class       *uint16
	IconSm      uintptr
}
type paintstruct struct {
	Hdc       uintptr
	Erase     int32
	R         rect
	Restore   int32
	IncUpdate int32
	Reserved  [32]byte
}

type uiState struct {
	hwnd                  uintptr
	frame                 int
	inject, next          bool
	viewWidth, viewHeight int32
}

var app uiState

func rgb(r, g, b uint8) uintptr { return uintptr(uint32(r) | uint32(g)<<8 | uint32(b)<<16) }

var (
	bg     = rgb(13, 18, 28)
	panel  = rgb(25, 33, 47)
	panel2 = rgb(35, 46, 64)
	white  = rgb(227, 235, 247)
	muted  = rgb(164, 182, 204)
	teal   = rgb(45, 209, 182)
	red    = rgb(246, 102, 119)
	gold   = rgb(244, 193, 96)
)

func ptr(s string) *uint16 { p, _ := syscall.UTF16PtrFromString(s); return p }
func paintRect(h uintptr, x, y, w, hh int32, c uintptr) {
	if w <= 0 || hh <= 0 {
		return
	}
	b, _, _ := pBrush.Call(c)
	r := rect{x, y, x + w, y + hh}
	pFill.Call(h, uintptr(unsafe.Pointer(&r)), b)
	pDelete.Call(b)
}
func textAt(h uintptr, x, y int32, s string, c uintptr, size int32, bold bool) {
	height := uintptr(int64(-size))
	weight := uintptr(400)
	if bold {
		weight = 700
	}
	face := ptr("Segoe UI")
	f, _, _ := pFont.Call(height, 0, 0, 0, weight, 0, 0, 0, 1, 0, 0, 5, 0, uintptr(unsafe.Pointer(face)))
	old, _, _ := pSelect.Call(h, f)
	pBkMode.Call(h, 1)
	pTextColor.Call(h, c)
	utf := syscall.StringToUTF16(s)
	if len(utf) > 0 {
		pText.Call(h, uintptr(x), uintptr(y), uintptr(unsafe.Pointer(&utf[0])), uintptr(len(utf)-1))
	}
	pSelect.Call(h, old)
	pDelete.Call(f)
}
func border(h uintptr, x, y, w, hh int32, c uintptr) {
	paintRect(h, x, y, w, 2, c)
	paintRect(h, x, y+hh-2, w, 2, c)
	paintRect(h, x, y, 2, hh, c)
	paintRect(h, x+w-2, y, 2, hh, c)
}

func drawRoad(h uintptr, x, y, w, hh int32, ref bool, road []uint16, frame int, inject bool) {
	paintRect(h, x, y, w, hh, rgb(18, 26, 40))
	// Synthetic horizon, perspective trapezoid, then 104 independent road-mask stripes.
	paintRect(h, x+8, y+10, w-16, hh/3, rgb(39, 58, 80))
	paintRect(h, x+8, y+hh/3, w-16, hh/3, rgb(43, 54, 55))
	for i := 0; i < 104; i++ {
		bandY := y + hh/3 + int32(i)*(hh*2/3)/104
		nextY := y + hh/3 + int32(i+1)*(hh*2/3)/104
		if nextY <= bandY {
			nextY = bandY + 1
		}
		dist := int32(i + 10)
		roadWidth := int32(40) + dist*(w-58)/114
		if roadWidth > w-14 {
			roadWidth = w - 14
		}
		mid := x + w/2 + int32((frame*8+i/13)%27-13)
		left := mid - roadWidth/2
		active := road[i] == 0x9800
		col := rgb(88, 99, 117)
		if active {
			col = rgb(75, 127, 164)
		}
		if !ref && inject && i == 5 {
			col = red
		}
		paintRect(h, left, bandY, roadWidth, nextY-bandY, col)
		if i%15 < 3 {
			paintRect(h, mid-2, bandY, 4, nextY-bandY, rgb(225, 225, 199))
		}
		if i%10 < 2 {
			paintRect(h, left, bandY, 4, nextY-bandY, gold)
			paintRect(h, left+roadWidth-4, bandY, 4, nextY-bandY, gold)
		}
	}
	// Synthetic placeholders for sprite transfer records, NOT real sprite decoding.
	for i := 0; i < 8+frame; i++ {
		cx := x + w/2 + int32((i*29+frame*10)%120-60)
		cy := y + hh/2 + int32((i*27+frame*9)%120)
		scale := int32(8 + (i%3)*3)
		carCol := rgb(240, 93, 99)
		if i%3 == 0 {
			carCol = teal
		}
		paintRect(h, cx, cy, scale, scale/2+5, carCol)
		paintRect(h, cx+2, cy-3, scale-4, 4, rgb(18, 29, 45))
	}
	border(h, x, y, w, hh, rgb(66, 85, 103))
}
func paintMain(h uintptr, width, height int32) {
	paintRect(h, 0, 0, width, height, bg)
	paintRect(h, 0, 0, width, 6, teal)
	textAt(h, 30, 17, "CHASEHQ / NATIVE PREVIEW", white, 25, true)
	textAt(h, 31, 51, "Standalone Windows POC    •    ROM-free synthetic demo    •    NOT an RC5 release", muted, 14, false)
	if app.next {
		paintNext(h, width, height)
		return
	}
	v, err := Validate(app.frame, app.inject)
	if err != nil {
		textAt(h, 30, 145, "ERROR: "+err.Error(), red, 19, true)
		return
	}
	showW := (width - 90) / 2
	if showW < 200 {
		showW = 200
	}
	left := int32(30)
	right := left + showW + 30
	top := int32(154)
	viewHeight := height - 420
	if viewHeight < 160 {
		viewHeight = 160
	}
	paintRect(h, left, top, showW, viewHeight+78, panel)
	paintRect(h, right, top, showW, viewHeight+78, panel)
	textAt(h, left+18, top+11, "REFERENCE  /  SYNTHETIC FIXTURE", white, 16, true)
	textAt(h, right+18, top+11, "NATIVE CALCULATION  /  SYNTHETIC", white, 16, true)
	refRoad := referenceRoad(app.frame)
	drawRoad(h, left+18, top+42, showW-36, viewHeight-14, true, refRoad, app.frame, false)
	drawRoad(h, right+18, top+42, showW-36, viewHeight-14, false, v.Scene.Road, app.frame, app.inject)
	textAt(h, left+18, top+viewHeight+34, "Expected bytes: independent fixture", muted, 13, false)
	textAt(h, right+18, top+viewHeight+34, "Compiled model output: compared byte-for-byte", muted, 13, false)
	detailY := top + viewHeight + 97
	paintRect(h, 30, detailY, width-60, 108, panel2)
	resultCol := teal
	resultTxt := "PASS: native output matches synthetic reference"
	if v.Mismatches > 0 {
		resultCol = red
		resultTxt = fmt.Sprintf("MISMATCH: %d bytes; %s", v.Mismatches, v.FirstMismatch)
	}
	textAt(h, 47, detailY+12, resultTxt, resultCol, 18, true)
	textAt(h, 47, detailY+45, fmt.Sprintf("Frame %d/4   |   Road mask: %d of 104 entries active   |   Sprite records: %d/256", app.frame+1, v.Scene.Selected, v.Scene.Records), white, 15, false)
	textAt(h, 47, detailY+73, fmt.Sprintf("SPRITE: %s    ROAD: %s    MUTATED SOURCE: %s", yn(v.SpriteOK), yn(v.RoadOK), yn(v.SourceOK)), muted, 14, true)
	textAt(h, 31, height-45, "Native prototypes: OFFLINE candidate  |  Live SHADOW: PENDING  |  Emulator / original gameplay: NOT IN THIS DEMO", muted, 13, false)
}
func referenceRoad(frame int) []uint16 {
	o, e := getOracle()
	if e != nil || frame < 0 || frame >= len(o.Cases) {
		return make([]uint16, 104)
	}
	raw, e := base64.StdEncoding.DecodeString(o.Cases[frame].Road)
	if e != nil || len(raw) != 208 {
		return make([]uint16, 104)
	}
	words := make([]uint16, 104)
	for i := range words {
		words[i] = binary.BigEndian.Uint16(raw[2*i:])
	}
	return words
}
func yn(v bool) string {
	if v {
		return "PASS"
	}
	return "FAIL"
}
func paintNext(h uintptr, width, height int32) {
	paintRect(h, 30, 157, width-60, height-225, panel)
	textAt(h, 50, 178, "NATIVE DEVELOPMENT / NEXT STEPS", white, 23, true)
	lines := []struct {
		Status, S string
		Col       uintptr
	}{
		{"COMPLETE", "CPU-A sprite-transfer: independent offline C++ candidate; 6,144 captured bytes match.", teal},
		{"COMPLETE", "CPU-B 104-word road mask: C++ candidate; 832 captured bytes match.", teal},
		{"COMPLETE", "Road classes 4/6: material → CPU-B → TC0150 RAM → rendered pixels verified.", teal},
		{"COMPLETE", "Beacon proximity, request, 180-frame sound command, score, turbo timer researched.", teal},
		{"THIS DEMO", "Self-contained Windows preview: synthetic Go-native visualization and reference tests.", gold},
		{"OPEN", "RC4.7 source received: C++ candidates in runtime, opt-in read-only shadow hooks added.", gold},
		{"OPEN", "Instruction-time capture, live SHADOW/COMPARE, recoverable mismatch evidence.", gold},
		{"OPEN", "Workbench Next Steps updated in source; verify actual Windows display and workflow.", gold},
		{"OPEN", "Follow prescribed Windows build, regression, packaging and clean-run qualification.", gold},
		{"OPEN", "Real native graphics/gameplay/audio beyond limited routine candidates.", gold},
	}
	y := int32(226)
	for _, item := range lines {
		textAt(h, 52, y, item.Status, item.Col, 13, true)
		textAt(h, 169, y, item.S, white, 14, false)
		y += 37
	}
	textAt(h, 50, height-110, "This is the preview POC's status. It does not replace the actual ChaseHQ Workbench Next Steps tab.", muted, 14, false)
}
func makeButton(parent uintptr, id int32, title string, x, w int32) {
	klass := ptr("BUTTON")
	name := ptr(title)
	pCreate.Call(0, uintptr(unsafe.Pointer(klass)), uintptr(unsafe.Pointer(name)), wsChild|wsVisible,
		uintptr(x), uintptr(102), uintptr(w), uintptr(37), parent, uintptr(id), 0, 0)
}
func wndProc(hwnd uintptr, message uint32, wparam, lparam uintptr) uintptr {
	switch message {
	case wmDestroy:
		pPostQuit.Call(0)
		return 0
	case wmEraseBkgnd:
		return 1
	case wmSize:
		app.viewWidth = int32(uint16(lparam & 0xffff))
		app.viewHeight = int32(uint16((lparam >> 16) & 0xffff))
		return 0
	case wmCommand:
		switch int(wparam & 0xffff) {
		case idStep:
			app.frame = (app.frame + 1) % 4
		case idReset:
			app.frame = 0
			app.inject = false
		case idFault:
			app.inject = !app.inject
		case idNext:
			app.next = !app.next
		}
		pInvalid.Call(hwnd, 0, 1)
		return 0
	case wmPaint:
		var ps paintstruct
		dc, _, _ := pBegin.Call(hwnd, uintptr(unsafe.Pointer(&ps)))
		var bounds rect
		pGetClient.Call(hwnd, uintptr(unsafe.Pointer(&bounds)))
		paintMain(dc, bounds.R-bounds.L, bounds.B-bounds.T)
		pEnd.Call(hwnd, uintptr(unsafe.Pointer(&ps)))
		return 0
	}
	ret, _, _ := pDefault.Call(hwnd, uintptr(message), wparam, lparam)
	return ret
}
func main() {
	pDpi.Call()
	mod, _, _ := k32.NewProc("GetModuleHandleW").Call(0)
	class := ptr("CHQNativePreviewSyntheticClass")
	cursor, _, _ := pCursor.Call(0, 32512)
	proc := syscall.NewCallback(wndProc)
	cls := wc{Size: uint32(unsafe.Sizeof(wc{})), Style: 3, Proc: proc, Instance: mod, Cursor: cursor, Background: 6, Class: class}
	atom, _, e := pReg.Call(uintptr(unsafe.Pointer(&cls)))
	if atom == 0 {
		panic(fmt.Sprintf("RegisterClassExW: %v", e))
	}
	title := ptr("ChaseHQ Native Preview — Synthetic POC (NOT RC5)")
	hwnd, _, err := pCreate.Call(0, uintptr(unsafe.Pointer(class)), uintptr(unsafe.Pointer(title)), wsOverlappedWindow|wsVisible,
		100, 85, 1160, 810, 0, 0, mod, 0)
	if hwnd == 0 {
		panic(fmt.Sprintf("CreateWindowExW: %v", err))
	}
	app.hwnd = hwnd
	makeButton(hwnd, idStep, "Next synthetic frame", 31, 185)
	makeButton(hwnd, idReset, "Reset", 230, 111)
	makeButton(hwnd, idFault, "Toggle mismatch", 354, 164)
	makeButton(hwnd, idNext, "Next Steps / Demo", 531, 175)
	pShow.Call(hwnd, swShow)
	pUpdate.Call(hwnd)
	var message msg
	for {
		r, _, _ := pGet.Call(uintptr(unsafe.Pointer(&message)), 0, 0, 0)
		if int32(r) <= 0 {
			break
		}
		pTranslate.Call(uintptr(unsafe.Pointer(&message)))
		pDispatch.Call(uintptr(unsafe.Pointer(&message)))
	}
}
