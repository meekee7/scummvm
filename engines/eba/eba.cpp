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

#include "eba/eba.h"
#include "eba/detection.h"
#include "eba/console.h"
#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/compression/unzip.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/paletteman.h"

namespace Eba {

EbaEngine *g_engine;

EbaEngine::EbaEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc), _randomSource("Eba") {
	g_engine = this;
}

EbaEngine::~EbaEngine() {
}

uint32 EbaEngine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String EbaEngine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Error EbaEngine::run() {
	// Initialize 800x600 paletted graphics mode
	const auto pixelFormat = Graphics::PixelFormat(4,8,8,8,8,24,16,8,0);

	initGraphics(800, 600, &pixelFormat);


	// Set the engine's debugger console
	setDebugger(new Console());

	runGame();

	return Common::kNoError;
}

Common::Error EbaEngine::syncGame(Common::Serializer &s) {
	// The Serializer has methods isLoading() and isSaving()
	// if you need to specific steps; for example setting
	// an array size after reading it's length, whereas
	// for saving it would write the existing array's length
	int dummy = 0;
	s.syncAsUint32LE(dummy);

	return Common::kNoError;
}

} // End of namespace Eba
