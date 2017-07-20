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

#include "engines/util.h"

#include "sludge/backdrop.h"
#include "sludge/freeze.h"
#include "sludge/graphics.h"
#include "sludge/newfatal.h"
#include "sludge/sludge.h"
#include "sludge/sludger.h"
#include "sludge/sprites.h"
#include "sludge/zbuffer.h"

namespace Sludge {

extern InputType input;

GraphicsManager::GraphicsManager(SludgeEngine *vm) {
	_vm = vm;

	// Init screen surface
	_winWidth = _sceneWidth = 640;
	_winHeight = _sceneHeight = 480;

	// LightMap
	_lightMapMode = LIGHTMAPMODE_PIXEL;
	_lightMapNumber = 0;

	// Parallax
	_parallaxStuff = new Parallax;

	// Camera
	_cameraZoom = 1.0;
	_cameraX = _cameraY = 0;

	// Freeze
	_frozenStuff = nullptr;

	// Back drop
	_backdropExists = false;

	// Sprites
	_spriteLayers = new SpriteLayers;
	_spriteLayers->numLayers = 0;

	// ZBuffer
	_zBuffer = new ZBufferData;
	_zBuffer->originalNum = -1;
	_zBuffer->sprites = nullptr;

	// Colors
	_currentBlankColour = _renderSurface.format.ARGBToColor(255, 0, 0, 0);
	_currentBurnR = 0;
	_currentBurnG = 0;
	_currentBurnB = 0;
}

GraphicsManager::~GraphicsManager() {
	// kill parallax
	killParallax();
	delete _parallaxStuff;
	_parallaxStuff = nullptr;

	// kill frozen stuff
	FrozenStuffStruct *killMe = _frozenStuff;
	while (killMe) {
		_frozenStuff = _frozenStuff->next;
		if (killMe->backdropSurface.getPixels())
			killMe->backdropSurface.free();
		if (killMe->lightMapSurface.getPixels())
			killMe->lightMapSurface.free();
		delete killMe;
		killMe = nullptr;
		killMe = _frozenStuff;
	}

	// kill sprite layers
	killSpriteLayers();
	delete _spriteLayers;
	_spriteLayers = nullptr;

	// kill zbuffer
	killZBuffer();
	delete _zBuffer;
	_zBuffer = nullptr;

	// kill surfaces
	if (_renderSurface.getPixels())
		_renderSurface.free();

	if (_snapshotSurface.getPixels())
		_snapshotSurface.free();

	if (_backdropSurface.getPixels())
		_backdropSurface.free();

	if (_origBackdropSurface.getPixels())
		_origBackdropSurface.free();
}

bool GraphicsManager::init() {
	initGraphics(_winWidth, _winHeight, true, _vm->getScreenPixelFormat());
	_renderSurface.create(_winWidth, _winHeight, *_vm->getScreenPixelFormat());

	if (!killResizeBackdrop(_winWidth, _winHeight))
		return fatal("Couldn't allocate memory for backdrop");

	return true;
}

void GraphicsManager::display() {
	g_system->copyRectToScreen((byte *)_renderSurface.getPixels(), _renderSurface.pitch, 0, 0, _renderSurface.w, _renderSurface.h);
	g_system->updateScreen();
}

bool GraphicsManager::loadParallax(uint16 v, uint16 fracX, uint16 fracY) {
	if (!_parallaxStuff)
		_parallaxStuff = new Parallax;
	return _parallaxStuff->add(v, fracX, fracY);
}

void GraphicsManager::killParallax() {
	if (!_parallaxStuff)
		return;
	_parallaxStuff->kill();
}

void GraphicsManager::saveParallax(Common::WriteStream *fp) {
	if (_parallaxStuff)
		_parallaxStuff->save(fp);
}

void GraphicsManager::drawParallax() {
	if (_parallaxStuff)
		_parallaxStuff->draw();
}

void GraphicsManager::aimCamera(int cameraX, int cameraY) {
	_cameraX = cameraX;
	_cameraY = cameraY;
	_cameraX -= (float)(_winWidth >> 1) / _cameraZoom;
	_cameraY -= (float)(_winHeight >> 1) / _cameraZoom;

	if (_cameraX < 0)
		_cameraX = 0;
	else if (_cameraX > _sceneWidth - (float)_winWidth / _cameraZoom)
		_cameraX = _sceneWidth - (float)_winWidth / _cameraZoom;
	if (_cameraY < 0)
		_cameraY = 0;
	else if (_cameraY > _sceneHeight - (float)_winHeight / _cameraZoom)
		_cameraY = _sceneHeight - (float)_winHeight / _cameraZoom;
}

void GraphicsManager::zoomCamera(int z) {
	input.mouseX = input.mouseX * _cameraZoom;
	input.mouseY = input.mouseY * _cameraZoom;

	_cameraZoom = (float)z * 0.01;
	if ((float)_winWidth / _cameraZoom > _sceneWidth)
		_cameraZoom = (float)_winWidth / _sceneWidth;
	if ((float)_winHeight / _cameraZoom > _sceneHeight)
		_cameraZoom = (float)_winHeight / _sceneHeight;

	input.mouseX = input.mouseX / _cameraZoom;
	input.mouseY = input.mouseY / _cameraZoom;
}

void GraphicsManager::saveColors(Common::WriteStream *stream) {
	stream->writeUint16BE(_currentBlankColour);
	stream->writeByte(_currentBurnR);
	stream->writeByte(_currentBurnG);
	stream->writeByte(_currentBurnB);
}

void GraphicsManager::loadColors(Common::SeekableReadStream *stream) {
	_currentBlankColour = stream->readUint16BE();
	_currentBurnR = stream->readByte();
	_currentBurnG = stream->readByte();
	_currentBurnB = stream->readByte();
}

} // End of namespace Sludge
