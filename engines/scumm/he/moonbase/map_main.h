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

#ifndef SCUMM_HE_MOONBASE_MAP_MAIN
#define SCUMM_HE_MOONBASE_MAP_MAIN

#ifdef ENABLE_HE

#include "common/stream.h"
#include "common/random.h"

namespace Scumm {

class Map {
public:
	Map(ScummEngine_v100he *vm);
	~Map();

	Common::SeekableReadStream *substituteFile(const byte *fileName);

private:
	ScummEngine_v100he *_vm;

	// We require our own random number generator
	// so we can send and set seeds from online players to ensure
	// they're playing on the same generated map.
	Common::RandomSource _rnd;
};

} // End of namespace Scumm


#endif // ENABLE_HE

#endif // SCUMM_HE_MOONBASE_MAP_MAIN
