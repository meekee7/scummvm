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
 * MIT License:
 *
 * Copyright (c) 2009 Alexei Svitkine, Eugene Sandulenko
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "common/file.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/config-manager.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/translation.h"

#include "gui/saveload.h"

#include "graphics/thumbnail.h"
#include "graphics/surface.h"

#include "wage/wage.h"
#include "wage/world.h"
#include "wage/entities.h"

#define SAVEGAME_CURRENT_VERSION 1

//
// Original saves format is supported.
// ScummVM adds flags, description and thumbnail
// in the end of the file (shouldn't make saves incompatible).
//
// Version 0 (original/ScummVM):  first ScummVM version
//

namespace Wage {

static const uint32 WAGEflag = MKTAG('W', 'A', 'G', 'E');

//TODO: make sure these are calculated right: (we add flag, description, etc)
#define VARS_INDEX 0x005E
#define SCENES_INDEX 0x0232

#define SCENE_SIZE 0x0010
#define CHR_SIZE 0x0016
#define OBJ_SIZE 0x0010

#define GET_HEX_OFFSET(ptr, baseOffset, entrySize) ((ptr) == nullptr ? -1 : ((baseOffset) + (entrySize) * (ptr)->_index))
#define GET_HEX_CHR_OFFSET(ptr) GET_HEX_OFFSET((ptr), chrsHexOffset, CHR_SIZE)
#define GET_HEX_OBJ_OFFSET(ptr) GET_HEX_OFFSET((ptr), objsHexOffset, OBJ_SIZE)
#define GET_HEX_SCENE_OFFSET(ptr) ((ptr) == nullptr ? -1 : (SCENES_INDEX + getSceneIndex(_world->_player->_currentScene) * SCENE_SIZE))

int WageEngine::getSceneIndex(Scene *scene) const {
	assert(scene);
	Common::Array<Scene *> &orderedScenes = _world->_orderedScenes;
	for (uint32 i = 0; i < orderedScenes.size(); ++i) {
		if (orderedScenes[i] == scene) return i;
	}

	warning("Scene's index not found");
	return -1;
}

int WageEngine::saveGame(const Common::String &fileName, const Common::String &descriptionString) {
	Common::OutSaveFile *out;
	int result = 0;

	debug(9, "WageEngine::saveGame(%s, %s)", fileName.c_str(), descriptionString.c_str());
	if (!(out = _saveFileMan->openForSaving(fileName))) {
		warning("Can't create file '%s', game not saved", fileName.c_str());
		return -1;
	} else {
		debug(9, "Successfully opened %s for writing", fileName.c_str());
	}

	// Counters
	out->writeSint16LE(_world->_scenes.size()); //numScenes
	out->writeSint16LE(_world->_chrs.size()); //numChars
	out->writeSint16LE(_world->_objs.size()); //numObjs

	// Hex Offsets
	int chrsHexOffset = SCENES_INDEX + _world->_scenes.size() * SCENE_SIZE; //chrs start after scenes
	int objsHexOffset = chrsHexOffset + _world->_chrs.size() * CHR_SIZE; //objs start after chrs
	out->writeSint32LE(chrsHexOffset);
	out->writeSint32LE(objsHexOffset);

	// Unique 8-byte World Signature
	out->writeSint32LE(_world->_signature); //8-byte ints? seriously? (uses 4 bytes in java code too)

	Chr *player = _world->_player;
	Context &playerContext = player->_context;

	// More Counters
	out->writeSint32LE(playerContext._visits); //visitNum
	out->writeSint32LE(_loopCount); //loopNum
	out->writeSint32LE(playerContext._kills); //killNum

	// Hex offset to player character
	out->writeSint32LE(GET_HEX_CHR_OFFSET(player)); //getPlayerHexOffset() == getHexOffsetForChr(player)

	// character in this scene?
	out->writeSint32LE(GET_HEX_CHR_OFFSET(_monster)); //getPresCharHexOffset() == getHexOffsetForChr(monster)

	// Hex offset to current scene
	out->writeSint32LE(GET_HEX_SCENE_OFFSET(player->_currentScene)); //getCurSceneHexOffset()

	// wearing a helmet?
	out->writeSint32LE(GET_HEX_OBJ_OFFSET(player->_armor[Chr::ChrArmorType::HEAD_ARMOR])); //helmetIndex

	// holding a shield?
	out->writeSint32LE(GET_HEX_OBJ_OFFSET(player->_armor[Chr::ChrArmorType::SHIELD_ARMOR])); //shieldIndex

	// wearing chest armor?
	out->writeSint32LE(GET_HEX_OBJ_OFFSET(player->_armor[Chr::ChrArmorType::BODY_ARMOR])); //chestArmIndex

	// wearing spiritual armor?
	out->writeSint32LE(GET_HEX_OBJ_OFFSET(player->_armor[Chr::ChrArmorType::MAGIC_ARMOR])); //sprtArmIndex

	// TODO:
	out->writeSint16LE(0xffff);	// ???? - always FFFF
	out->writeSint16LE(0xffff);	// ???? - always FFFF
	out->writeSint16LE(0xffff);	// ???? - always FFFF
	out->writeSint16LE(0xffff);	// ???? - always FFFF

	// did a character just escape?
	out->writeSint32LE(GET_HEX_CHR_OFFSET(_running)); //getRunCharHexOffset() == getHexOffsetForChr(running)

	// players experience points
	out->writeSint32LE(playerContext._experience);

	out->writeSint16LE(_aim); //aim
	out->writeSint16LE(_opponentAim); //opponentAim

	// TODO:
	out->writeSint16LE(0x0000);	// always 0
	out->writeSint16LE(0x0000);	// always 0
	out->writeSint16LE(0x0000);	// always 0

	// Base character stats
	out->writeByte(playerContext._statVariables[PHYS_STR_BAS]); //state.getBasePhysStr()
	out->writeByte(playerContext._statVariables[PHYS_HIT_BAS]); //state.getBasePhysHp()
	out->writeByte(playerContext._statVariables[PHYS_ARM_BAS]); //state.getBasePhysArm()
	out->writeByte(playerContext._statVariables[PHYS_ACC_BAS]); //state.getBasePhysAcc()
	out->writeByte(playerContext._statVariables[SPIR_STR_BAS]); //state.getBaseSprtStr()
	out->writeByte(playerContext._statVariables[SPIR_HIT_BAS]); //state.getBaseSprtHp()
	out->writeByte(playerContext._statVariables[SPIR_ARM_BAS]); //state.getBaseSprtArm()
	out->writeByte(playerContext._statVariables[SPIR_ACC_BAS]); //state.getBaseSprtAcc()
	out->writeByte(playerContext._statVariables[PHYS_SPE_BAS]); //state.getBaseRunSpeed()

	// TODO:
	out->writeByte(0x0A);		// ???? - always seems to be 0x0A

	// write user vars
	for (uint32 i = 0; i < 26 * 9; ++i)
		out->writeSint16LE(playerContext._userVariables[i]);
	
	// write updated info for all scenes
	Common::Array<Scene *> &orderedScenes = _world->_orderedScenes;
	for (uint32 i = 0; i < orderedScenes.size(); ++i) {
		Scene *scene = orderedScenes[i];
		if (scene != _world->_storageScene) {
			out->writeSint16LE(scene->_resourceId);
			out->writeSint16LE(scene->_worldY);
			out->writeSint16LE(scene->_worldX);
			out->writeByte(scene->_blocked[NORTH] ? 0x01 : 0x00);
			out->writeByte(scene->_blocked[SOUTH] ? 0x01 : 0x00);
			out->writeByte(scene->_blocked[EAST] ? 0x01 : 0x00);
			out->writeByte(scene->_blocked[WEST] ? 0x01 : 0x00);
			out->writeSint16LE(scene->_soundFrequency);
			out->writeByte(scene->_soundType);
			// the following two bytes are currently unknown
			out->writeByte(0);
			out->writeByte(0);
			out->writeByte(scene->_visited ? 0x01 : 0x00);
		}
	}

	// write updated info for all characters
	Common::Array<Chr *> &orderedChrs = _world->_orderedChrs;
	for (uint32 i = 0; i < orderedChrs.size(); ++i) {
		Chr *chr = orderedChrs[i];
		out->writeSint16LE(chr->_resourceId);
		out->writeSint16LE(chr->_currentScene->_resourceId);
		Context &chrContext = chr->_context;
		out->writeByte(chrContext._statVariables[PHYS_STR_CUR]);
		out->writeByte(chrContext._statVariables[PHYS_HIT_CUR]);
		out->writeByte(chrContext._statVariables[PHYS_ARM_CUR]);
		out->writeByte(chrContext._statVariables[PHYS_ACC_CUR]);
		out->writeByte(chrContext._statVariables[SPIR_STR_CUR]);
		out->writeByte(chrContext._statVariables[SPIR_HIT_CUR]);
		out->writeByte(chrContext._statVariables[SPIR_ARM_CUR]);
		out->writeByte(chrContext._statVariables[SPIR_ACC_CUR]);
		out->writeByte(chrContext._statVariables[PHYS_SPE_CUR]);
		out->writeByte(chr->_rejectsOffers);
		out->writeByte(chr->_followsOpponent);
		// bytes 16-20 are unknown
		out->writeByte(0);
		out->writeByte(0);
		out->writeByte(0);
		out->writeByte(0);
		out->writeByte(0);
		out->writeByte(chr->_weaponDamage1);
		out->writeByte(chr->_weaponDamage2);
	}

	// write updated info for all objects
	Common::Array<Obj *> &orderedObjs = _world->_orderedObjs;
	for (uint32 i = 0; i < orderedObjs.size(); ++i) {
		Obj *obj = orderedObjs[i];
		Scene *location = obj->_currentScene;
		Chr *owner = obj->_currentOwner;

		out->writeSint16LE(obj->_resourceId);
		out->writeSint16LE(location == nullptr ? 0 : location->_resourceId);
		out->writeSint16LE(owner == nullptr ? 0 : owner->_resourceId);

		// bytes 7-9 are unknown (always = 0)
		out->writeByte(0);
		out->writeByte(0);
		out->writeByte(0);

		out->writeByte(obj->_accuracy);
		out->writeByte(obj->_value);
		out->writeByte(obj->_type);
		out->writeByte(obj->_damage);
		out->writeByte(obj->_attackType);
		out->writeSint16LE(obj->_numberOfUses);
	}

	// the following is appended by ScummVM
	out->writeUint32BE(WAGEflag);

	// Write description of saved game, limited to WAGE_SAVEDGAME_DESCRIPTION_LEN characters + terminating NUL
	const int WAGE_SAVEDGAME_DESCRIPTION_LEN = 127;
	char description[WAGE_SAVEDGAME_DESCRIPTION_LEN + 1];

	memset(description, 0, sizeof(description));
	strncpy(description, descriptionString.c_str(), WAGE_SAVEDGAME_DESCRIPTION_LEN);
	assert(WAGE_SAVEDGAME_DESCRIPTION_LEN + 1 == 128); // safety
	out->write(description, 128);

	out->writeByte(SAVEGAME_CURRENT_VERSION);
	debug(9, "Writing save game version (%d)", SAVEGAME_CURRENT_VERSION);

	// Thumbnail
	Graphics::saveThumbnail(*out);

	// this one to make checking easier:
	// it couldn't be added to the beginning
	// and we won't be able to find it in the middle,
	// so these would be the last 4 bytes of the file
	out->writeUint32BE(WAGEflag);

	out->finalize();
	if (out->err()) {
		warning("Can't write file '%s'. (Disk full?)", fileName.c_str());
		result = -1;
	} else
		debug(9, "Saved game %s in file %s", descriptionString.c_str(), fileName.c_str());

	delete out;
	return result;
}

int WageEngine::loadGame(int slotId) {
	Common::InSaveFile *data;
	Common::String fileName = getSavegameFilename(slotId);

	debug(9, "WageEngine::loadGame(%d)", slotId);
	if (!(data = _saveFileMan->openForLoading(fileName))) {
		warning("Can't open file '%s', game not loaded", fileName.c_str());
		return -1;
	} else {
		debug(9, "Successfully opened %s for reading", fileName.c_str());
	}

	// Counters
	int numScenes = data->readSint16LE();
	int numChars = data->readSint16LE();
	int numObjs = data->readSint16LE();

	// Hex Offsets
	int chrsHexOffset = data->readSint32LE();
	int objsHexOffset = data->readSint32LE();

	// Unique 8-byte World Signature
	_world->_signature = data->readSint32LE();

	//Chr *player = _world->_player;
	//Context &playerContext = player->_context;

	// More Counters
	int visitNum = data->readSint32LE(); //visitNum @ playerContext._visits
	_loopCount = data->readSint32LE(); //loopNum
	int killNum = data->readSint32LE(); //killNum @ playerContext._kills

	// Hex offset to player character
	int playerOffset = data->readSint32LE();

	// character in this scene?
	int presCharOffset = data->readSint32LE();

	// Hex offset to current scene
	int currentSceneOffset = data->readSint32LE();

	// wearing a helmet?
	int helmetOffset = data->readSint32LE(); //helmetIndex @ player->_armor[Chr::ChrArmorType::HEAD_ARMOR]

	// holding a shield?
	int shieldOffset = data->readSint32LE(); //shieldIndex @ player->_armor[Chr::ChrArmorType::SHIELD_ARMOR]

	// wearing chest armor?
	int armorOffset = data->readSint32LE(); //chestArmIndex @ player->_armor[Chr::ChrArmorType::BODY_ARMOR]

	// wearing spiritual armor?
	int spiritualArmorOffset = data->readSint32LE(); //sprtArmIndex @ player->_armor[Chr::ChrArmorType::MAGIC_ARMOR]

	data->readSint16LE();	// FFFF
	data->readSint16LE();	// FFFF
	data->readSint16LE();	// FFFF
	data->readSint16LE();	// FFFF

	int runCharOffset = data->readSint32LE();

	// players experience points
	int exp = data->readSint32LE(); // @ playerContext._experience

	_aim = data->readSint16LE(); //aim
	_opponentAim = data->readSint16LE(); //opponentAim

	data->readSint16LE(); // 0000
	data->readSint16LE(); // 0000
	data->readSint16LE(); // 0000

	// Base character stats
	int basePhysStr = data->readByte(); // @ playerContext._statVariables[PHYS_STR_BAS]
	int basePhysHp = data->readByte(); // @ playerContext._statVariables[PHYS_HIT_BAS]
	int basePhysArm = data->readByte(); // @ playerContext._statVariables[PHYS_ARM_BAS]
	int basePhysAcc = data->readByte(); // @ playerContext._statVariables[PHYS_ACC_BAS]
	int baseSprtStr = data->readByte(); // @ playerContext._statVariables[SPIR_STR_BAS]
	int baseSprtHp = data->readByte(); // @ playerContext._statVariables[SPIR_HIT_BAS]
	int baseSprtArm = data->readByte(); // @ playerContext._statVariables[SPIR_ARM_BAS]
	int baseSprtAcc = data->readByte(); // @ playerContext._statVariables[SPIR_ACC_BAS]
	int baseRunSpeed = data->readByte(); // @ playerContext._statVariables[PHYS_SPE_BAS]

	data->readByte(); // 0x0A?

	// write user vars
	for (uint32 i = 0; i < 26 * 9; ++i)
		data->readSint16LE(); // @ playerContext._userVariables[i]

	// write updated info for all scenes	
	for (uint32 i = 0; i < numScenes; ++i) {		
		data->readSint16LE(); // @ scene->_resourceId
		data->readSint16LE(); // @ scene->_worldY
		data->readSint16LE(); // @ scene->_worldX
		data->readByte(); // @ scene->_blocked[NORTH]
		data->readByte(); // @ scene->_blocked[SOUTH]
		data->readByte(); // @ scene->_blocked[EAST]
		data->readByte(); // @ scene->_blocked[WEST]
		data->readSint16LE(); // @ scene->_soundFrequency
		data->readByte(); // @ scene->_soundType
		// the following two bytes are currently unknown
		data->readByte();
		data->readByte();
		data->readByte(); // @ scene->_visited
	}

	// write updated info for all characters
	Common::Array<Chr *> &orderedChrs = _world->_orderedChrs;
	for (uint32 i = 0; i < orderedChrs.size(); ++i) {
		int resourceId = data->readSint16LE();
		int sceneResourceId = data->readSint16LE();

		int strength = data->readByte(); // @ chrContext._statVariables[PHYS_STR_CUR]
		int hp = data->readByte(); // @ chrContext._statVariables[PHYS_HIT_CUR]
		int armor = data->readByte(); // @ chrContext._statVariables[PHYS_ARM_CUR]
		int accuracy = data->readByte(); // @ chrContext._statVariables[PHYS_ACC_CUR]
		int spirStrength = data->readByte(); // @ chrContext._statVariables[SPIR_STR_CUR]
		int spirHp = data->readByte(); // @ chrContext._statVariables[SPIR_HIT_CUR]
		int spirArmor = data->readByte(); // @ chrContext._statVariables[SPIR_ARM_CUR]
		int spirAccuracy = data->readByte(); // @ chrContext._statVariables[SPIR_ACC_CUR]
		int speed = data->readByte(); // @ chrContext._statVariables[PHYS_SPE_CUR]
		int rejectsOffers = data->readByte(); // @ chr->_rejectsOffers
		int followsOpponent = data->readByte(); // @ chr->_followsOpponent

		// bytes 16-20 are unknown
		data->readByte();
		data->readByte();
		data->readByte();
		data->readByte();
		data->readByte();

		data->readByte(); // @ chr->_weaponDamage1
		data->readByte(); // @ chr->_weaponDamage2
	}

	// write updated info for all objects
	for (uint32 i = 0; i < numObjs; ++i) {
		int resourceId = data->readSint16LE();
		int locationResourceId = data->readSint16LE();
		int ownerResourceId = data->readSint16LE();

		// bytes 7-9 are unknown (always = 0)
		data->readByte();
		data->readByte();
		data->readByte();

		data->readByte(); // @ obj->_accuracy
		data->readByte(); // @ obj->_value
		data->readByte(); // @ obj->_type
		data->readByte(); // @ obj->_damage
		data->readByte(); // @ obj->_attackType
		data->readSint16LE(); // @ obj->_numberOfUses
	}

	// the following is appended by ScummVM
	if (data->pos() < data->size()) {
		int scummvmWageFlag = data->readUint32BE();

		if (scummvmWageFlag != WAGEflag) {
			warning("Extra bytes after original save's information found, but that's not ScummVM's");
			delete data;
			return 0;
		}

		// Write description of saved game, limited to WAGE_SAVEDGAME_DESCRIPTION_LEN characters + terminating NUL
		const int WAGE_SAVEDGAME_DESCRIPTION_LEN = 127;
		char description[WAGE_SAVEDGAME_DESCRIPTION_LEN + 1];
		data->read(description, 128);
		if (description[WAGE_SAVEDGAME_DESCRIPTION_LEN] != 0) {
			warning("Description's last byte is not '\0'");
			description[WAGE_SAVEDGAME_DESCRIPTION_LEN] = 0;
		}

		int version = data->readByte();
		if (version != SAVEGAME_CURRENT_VERSION) {
			warning("Reading version %d while current is %d", version, SAVEGAME_CURRENT_VERSION);
		}

		// Thumbnail
		Graphics::loadThumbnail(*data);
	}

	delete data;
	return 0;
}

Common::String WageEngine::getSavegameFilename(int16 slotId) const {
	Common::String saveLoadSlot = _targetName;
	saveLoadSlot += Common::String::format(".%.3d", slotId);
	return saveLoadSlot;
}

Common::Error WageEngine::saveGameState(int slot, const Common::String &description) {
	Common::String saveLoadSlot = getSavegameFilename(slot);
	if (saveGame(saveLoadSlot, description) == 0)
		return Common::kNoError;
	else
		return Common::kUnknownError;
}

} // End of namespace Agi
