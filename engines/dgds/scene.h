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

#ifndef DGDS_SCENE_H
#define DGDS_SCENE_H

#include "common/stream.h"
#include "common/array.h"

#include "dgds/dialog.h"
#include "dgds/dgds_rect.h"

namespace Dgds {

class ResourceManager;
class Decompressor;
class Font;

enum SceneCondition {
	kSceneCondNone = 0,
	kSceneCondLessThan = 1,
	kSceneCondEqual = 2,
	kSceneCondNegate = 4,
	kSceneCondAbsVal = 8,
	kSceneCondAlwaysTrue = 0x10,
	kSceneCondNeedItemField14 = 0x20,
	kSceneCondNeedItemField12 = 0x40,
	kSceneCondSceneState = 0x80
};

struct SceneConditions {
	uint16 _num;
	SceneCondition _flags; /* eg, see usage in FUN_1f1a_2106 */
	uint16 _val;

	Common::String dump(const Common::String &indent) const;
};

struct HotArea {
	DgdsRect rect;
	uint16 _num; //
	uint16 _cursorNum;
	Common::Array<struct SceneConditions> enableConditions;
	Common::Array<struct SceneOp> opList1;
	Common::Array<struct SceneOp> opList2;
	Common::Array<struct SceneOp> onClickOps;

	virtual Common::String dump(const Common::String &indent) const;
};

enum SceneOpCode {
	kSceneOpNone = 0,
	kSceneOpChangeScene = 1,  	// args: scene num
	kSceneOpNoop = 2,		 	// args: none. Maybe should close dialogue?
	kSceneOpGlobal = 3,			// args: array of uints
	kSceneOpSegmentStateOps = 4,	// args: array of uint pairs [op seg, op seg], term with 0,0 that modify segment states
	kSceneOpSetItemAttr = 5,	// args: [item num, item param 0x28, item param 0x2c]. set item attrs?
	kSceneOpGiveItem = 6,		// args: item num. give item?
	kSceneOp7 = 7,				// args: none.
	kSceneOpShowDlg = 8,		// args: dialogue number. show dialogue?
	kSceneOp9 = 9,				// args: none.
	kSceneOp10 = 10,			// args: none. Clean up the hot area list?
	kSceneOpEnableTrigger = 11,	// args: trigger num
	kSceneOpChangeSceneToStored = 12,	// args: none. Change scene to stored number
	kSceneOp13 = 13,			// args: none.
	kSceneOpRestartGame = 14,	// args: none.
	kSceneOp15 = 15,			// args: none.
	kSceneOpShowClock = 16,		// args: none.  set some clock-related globals
	kSceneOpHideClock = 17,		// args: none.  set some clock-related values.
	kSceneOpShowMouse = 18,		// args: none.
	kSceneOpHideMouse = 19,		// args: none.

	// From here on might be game-specific?
	kSceneOp100 = 100,			// args: none.
	kSceneOpMeanwhile = 101,	// args: none. Clears screen and displays "meanwhile".
	kSceneOp102 = 102,			// args: none.
	kSceneOp103 = 103,			// args: none.
	kSceneOp104 = 104,			// args: none.
	kSceneOp105 = 105,			// args: none. Draw some number at 141, 56
	kSceneOp106 = 106,			// args: none. Draw some number at 42, 250
	kSceneOpOpenPlaySkipIntroMenu = 107, // args: none.  Show the "Play Introduction" / "Skip Introduction" menu.  Dragon only??
	kSceneOp108 = 108,			// args: none.
};

struct SceneOp {
	Common::Array<struct SceneConditions> _conditionList;
	Common::Array<uint16> _args;
	SceneOpCode _opCode;

	Common::String dump(const Common::String &indent) const;
};

struct GameItem : public HotArea {
	Common::Array<struct SceneOp> opList4;
	Common::Array<struct SceneOp> opList5;
	uint16 field10_0x24;
	uint16 _iconNum;
	uint16 field12_0x28;
	uint16 _flags;
	uint16 field14_0x2c;

	Common::String dump(const Common::String &indent) const override;
};

struct MouseCursor {
	uint16 _hotX;
	uint16 _hotY;
	uint16 _iconNum;
	// pointer to cursor image
	//Common::SharedPtr<Image> _img;

	Common::String dump(const Common::String &indent) const;
};

struct SceneStruct4 {
	uint16 val1;
	uint16 val2;
	Common::Array<struct SceneOp> opList;

	Common::String dump(const Common::String &indent) const;
};

struct SceneTrigger {
	uint16 _num;
	bool _enabled;
	Common::Array<struct SceneConditions> conditionList;
	Common::Array<struct SceneOp> sceneOpList;

	Common::String dump(const Common::String &indent) const;
};

/* A global value that only applies on a per-SDS-scene,
   but stays with the GDS data as it sticks around during
   the game */
struct PerSceneGlobal {
	uint16 _num;
	uint16 _sceneNo;
	int16 _val;

	Common::String dump(const Common::String &indent) const;
};


/**
 * A scene is described by an SDS file, which points to the ADS script to load
 * and holds the dialog info.
 */
class Scene {
public:
	Scene();
	virtual ~Scene() {};

	virtual bool parse(Common::SeekableReadStream *s) = 0;

	bool isVersionOver(const char *version) const;
	bool isVersionUnder(const char *version) const;

	uint32 getMagic() const { return _magic; }
	const Common::String &getVersion() const { return _version; }
	void runPreTickOps() { runOps(_preTickOps); }
	void runPostTickOps() { runOps(_postTickOps); }

	void mouseMoved(const Common::Point pt);
	void mouseClicked(const Common::Point pt);

protected:
	bool readConditionList(Common::SeekableReadStream *s, Common::Array<SceneConditions> &list) const;
	bool readHotArea(Common::SeekableReadStream *s, HotArea &dst) const;
	bool readHotAreaList(Common::SeekableReadStream *s, Common::Array<HotArea> &list) const;
	bool readGameItemList(Common::SeekableReadStream *s, Common::Array<GameItem> &list) const;
	bool readMouseHotspotList(Common::SeekableReadStream *s, Common::Array<MouseCursor> &list) const;
	bool readStruct4List(Common::SeekableReadStream *s, Common::Array<SceneStruct4> &list) const;
	bool readOpList(Common::SeekableReadStream *s, Common::Array<SceneOp> &list) const;
	bool readDialogList(Common::SeekableReadStream *s, Common::Array<Dialog> &list) const;
	bool readTriggerList(Common::SeekableReadStream *s, Common::Array<SceneTrigger> &list) const;
	bool readDialogActionList(Common::SeekableReadStream *s, Common::Array<DialogAction> &list) const;

	bool runOps(const Common::Array<SceneOp> &ops);
	bool checkConditions(const Common::Array<struct SceneConditions> &cond);

	virtual void enableTrigger(uint16 num) {}
	virtual void showDialog(uint16 num) {}
	virtual void globalOps(const Common::Array<uint16> &args) {}
	virtual void segmentStateOps(const Common::Array<uint16> &args);
	void segmentStateOp9(uint16 arg);
	void segmentStateOp10(uint16 arg);
	void segmentStateOp11(uint16 arg);
	void segmentStateOp12(uint16 arg);

	uint32 _magic;
	Common::String _version;

	Common::Array<struct SceneOp> _preTickOps;
	Common::Array<struct SceneOp> _postTickOps;
};


class GDSScene : public Scene {
public:
	GDSScene();

	bool load(const Common::String &filename, ResourceManager *resourceManager, Decompressor *decompressor);
	bool parse(Common::SeekableReadStream *s) override;
	bool parseInf(Common::SeekableReadStream *s);
	const Common::String &getIconFile() const { return _iconFile; }
	bool readPerSceneGlobals(Common::SeekableReadStream *s);

	uint16 getGlobal(uint num);

	Common::String dump(const Common::String &indent) const;

	void runStartGameOps() { runOps(_startGameOps); }
	void runQuitGameOps() { runOps(_quitGameOps); }
	void runChangeSceneOps() { runOps(_onChangeSceneOps); }
	void globalOps(const Common::Array<uint16> &args) override;
	int16 getGlobal(uint16 num);
	int16 setGlobal(uint16 num, int16 val);

private:
	//byte _unk[32];
	Common::String _iconFile;
	Common::Array<struct GameItem> _gameItems;
	Common::Array<struct SceneOp> _startGameOps;
	Common::Array<struct SceneOp> _quitGameOps;
	Common::Array<struct SceneOp> _onChangeSceneOps;
	Common::Array<struct PerSceneGlobal> _perSceneGlobals;
	Common::Array<struct SceneStruct4> _struct4List1;
	Common::Array<struct SceneStruct4> _struct4List2;
};

class SDSScene : public Scene {
public:
	SDSScene();

	bool load(const Common::String &filename, ResourceManager *resourceManager, Decompressor *decompressor);
	bool parse(Common::SeekableReadStream *s) override;
	void unload();

	const Common::String &getAdsFile() const { return _adsFile; }
	void runEnterSceneOps() { runOps(_enterSceneOps); }
	void runLeaveSceneOps() { runOps(_leaveSceneOps); }
	void checkTriggers();

	int getNum() const { return _num; }
	Common::String dump(const Common::String &indent) const;

	bool checkDialogActive();
	void drawActiveDialogBgs(Graphics::Surface *dst);
	bool drawAndUpdateDialogs(Graphics::Surface *dst);
	bool checkForClearedDialogs();

	void globalOps(const Common::Array<uint16> &args) override;

	void mouseMoved(const Common::Point &pt);
	void mouseClicked(const Common::Point &pt);

private:
	HotArea *findAreaUnderMouse(const Common::Point &pt);
	void enableTrigger(uint16 num) override;
	void showDialog(uint16 num) override;

	int _num;
	Common::Array<struct SceneOp> _enterSceneOps;
	Common::Array<struct SceneOp> _leaveSceneOps;
	//uint _field5_0x12;
	uint _field6_0x14;
	Common::String _adsFile;
	//uint _field8_0x23;
	Common::Array<struct HotArea> _hotAreaList;
	Common::Array<struct SceneStruct4> _struct4List1;
	Common::Array<struct SceneStruct4> _struct4List2;
	//uint _field12_0x2b;
	Common::Array<class Dialog> _dialogs;
	Common::Array<struct SceneTrigger> _triggers;
	//uint _field15_0x33;

	static bool _dlgWithFlagLo8IsClosing;
	static DialogFlags _sceneDialogFlags;
};

} // End of namespace Dgds

#endif // DGDS_SCENE_H
