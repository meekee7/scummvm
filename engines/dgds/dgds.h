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

#ifndef DGDS_DGDS_H
#define DGDS_DGDS_H

#include "common/error.h"
#include "common/platform.h"

#include "graphics/surface.h"
#include "graphics/managed_surface.h"

#include "engines/advancedDetector.h"
#include "engines/engine.h"

#include "gui/debugger.h"

namespace Dgds {

class Console;
class ResourceManager;
class Decompressor;
class Image;
class PFont;
class FFont;
class Scene;

class DgdsMidiPlayer;
struct DgdsADS;

enum DgdsGameId {
	GID_DRAGON,
	GID_CHINA,
	GID_BEAMISH
};

class DgdsEngine : public Engine {
public:
	Common::Platform _platform;

private:
	Console *_console;
	DgdsMidiPlayer *_midiPlayer;

	ResourceManager *_resource;
	Decompressor *_decompressor;

	DgdsGameId _gameId;
	Graphics::Surface _bottomBuffer;
	Graphics::Surface _topBuffer;
	Scene *_scene;

	PFont *_fntP;
	FFont *_fntF;
	Common::StringArray _BMPs;
	uint32 _musicSize;
	byte *_musicData;
	Common::SeekableReadStream *_soundData;

protected:
	virtual Common::Error run();

public:
	DgdsEngine(OSystem *syst, const ADGameDescription *gameDesc);
	virtual ~DgdsEngine();

	DgdsGameId getGameId() { return _gameId; }

    void playSfx(const Common::String &fileName, byte channel, byte volume);
    void stopSfx(byte channel);

	bool playPCM(const byte *data, uint32 size);
	void playMusic(const Common::String &fileName);

	void parseFile(const Common::String &filename, int resource = 0);

	Graphics::Surface &getTopBuffer() { return _topBuffer; }
	Graphics::Surface &getBottomBuffer() { return _bottomBuffer; }
	Common::SeekableReadStream *getResource(const Common::String &name, bool ignorePatches);
	ResourceManager *getResourceManager() { return _resource; }
	Decompressor *getDecompressor() { return _decompressor; }
	const Scene *getScene() const { return _scene; }

	const PFont *getFntP() const { return _fntP; }
	Image *_image;
	Graphics::ManagedSurface _resData;

private:
	void parseFileInner(Common::Platform platform, Common::SeekableReadStream &file, const char *name, int resource, Decompressor *decompressor);
};

//void explode(Common::Platform platform, const char *indexName, const char *fileName, int resource);

} // End of namespace Dgds

#endif // DGDS_DGDS_H
