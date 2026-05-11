/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "arcatera/arcatera.h"
#include "arcatera/detection.h"
#include "arcatera/console.h"
#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/paletteman.h"

namespace Arcatera {

ArcateraEngine *g_engine;

ArcateraEngine::ArcateraEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc), _randomSource("Arcatera") {
	g_engine = this;
}

ArcateraEngine::~ArcateraEngine() {
}

uint32 ArcateraEngine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String ArcateraEngine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Error ArcateraEngine::run() {
	//Graphics::PixelFormat format = Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
	Graphics::PixelFormat format = Graphics::PixelFormat::createFormatRGBA32();
	initGraphics(800, 600, &format);
	
	
	byte palette[256 * 3] = {};
	for (int i = 0; i < 256; i++) {
		palette[i] = (byte)i;
		palette[i + 256] = (byte)i;
		palette[i + 256 * 2] = (byte)i;
	}

	//g_system->getPaletteManager()->setPalette(palette, 0, 256);
	

	// Set the engine's debugger console
	setDebugger(new Console());

	runGame();

	return Common::kNoError;
}

Common::Error ArcateraEngine::syncGame(Common::Serializer &s) {
	// The Serializer has methods isLoading() and isSaving()
	// if you need to specific steps; for example setting
	// an array size after reading its length, whereas
	// for saving it would write the existing array's length
	int dummy = 0;
	s.syncAsUint32LE(dummy);

	return Common::kNoError;
}

} // End of namespace Arcatera
