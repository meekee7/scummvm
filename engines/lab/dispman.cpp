/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#include "graphics/palette.h"

#include "lab/lab.h"

#include "lab/anim.h"
#include "lab/dispman.h"
#include "lab/eventman.h"
#include "lab/music.h"
#include "lab/image.h"
#include "lab/resource.h"
#include "lab/utils.h"

namespace Lab {

DisplayMan::DisplayMan(LabEngine *vm) : _vm(vm) {
	_longWinInFront = false;
	_lastMessageLong = false;
	_actionMessageShown = false;

	_screenBytesPerPage = 0;
	_curPen = 0;
	_curBitmap = nullptr;
	_displayBuffer = nullptr;
	_currentDisplayBuffer = nullptr;
	_fadePalette = nullptr;

	_screenWidth = 0;
	_screenHeight = 0;

	for (int i = 0; i < 256 * 3; i++)
		_curvgapal[i] = 0;

	_dispBitMap = new BitMap;
}

DisplayMan::~DisplayMan() {
	freePict();
	delete _dispBitMap;
	delete[] _displayBuffer;
}

void DisplayMan::loadPict(const Common::String filename) {
	freePict();
	_curBitmap = _vm->_resource->openDataFile(filename);
}

void DisplayMan::loadBackPict(const Common::String fileName, uint16 *highPal) {
	_fadePalette = highPal;
	_vm->_anim->_noPalChange = true;
	readPict(fileName);

	for (int i = 0; i < 16; i++) {
		highPal[i] = ((_vm->_anim->_diffPalette[i * 3] >> 2) << 8) +
			((_vm->_anim->_diffPalette[i * 3 + 1] >> 2) << 4) +
			((_vm->_anim->_diffPalette[i * 3 + 2] >> 2));
	}

	_vm->_anim->_noPalChange = false;
}

void DisplayMan::readPict(const Common::String filename, bool playOnce, bool onlyDiffData, byte *memoryBuffer) {
	_vm->_anim->stopDiff();
	loadPict(filename);
	_vm->updateMusicAndEvents();

	if (!_vm->_music->_loopSoundEffect)
		_vm->_music->stopSoundEffect();

	_dispBitMap->_bytesPerRow  = _screenWidth;
	_dispBitMap->_drawOnScreen = (memoryBuffer == nullptr);
	if (memoryBuffer)
		_dispBitMap->_planes[0] = memoryBuffer;

	_vm->_anim->readDiff(_curBitmap, playOnce, onlyDiffData);
}

void DisplayMan::freePict() {
	delete _curBitmap;
	_curBitmap = nullptr;
}

Common::String DisplayMan::getWord(const char *mainBuffer) {
	Common::String result;

	for (int i = 0; mainBuffer[i] && (mainBuffer[i] != ' ') && (mainBuffer[i] != '\n'); i++)
		result += mainBuffer[i];

	return result;
}

Common::String DisplayMan::getLine(TextFont *tf, const char **mainBuffer, uint16 lineWidth) {
	uint16 curWidth = 0;
	Common::String result;
	bool doit = true;

	lineWidth += textLength(tf, " ");

	while ((*mainBuffer)[0] && doit) {
		Common::String wordBuffer = getWord(*mainBuffer) + " ";

		if ((curWidth + textLength(tf, wordBuffer)) <= lineWidth) {
			result += wordBuffer;
			(*mainBuffer) += wordBuffer.size() - 1;

			if ((*mainBuffer)[0] == '\n')
				doit = false;

			if ((*mainBuffer)[0])
				(*mainBuffer)++;

			curWidth = textLength(tf, result);
		} else
			doit = false;
	}

	return result;
}

int DisplayMan::flowText(TextFont *font, int16 spacing, byte penColor, byte backPen,
			bool fillBack, bool centerh, bool centerv, bool output, Common::Rect textRect, const char *str) {
	if (fillBack) {
		setPen(backPen);
		rectFill(textRect);
	}

	if (!str)
		return 0;

	setPen(penColor);

	TextFont *msgFont = font;
	uint16 fontHeight = textHeight(msgFont) + spacing;
	uint16 numLines   = (textRect.height() + 1) / fontHeight;
	uint16 width      = textRect.width() + 1;
	uint16 y          = textRect.top;
	Common::String lineBuffer;

	if (centerv && output) {
		const char *temp = str;
		uint16 actlines = 0;

		while (temp[0]) {
			lineBuffer = getLine(msgFont, &temp, width);
			actlines++;
		}

		if (actlines <= numLines)
			y += ((textRect.height() + 1) - (actlines * fontHeight)) / 2;
	}

	int len = 0;
	while (numLines && str[0]) {
		lineBuffer = getLine(msgFont, &str, width);

		uint16 x = textRect.left;
		len += lineBuffer.size();

		if (centerh)
			x += (width - textLength(msgFont, lineBuffer)) / 2;

		if (output)
			drawText(msgFont, x, y, penColor, lineBuffer);

		numLines--;
		y += fontHeight;
	}

	len--;

	return len;
}

int DisplayMan::flowTextToMem(Image *destIm, TextFont *font, int16 spacing, byte penColor,
			byte backPen, bool fillBack, bool centerh, bool centerv, bool output, Common::Rect textRect,
			const char *str) {
	byte *saveDisplayBuffer = _currentDisplayBuffer;
	uint32 bytesPerPage = _screenBytesPerPage;

	_currentDisplayBuffer = destIm->_imageData;
	_screenBytesPerPage = (uint32)destIm->_width * (int32)destIm->_height;

	int res = flowText(font, spacing, penColor, backPen, fillBack, centerh, centerv, output, textRect, str);

	_screenBytesPerPage = bytesPerPage;
	_currentDisplayBuffer = saveDisplayBuffer;

	return res;
}

void DisplayMan::createBox(uint16 y2) {
	// Message box area
	setPen(7);
	rectFillScaled(4, 154, 315, y2 - 2);

	// Box around message area
	setPen(0);
	drawHLine(_vm->_utils->vgaScaleX(2), _vm->_utils->vgaScaleY(152), _vm->_utils->vgaScaleX(317));
	drawVLine(_vm->_utils->vgaScaleX(317), _vm->_utils->vgaScaleY(152), _vm->_utils->vgaScaleY(y2));
	drawHLine(_vm->_utils->vgaScaleX(2), _vm->_utils->vgaScaleY(y2), _vm->_utils->vgaScaleX(317));
	drawVLine(_vm->_utils->vgaScaleX(2), _vm->_utils->vgaScaleY(152), _vm->_utils->vgaScaleY(y2));
}

int DisplayMan::longDrawMessage(Common::String str, bool isActionMessage) {
	if (isActionMessage) {
		_actionMessageShown = true;
	} else if (_actionMessageShown) {
		_actionMessageShown = false;
		return 0;
	}

	if (str.empty())
		return 0;

	_vm->_event->attachButtonList(nullptr);
	_vm->_event->mouseHide();

	if (!_longWinInFront) {
		_longWinInFront = true;
		// Clear Area
		setPen(3);
		rectFill(0, _vm->_utils->vgaScaleY(149) + _vm->_utils->svgaCord(2), _vm->_utils->vgaScaleX(319), _vm->_utils->vgaScaleY(199));
	}

	createBox(198);
	_vm->_event->mouseShow();

	return flowText(_vm->_msgFont, 0, 1, 7, false, true, true, true, _vm->_utils->vgaRectScale(6, 155, 313, 195), str.c_str());
}

void DisplayMan::drawMessage(Common::String str, bool isActionMessage) {
	if (isActionMessage) {
		_actionMessageShown = true;
	} else if (_actionMessageShown) {
		_actionMessageShown = false;
		return;
	}

	if (str.empty())
		return;

	if ((textLength(_vm->_msgFont, str) > _vm->_utils->vgaScaleX(306))) {
		longDrawMessage(str, isActionMessage);
		_lastMessageLong = true;
	} else {
		if (_longWinInFront) {
			_longWinInFront = false;
			drawPanel();
		}

		_vm->_event->mouseHide();
		createBox(168);
		drawText(_vm->_msgFont, _vm->_utils->vgaScaleX(7), _vm->_utils->vgaScaleY(155) + _vm->_utils->svgaCord(2), 1, str);
		_vm->_event->mouseShow();
		_lastMessageLong = false;
	}
}

void DisplayMan::drawPanel() {
	_vm->_event->mouseHide();

	// Clear Area
	setPen(3);
	rectFill(0, _vm->_utils->vgaScaleY(149) + _vm->_utils->svgaCord(2), _vm->_utils->vgaScaleX(319), _vm->_utils->vgaScaleY(199));

	// First Line
	setPen(0);
	drawHLine(0, _vm->_utils->vgaScaleY(149) + _vm->_utils->svgaCord(2), _vm->_utils->vgaScaleX(319));
	// Second Line
	setPen(5);
	drawHLine(0, _vm->_utils->vgaScaleY(149) + 1 + _vm->_utils->svgaCord(2), _vm->_utils->vgaScaleX(319));
	// Button Separators
	setPen(0);
	// First black line to separate buttons
	drawHLine(0, _vm->_utils->vgaScaleY(170), _vm->_utils->vgaScaleX(319));

	if (!_vm->_alternate) {
		setPen(4);
		// The horizontal lines under the black one
		drawHLine(0, _vm->_utils->vgaScaleY(170) + 1, _vm->_utils->vgaScaleX(319));
		_vm->_event->drawButtonList(&_vm->_moveButtonList);
	} else {
		if (_vm->getPlatform() != Common::kPlatformWindows) {
			// Vertical Black lines
			drawVLine(_vm->_utils->vgaScaleX(124), _vm->_utils->vgaScaleY(170) + 1, _vm->_utils->vgaScaleY(199));
			drawVLine(_vm->_utils->vgaScaleX(194), _vm->_utils->vgaScaleY(170) + 1, _vm->_utils->vgaScaleY(199));
		} else {
			// Vertical Black lines
			drawVLine(_vm->_utils->vgaScaleX(90), _vm->_utils->vgaScaleY(170) + 1, _vm->_utils->vgaScaleY(199));
			drawVLine(_vm->_utils->vgaScaleX(160), _vm->_utils->vgaScaleY(170) + 1, _vm->_utils->vgaScaleY(199));
			drawVLine(_vm->_utils->vgaScaleX(230), _vm->_utils->vgaScaleY(170) + 1, _vm->_utils->vgaScaleY(199));
		}

		setPen(4);
		// The horizontal lines under the black one
		drawHLine(0, _vm->_utils->vgaScaleY(170) + 1, _vm->_utils->vgaScaleX(122));
		drawHLine(_vm->_utils->vgaScaleX(126), _vm->_utils->vgaScaleY(170) + 1, _vm->_utils->vgaScaleX(192));
		drawHLine(_vm->_utils->vgaScaleX(196), _vm->_utils->vgaScaleY(170) + 1, _vm->_utils->vgaScaleX(319));
		// The vertical high light lines
		drawVLine(_vm->_utils->vgaScaleX(1), _vm->_utils->vgaScaleY(170) + 2, _vm->_utils->vgaScaleY(198));

		if (_vm->getPlatform() != Common::kPlatformWindows) {
			drawVLine(_vm->_utils->vgaScaleX(126), _vm->_utils->vgaScaleY(170) + 2, _vm->_utils->vgaScaleY(198));
			drawVLine(_vm->_utils->vgaScaleX(196), _vm->_utils->vgaScaleY(170) + 2, _vm->_utils->vgaScaleY(198));
		} else {
			drawVLine(_vm->_utils->vgaScaleX(92), _vm->_utils->vgaScaleY(170) + 2, _vm->_utils->vgaScaleY(198));
			drawVLine(_vm->_utils->vgaScaleX(162), _vm->_utils->vgaScaleY(170) + 2, _vm->_utils->vgaScaleY(198));
			drawVLine(_vm->_utils->vgaScaleX(232), _vm->_utils->vgaScaleY(170) + 2, _vm->_utils->vgaScaleY(198));
		}

		_vm->_event->drawButtonList(&_vm->_invButtonList);
	}

	_vm->_event->mouseShow();
}

void DisplayMan::setUpScreens() {
	EventManager *e = _vm->_event;
	ButtonList *moveButtonList = &_vm->_moveButtonList;
	ButtonList *invButtonList = &_vm->_invButtonList;
	Image **moveImages = _vm->_moveImages;
	Image **invImages = _vm->_invImages;

	createScreen(_vm->_isHiRes);

	// TODO: The CONTROL file is not present in the Amiga version
	Common::File *controlFile = _vm->_resource->openDataFile("P:Control");
	for (int i = 0; i < 20; i++)
		_vm->_moveImages[i] = new Image(controlFile, _vm);
	delete controlFile;

	// Creates the buttons for the movement control panel
	// The key mapping was only set for the Windows version.
	// It's very convenient to have those shortcut, so I added them
	// for all versions. (Strangerke)
	uint16 y = _vm->_utils->vgaScaleY(173) - _vm->_utils->svgaCord(2);
	moveButtonList->push_back(e->createButton(  1, y, 0,          't', moveImages[0],  moveImages[1]));
	moveButtonList->push_back(e->createButton( 33, y, 1,          'm', moveImages[2],  moveImages[3]));
	moveButtonList->push_back(e->createButton( 65, y, 2,          'o', moveImages[4],  moveImages[5]));
	moveButtonList->push_back(e->createButton( 97, y, 3,          'c', moveImages[6],  moveImages[7]));
	moveButtonList->push_back(e->createButton(129, y, 4,          'l', moveImages[8],  moveImages[9]));
	moveButtonList->push_back(e->createButton(161, y, 5,          'i', moveImages[12], moveImages[13]));
	moveButtonList->push_back(e->createButton(193, y, 6, VKEY_LTARROW, moveImages[14], moveImages[15]));
	moveButtonList->push_back(e->createButton(225, y, 7, VKEY_UPARROW, moveImages[16], moveImages[17]));
	moveButtonList->push_back(e->createButton(257, y, 8, VKEY_RTARROW, moveImages[18], moveImages[19]));
	moveButtonList->push_back(e->createButton(289, y, 9,          'p', moveImages[10], moveImages[11]));

	// TODO: The INV file is not present in the Amiga version
	Common::File *invFile = _vm->_resource->openDataFile("P:Inv");
	if (_vm->getPlatform() == Common::kPlatformWindows) {
		for (int imgIdx = 0; imgIdx < 10; imgIdx++)
			_vm->_invImages[imgIdx] = new Image(invFile, _vm);
	} else {
		for (int imgIdx = 0; imgIdx < 6; imgIdx++)
			_vm->_invImages[imgIdx] = new Image(invFile, _vm);
	}
	invButtonList->push_back(e->createButton( 24, y, 0,          'm', invImages[0],   invImages[1]));
	invButtonList->push_back(e->createButton( 56, y, 1,          'g', invImages[2],   invImages[3]));
	invButtonList->push_back(e->createButton( 94, y, 2,          'u', invImages[4],   invImages[5]));
	invButtonList->push_back(e->createButton(126, y, 3,          'l', moveImages[8],  moveImages[9]));
	invButtonList->push_back(e->createButton(164, y, 4, VKEY_LTARROW, moveImages[14], moveImages[15]));
	invButtonList->push_back(e->createButton(196, y, 5, VKEY_RTARROW, moveImages[18], moveImages[19]));

	// The windows version has 2 extra buttons for breadcrumb trail
	// CHECKME: the game is really hard to play without those, maybe we could add something to enable that.
	if (_vm->getPlatform() == Common::kPlatformWindows) {
		invButtonList->push_back(e->createButton(234, y, 6, 'b', invImages[6], invImages[7]));
		invButtonList->push_back(e->createButton(266, y, 7, 'f', invImages[8], invImages[9]));
	}

	delete invFile;
}

void DisplayMan::setPen(byte penNum) {
	_curPen = penNum;
}

void DisplayMan::rectFill(Common::Rect fillRect) {
	int width = fillRect.width() + 1;
	int height = fillRect.height() + 1;

	if (fillRect.left + width > _screenWidth)
		width = _screenWidth - fillRect.left;

	if (fillRect.top + height > _screenHeight)
		height = _screenHeight - fillRect.top;

	if ((width > 0) && (height > 0)) {
		char *d = (char *)getCurrentDrawingBuffer() + fillRect.top * _screenWidth + fillRect.left;

		while (height-- > 0) {
			char *dd = d;
			int ww = width;

			while (ww-- > 0) {
				*dd++ = _curPen;
			}

			d += _screenWidth;
		}
	}
}

void DisplayMan::rectFill(uint16 x1, uint16 y1, uint16 x2, uint16 y2) {
	rectFill(Common::Rect(x1, y1, x2, y2));
}

void DisplayMan::rectFillScaled(uint16 x1, uint16 y1, uint16 x2, uint16 y2) {
	rectFill(_vm->_utils->vgaRectScale(x1, y1, x2, y2));
}

void DisplayMan::drawVLine(uint16 x, uint16 y1, uint16 y2) {
	rectFill(x, y1, x, y2);
}

void DisplayMan::drawHLine(uint16 x1, uint16 y, uint16 x2) {
	rectFill(x1, y, x2, y);
}

void DisplayMan::screenUpdate() {
	_vm->_system->copyRectToScreen(_displayBuffer, _screenWidth, 0, 0, _screenWidth, _screenHeight);
	_vm->_system->updateScreen();

	_vm->_event->processInput();
}

void DisplayMan::createScreen(bool hiRes) {
	if (hiRes) {
		_screenWidth  = 640;
		_screenHeight = 480;
	} else {
		_screenWidth  = 320;
		_screenHeight = 200;
	}
	_screenBytesPerPage = _screenWidth * _screenHeight;

	if (_displayBuffer)
		delete[] _displayBuffer;
	_displayBuffer = new byte[_screenBytesPerPage];
}

void DisplayMan::setAmigaPal(uint16 *pal) {
	byte vgaPal[16 * 3];
	uint16 vgaIdx = 0;

	for (int i = 0; i < 16; i++) {
		vgaPal[vgaIdx++] = (byte)(((pal[i] & 0xf00) >> 8) << 2);
		vgaPal[vgaIdx++] = (byte)(((pal[i] & 0x0f0) >> 4) << 2);
		vgaPal[vgaIdx++] = (byte)(((pal[i] & 0x00f)) << 2);
	}

	writeColorRegs(vgaPal, 0, 16);
	_vm->waitTOF();
}

void DisplayMan::writeColorRegs(byte *buf, uint16 first, uint16 numReg) {
	byte tmp[256 * 3];

	for (int i = 0; i < 256 * 3; i++)
		tmp[i] = (buf[i] << 2) | (buf[i] >> 4);	// better results than buf[i] * 4

	_vm->_system->getPaletteManager()->setPalette(tmp, first, numReg);
	memcpy(&(_curvgapal[first * 3]), buf, numReg * 3);
}

void DisplayMan::setPalette(void *newPal, uint16 numColors) {
	if (memcmp(newPal, _curvgapal, numColors * 3) != 0)
		writeColorRegs((byte *)newPal, 0, numColors);
}

byte *DisplayMan::getCurrentDrawingBuffer() {
	if (_currentDisplayBuffer)
		return _currentDisplayBuffer;

	return _displayBuffer;
}

void DisplayMan::checkerboardEffect(uint16 penColor, uint16 x1, uint16 y1, uint16 x2, uint16 y2) {
	int w = x2 - x1 + 1;
	int h = y2 - y1 + 1;

	if (x1 + w > _screenWidth)
		w = _screenWidth - x1;

	if (y1 + h > _screenHeight)
		h = _screenHeight - y1;

	if ((w > 0) && (h > 0)) {
		byte *d = (byte *)getCurrentDrawingBuffer() + y1 * _screenWidth + x1;

		while (h-- > 0) {
			byte *dd = d;
			int ww = w;

			if (y1 & 1) {
				dd++;
				ww--;
			}

			while (ww > 0) {
				*dd = penColor;
				dd += 2;
				ww -= 2;
			}

			d += _screenWidth;
			y1++;
		}
	}
}

void DisplayMan::closeFont(TextFont **font) {
	if (*font) {
		if ((*font)->_data)
			delete[] (*font)->_data;

		delete *font;
		*font = nullptr;
	}
}

uint16 DisplayMan::textLength(TextFont *font, const Common::String text) {
	uint16 length = 0;

	if (font) {
		int numChars = text.size();
		for (int i = 0; i < numChars; i++) {
			length += font->_widths[(byte)text[i]];
		}
	}

	return length;
}

uint16 DisplayMan::textHeight(TextFont *tf) {
	return (tf) ? tf->_height : 0;
}

void DisplayMan::drawText(TextFont *tf, uint16 x, uint16 y, uint16 color, const Common::String text) {
	byte *vgaTop = getCurrentDrawingBuffer();
	int numChars = text.size();

	for (int i = 0; i < numChars; i++) {
		uint32 realOffset = (_screenWidth * y) + x;
		uint16 curPage    = realOffset / _screenBytesPerPage;
		uint32 segmentOffset = realOffset - (curPage * _screenBytesPerPage);
		int32 leftInSegment = _screenBytesPerPage - segmentOffset;
		byte *vgaCur = vgaTop + segmentOffset;

		if (tf->_widths[(byte)text[i]]) {
			byte *cdata = tf->_data + tf->_offsets[(byte)text[i]];
			uint16 bwidth = *cdata++;
			byte *vgaTemp = vgaCur;
			byte *vgaTempLine = vgaCur;

			for (int rows = 0; rows < tf->_height; rows++) {
				int32 templeft = leftInSegment;

				vgaTemp = vgaTempLine;

				for (int cols = 0; cols < bwidth; cols++) {
					uint16 data = *cdata++;

					if (data && (templeft >= 8)) {
						for (int j = 7; j >= 0; j--) {
							if ((1 << j) & data)
								*vgaTemp = color;
							vgaTemp++;
						}

						templeft -= 8;
					} else if (data) {
						uint16 mask = 0x80;
						templeft = leftInSegment;

						for (int counterb = 0; counterb < 8; counterb++) {
							if (templeft <= 0) {
								curPage++;
								vgaTemp = (byte *)(vgaTop - templeft);
								// Set up VGATempLine for next line
								vgaTempLine -= _screenBytesPerPage;
								// Set up LeftInSegment for next line
								leftInSegment += _screenBytesPerPage + templeft;
								templeft += _screenBytesPerPage;
							}

							if (mask & data)
								*vgaTemp = color;

							vgaTemp++;

							mask = mask >> 1;
							templeft--;
						}
					} else {
						templeft -= 8;
						vgaTemp += 8;
					}
				}

				vgaTempLine += _screenWidth;
				leftInSegment -= _screenWidth;

				if (leftInSegment <= 0) {
					curPage++;
					vgaTempLine -= _screenBytesPerPage;
					leftInSegment += _screenBytesPerPage;
				}
			}
		}

		x += tf->_widths[(byte)text[i]];
	}
}

void DisplayMan::doScrollBlack() {
	uint16 width = _vm->_utils->vgaScaleX(320);
	uint16 height = _vm->_utils->vgaScaleY(149) + _vm->_utils->svgaCord(2);

	_vm->_event->mouseHide();

	byte *mem = new byte[width * height];
	int16 by = _vm->_utils->vgaScaleX(4);
	int16 verticalScroll = height;

	while (verticalScroll > 0) {
		scrollDisplayY(-by, 0, 0, width - 1, height - 1, mem);
		verticalScroll -= by;

		_vm->updateMusicAndEvents();
		_vm->waitTOF();
	}

	delete[] mem;

	_vm->_event->mouseShow();
}

void DisplayMan::copyPage(uint16 width, uint16 height, uint16 nheight, uint16 startLine, byte *mem) {
	byte *baseAddr = getCurrentDrawingBuffer();

	uint32 size = (int32)(height - nheight) * (int32)width;
	mem += startLine * width;
	uint16 curPage = ((int32)nheight * (int32)width) / _screenBytesPerPage;
	uint32 offSet = ((int32)nheight * (int32)width) - (curPage * _screenBytesPerPage);

	while (size) {
		uint32 copySize;
		if (size > (_screenBytesPerPage - offSet))
			copySize = _screenBytesPerPage - offSet;
		else
			copySize = size;

		size -= copySize;

		memcpy(baseAddr + (offSet >> 2), mem, copySize);
		mem += copySize;
		curPage++;
		offSet = 0;
	}
}

void DisplayMan::doScrollWipe(const Common::String filename) {
	_vm->_event->mouseHide();
	uint16 width = _vm->_utils->vgaScaleX(320);
	uint16 height = _vm->_utils->vgaScaleY(149) + _vm->_utils->svgaCord(2);

	while (_vm->_music->isSoundEffectActive()) {
		_vm->updateMusicAndEvents();
		_vm->waitTOF();
	}

	readPict(filename, true, true);
	setPalette(_vm->_anim->_diffPalette, 256);
	byte *mem = _vm->_anim->_scrollScreenBuffer;

	_vm->updateMusicAndEvents();
	uint16 by = _vm->_utils->vgaScaleX(3);
	uint16 nheight = height;
	uint16 startLine = 0, onRow = 0;

	while (onRow < _vm->_anim->_headerdata._height) {
		_vm->updateMusicAndEvents();

		if ((by > nheight) && nheight)
			by = nheight;

		if ((startLine + by) > (_vm->_anim->_headerdata._height - height - 1))
			break;

		if (nheight)
			nheight -= by;

		copyPage(width, height, nheight, startLine, mem);

		screenUpdate();

		if (!nheight)
			startLine += by;

		onRow += by;

		if (nheight <= (height / 4))
			by = _vm->_utils->vgaScaleX(5);
		else if (nheight <= (height / 3))
			by = _vm->_utils->vgaScaleX(4);
		else if (nheight <= (height / 2))
			by = _vm->_utils->vgaScaleX(3);
	}

	_vm->_event->mouseShow();
}

void DisplayMan::doScrollBounce() {
	const uint16 offsets[8] = { 3, 3, 2, 2, 2, 1, 1, 1 };
	const int multiplier = (_vm->_isHiRes) ? 2 : 1;

	_vm->_event->mouseHide();
	int width = _vm->_utils->vgaScaleX(320);
	int height = _vm->_utils->vgaScaleY(149) + _vm->_utils->svgaCord(2);
	byte *mem = _vm->_anim->_scrollScreenBuffer;

	_vm->updateMusicAndEvents();
	int startLine = _vm->_anim->_headerdata._height - height - 1;

	for (int i = 0; i < 5; i++) {
		_vm->updateMusicAndEvents();
		startLine -= (5 - i) * multiplier;
		copyPage(width, height, 0, startLine, mem);
		_vm->waitTOF();
	}

	for (int i = 8; i > 0; i--) {
		_vm->updateMusicAndEvents();
		startLine += offsets[i - 1] * multiplier;
		copyPage(width, height, 0, startLine, mem);
		_vm->waitTOF();
	}

	_vm->_event->mouseShow();
}

void DisplayMan::doTransWipe(CloseDataPtr *closePtrList, const Common::String filename) {
	uint16 lastY, linesLast;

	if (_vm->_isHiRes) {
		linesLast = 3;
		lastY = 358;
	} else {
		linesLast = 1;
		lastY = 148;
	}

	uint16 linesDone = 0;

	for (int j = 0; j < 2; j++) {
		for (int i = 0; i < 2; i++) {
			uint16 curY = i * 2;

			while (curY < lastY) {
				if (linesDone >= linesLast) {
					_vm->updateMusicAndEvents();
					_vm->waitTOF();
					linesDone = 0;
				}

				if (j == 9)
					checkerboardEffect(0, 0, curY, _screenWidth - 1, curY + 1);
				else
					rectFill(0, curY, _screenWidth - 1, curY + 1);
				curY += 4;
				linesDone++;
			}	// while
		}	// for i

		setPen(0);
	}	// for j

	if (filename.empty())
		_vm->_curFileName = _vm->getPictName(closePtrList);
	else if (filename[0] > ' ')
		_vm->_curFileName = filename;
	else
		_vm->_curFileName = _vm->getPictName(closePtrList);

	byte *bitMapBuffer = new byte[_screenWidth * (lastY + 5)];
	readPict(_vm->_curFileName, true, false, bitMapBuffer);

	setPalette(_vm->_anim->_diffPalette, 256);

	Image imSource(_vm);
	imSource._width = _screenWidth;
	imSource._height = lastY;
	imSource._imageData = bitMapBuffer;

	Image imDest(_vm);
	imDest._width = _screenWidth;
	imDest._height = _screenHeight;
	imDest._imageData = getCurrentDrawingBuffer();

	for (int j = 0; j < 2; j++) {
		for (int i = 0; i < 2; i++) {
			uint16 curY = i * 2;

			while (curY < lastY) {
				if (linesDone >= linesLast) {
					_vm->updateMusicAndEvents();
					_vm->waitTOF();
					linesDone = 0;
				}

				imDest._imageData = getCurrentDrawingBuffer();

				if (j == 0) {
					imSource.blitBitmap(0, curY, &imDest, 0, curY, _screenWidth, 2, false);
					checkerboardEffect(0, 0, curY, _screenWidth - 1, curY + 1);
				} else {
					uint16 bitmapHeight = (curY == lastY) ? 1 : 2;
					imSource.blitBitmap(0, curY, &imDest, 0, curY, _screenWidth, bitmapHeight, false);
				}
				curY += 4;
				linesDone++;
			}	// while
		}	// for i
	}	// for j

	// Prevent the Image destructor from deleting the drawing buffer
	imDest._imageData = nullptr;

	// bitMapBuffer will be deleted by the Image destructor
}

void DisplayMan::doTransition(TransitionType transitionType, CloseDataPtr *closePtrList, const Common::String filename) {
	switch (transitionType) {
	case kTransitionWipe:
	case kTransitionTransporter:
		doTransWipe(closePtrList, filename);
		break;
	case kTransitionScrollWipe:
		doScrollWipe(filename);
		break;
	case kTransitionScrollBlack:
		doScrollBlack();
		break;
	case kTransitionScrollBounce:
		doScrollBounce();
		break;
	case kTransitionReadFirstFrame:
		readPict(filename, false);
		break;
	case kTransitionReadNextFrame:
		_vm->_anim->diffNextFrame();
		break;
	case kTransitionNone:
	default:
		break;
	}
}

void DisplayMan::blackScreen() {
	byte pal[256 * 3];
	memset(pal, 0, 248 * 3);
	writeColorRegs(pal, 8, 248);

	_vm->_system->delayMillis(32);
}

void DisplayMan::whiteScreen() {
	byte pal[256 * 3];
	memset(pal, 255, 248 * 3);
	writeColorRegs(pal, 8, 248);
}

void DisplayMan::blackAllScreen() {
	byte pal[256 * 3];
	memset(pal, 0, 256 * 3);
	writeColorRegs(pal, 0, 256);

	_vm->_system->delayMillis(32);
}

void DisplayMan::scrollDisplayX(int16 dx, uint16 x1, uint16 y1, uint16 x2, uint16 y2, byte *buffer) {
	Image im(_vm);
	im._imageData = buffer;

	if (x1 > x2)
		SWAP<uint16>(x1, x2);

	if (y1 > y2)
		SWAP<uint16>(y1, y2);

	if (dx > 0) {
		im._width = x2 - x1 + 1 - dx;
		im._height = y2 - y1 + 1;

		im.readScreenImage(x1, y1);
		im.drawImage(x1 + dx, y1);

		setPen(0);
		rectFill(x1, y1, x1 + dx - 1, y2);
	} else if (dx < 0) {
		im._width = x2 - x1 + 1 + dx;
		im._height = y2 - y1 + 1;

		im.readScreenImage(x1 - dx, y1);
		im.drawImage(x1, y1);

		setPen(0);
		rectFill(x2 + dx + 1, y1, x2, y2);
	}

	// Prevent the Image destructor from deleting the external buffer
	im._imageData = nullptr;
}

void DisplayMan::scrollDisplayY(int16 dy, uint16 x1, uint16 y1, uint16 x2, uint16 y2, byte *buffer) {
	Image im(_vm);
	im._imageData = buffer;

	if (x1 > x2)
		SWAP<uint16>(x1, x2);

	if (y1 > y2)
		SWAP<uint16>(y1, y2);

	if (dy > 0) {
		im._width = x2 - x1 + 1;
		im._height = y2 - y1 + 1 - dy;

		im.readScreenImage(x1, y1);
		im.drawImage(x1, y1 + dy);

		setPen(0);
		rectFill(x1, y1, x2, y1 + dy - 1);
	} else if (dy < 0) {
		im._width = x2 - x1 + 1;
		im._height = y2 - y1 + 1 + dy;

		im.readScreenImage(x1, y1 - dy);
		im.drawImage(x1, y1);

		setPen(0);
		rectFill(x1, y2 + dy + 1, x2, y2);
	}

	// Prevent the Image destructor from deleting the external buffer
	im._imageData = nullptr;
}

uint16 DisplayMan::fadeNumIn(uint16 num, uint16 res, uint16 counter) {
	return (num - ((((int32)(15 - counter)) * ((int32)(num - res))) / 15));
}

uint16 DisplayMan::fadeNumOut(uint16 num, uint16 res, uint16 counter) {
	return (num - ((((int32) counter) * ((int32)(num - res))) / 15));
}

void DisplayMan::fade(bool fadeIn) {
	uint16 newPal[16];

	for (int i = 0; i < 16; i++) {
		for (int palIdx = 0; palIdx < 16; palIdx++) {
			if (fadeIn)
				newPal[palIdx] = (0x00F & fadeNumIn(0x00F & _fadePalette[palIdx], 0, i)) +
				(0x0F0 & fadeNumIn(0x0F0 & _fadePalette[palIdx], 0, i)) +
				(0xF00 & fadeNumIn(0xF00 & _fadePalette[palIdx], 0, i));
			else
				newPal[palIdx] = (0x00F & fadeNumOut(0x00F & _fadePalette[palIdx], 0, i)) +
				(0x0F0 & fadeNumOut(0x0F0 & _fadePalette[palIdx], 0, i)) +
				(0xF00 & fadeNumOut(0xF00 & _fadePalette[palIdx], 0, i));
		}

		setAmigaPal(newPal);
		_vm->waitTOF();
		_vm->updateMusicAndEvents();
	}
}

} // End of namespace Lab
