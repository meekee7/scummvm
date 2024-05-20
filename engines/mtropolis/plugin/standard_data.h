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

#ifndef MTROPOLIS_PLUGIN_STANDARD_DATA_H
#define MTROPOLIS_PLUGIN_STANDARD_DATA_H

#include "mtropolis/data.h"

namespace MTropolis {

namespace Data {

namespace Standard {

struct CursorModifier : public PlugInModifierData {
	CursorModifier();

	bool haveRemoveWhen;

	PlugInTypeTaggedValue applyWhen;
	PlugInTypeTaggedValue removeWhen;
	PlugInTypeTaggedValue cursorIDAsLabel;

protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct STransCtModifier : public PlugInModifierData {
	PlugInTypeTaggedValue enableWhen;  // Event
	PlugInTypeTaggedValue disableWhen;	// Event
	PlugInTypeTaggedValue transitionType;	// int
	PlugInTypeTaggedValue transitionDirection;	// int
	PlugInTypeTaggedValue unknown1; // int, seems to always be 1
	PlugInTypeTaggedValue steps;	// int, seems to always be 32
	PlugInTypeTaggedValue duration;    // int, always observed as 60000
	PlugInTypeTaggedValue fullScreen; // bool

protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct MediaCueMessengerModifier : public PlugInModifierData {
	enum MessageFlags {
		kMessageFlagImmediate = 0x1,
		kMessageFlagCascade = 0x2,
		kMessageFlagRelay = 0x4,
	};

	enum TriggerTiming {
		kTriggerTimingStart = 0,
		kTriggerTimingDuring = 1,
		kTriggerTimingEnd = 2,
	};

	MediaCueMessengerModifier();

	PlugInTypeTaggedValue enableWhen;
	PlugInTypeTaggedValue disableWhen;
	PlugInTypeTaggedValue sendEvent;
	PlugInTypeTaggedValue nonStandardMessageFlags;	// int type, non-standard
	uint16 unknown1;
	uint32 destination;
	uint32 unknown2;
	PlugInTypeTaggedValue with;
	PlugInTypeTaggedValue executeAt;	// May validly be a label, variable, integer, or integer range
	PlugInTypeTaggedValue triggerTiming;	// int type

protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct ObjectReferenceVariableModifier : public PlugInModifierData {
	PlugInTypeTaggedValue setToSourceParentWhen;
	PlugInTypeTaggedValue unknown1;
	PlugInTypeTaggedValue objectPath;

protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct ListVariableModifier : public PlugInModifierData {
	enum ContentsType {
		kContentsTypeInteger = 1,
		kContentsTypePoint = 2,
		kContentsTypeRange = 3,
		kContentsTypeFloat = 4,
		kContentsTypeString = 5,
		kContentsTypeObject = 6,
		kContentsTypeVector = 8,
		kContentsTypeBoolean = 9,
	};

	ListVariableModifier();
	~ListVariableModifier();

	uint16 unknown1;
	uint32 contentsType;
	uint8 unknown2[4];

	bool havePersistentData;
	uint32 numValues;
	PlugInTypeTaggedValue *values;

	bool persistentValuesGarbled;

protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct SysInfoModifier : public PlugInModifierData {
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct PanningModifier : public PlugInModifierData {
	PlugInTypeTaggedValue unknown1Event; // Probably "Enable When"
	PlugInTypeTaggedValue unknown2Event; // Probably "Disable When"
	PlugInTypeTaggedValue unknown3Int;   // Int
	PlugInTypeTaggedValue unknown4Int;   // Int
	PlugInTypeTaggedValue unknown5Int;   // Int

protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct FadeModifier : public PlugInModifierData {
	PlugInTypeTaggedValue unknown1Event;
	PlugInTypeTaggedValue unknown2Event;
	PlugInTypeTaggedValue unknown3Int;
	PlugInTypeTaggedValue unknown4Int;
	PlugInTypeTaggedValue unknown5Int;
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct OpenTitleModifier : public PlugInModifierData {
	PlugInTypeTaggedValue unknown1Event;
	PlugInTypeTaggedValue unknown2String;
	PlugInTypeTaggedValue unknown3Integer;
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct NavigateModifier : public PlugInModifierData {
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct ConductorModifier : public PlugInModifierData {
	PlugInTypeTaggedValue unknown1Null;
	PlugInTypeTaggedValue unknown2Null;
	PlugInTypeTaggedValue unknown3Int;
	PlugInTypeTaggedValue unknown4Null;
	PlugInTypeTaggedValue unknown5Null;
	PlugInTypeTaggedValue unknown6Null;
	PlugInTypeTaggedValue unknown7Event;
	PlugInTypeTaggedValue unknown8Null;
	PlugInTypeTaggedValue unknown9Int;
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct AlphaMaticModifier : public PlugInModifierData {
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct KeyStateModifier : public PlugInModifierData {
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct RotatorModifier : public PlugInModifierData {
	PlugInTypeTaggedValue unknown1Event;
	PlugInTypeTaggedValue unknown2Event;
	PlugInTypeTaggedValue unknown3Bool;
	PlugInTypeTaggedValue unknown4Int;
	PlugInTypeTaggedValue unknown5Float;
	PlugInTypeTaggedValue unknown6Bool;
	PlugInTypeTaggedValue unknown7Point;
	PlugInTypeTaggedValue unknown8Int;
	PlugInTypeTaggedValue unknown9Bool;
	PlugInTypeTaggedValue unknown10Int;
	PlugInTypeTaggedValue unknown11Event;
	PlugInTypeTaggedValue unknown12Label;
	PlugInTypeTaggedValue unknown13Null;
	PlugInTypeTaggedValue unknown14Int;
	PlugInTypeTaggedValue unknown15Point;
	PlugInTypeTaggedValue unknown16Point;
	PlugInTypeTaggedValue unknown17Point;
	PlugInTypeTaggedValue unknown18Bool;
	PlugInTypeTaggedValue unknown19Point;
	PlugInTypeTaggedValue unknown20Int;
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct TrackerModifier : public PlugInModifierData {
	PlugInTypeTaggedValue unknown1Event;
	PlugInTypeTaggedValue unknown2Event;
	PlugInTypeTaggedValue unknown3Label;
	PlugInTypeTaggedValue unknown4Int;
	PlugInTypeTaggedValue unknown5Int;
	PlugInTypeTaggedValue unknown6Label;
	PlugInTypeTaggedValue unknown7Bool;
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct BitmapVariableModifier : public PlugInModifierData {
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct CaptureBitmapModifier : public PlugInModifierData {
	PlugInTypeTaggedValue unknown1Event;
	PlugInTypeTaggedValue unknown2VarRef;
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct ImportBitmapModifier : public PlugInModifierData {
	PlugInTypeTaggedValue unknown1Event;
	PlugInTypeTaggedValue unknown2Bool;
	PlugInTypeTaggedValue unknown3Bool;
	PlugInTypeTaggedValue unknown4VarRef;
	PlugInTypeTaggedValue unknown5VarRef;
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct DisplayBitmapModifier : public PlugInModifierData {
	PlugInTypeTaggedValue unknown1Event;
	PlugInTypeTaggedValue unknown2VarRef;
	PlugInTypeTaggedValue unknown3VarRef;
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct ScaleBitmapModifier : public PlugInModifierData {
	PlugInTypeTaggedValue unknown1Event;
	PlugInTypeTaggedValue unknown2VarRef;
	PlugInTypeTaggedValue unknown3IncomingData;
	PlugInTypeTaggedValue unknown4Int;
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct SaveBitmapModifier : public PlugInModifierData {
	PlugInTypeTaggedValue unknown1Event;
	PlugInTypeTaggedValue unknown2VarRef;
	PlugInTypeTaggedValue unknown3Bool;
	PlugInTypeTaggedValue unknown4Bool;
	PlugInTypeTaggedValue unknown5VarRef;
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct PrintBitmapModifier : public PlugInModifierData {
	PlugInTypeTaggedValue unknown1Event;
	PlugInTypeTaggedValue unknown2VarRef;
	PlugInTypeTaggedValue unknown3Bool;
	PlugInTypeTaggedValue unknown4Bool;
	PlugInTypeTaggedValue unknown5Int;
	PlugInTypeTaggedValue unknown6Int;
	PlugInTypeTaggedValue unknown7Null;
	PlugInTypeTaggedValue unknown8Null;
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct DoubleClickModifier : public PlugInModifierData {
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct MouseTrapModifier : public PlugInModifierData {
	PlugInTypeTaggedValue unknown1UniversalTime;
	PlugInTypeTaggedValue unknown2Event;
	PlugInTypeTaggedValue unknown3Event;
	PlugInTypeTaggedValue unknown4Event;
	PlugInTypeTaggedValue unknown5Null;
	PlugInTypeTaggedValue unknown6Label;
	PlugInTypeTaggedValue unknown7Int;
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct WrapAroundModifier : public PlugInModifierData {
	PlugInTypeTaggedValue unknown1Event;
	PlugInTypeTaggedValue unknown2Event;
	PlugInTypeTaggedValue unknown3Point;
	PlugInTypeTaggedValue unknown4Int;
	PlugInTypeTaggedValue unknown5Bool;
	PlugInTypeTaggedValue unknown6Int;
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct EasyScrollerModifier : public PlugInModifierData {
	PlugInTypeTaggedValue unknown1Event;
	PlugInTypeTaggedValue unknown2Event;
	PlugInTypeTaggedValue unknown3Int;
	PlugInTypeTaggedValue unknown4Int;
	PlugInTypeTaggedValue unknown5Label;
	PlugInTypeTaggedValue unknown6Int;
	PlugInTypeTaggedValue unknown7Int;
	PlugInTypeTaggedValue unknown8Int;
	PlugInTypeTaggedValue unknown9Int;
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct TimeLoopModifier : public PlugInModifierData {
	PlugInTypeTaggedValue unknown1Event;
	PlugInTypeTaggedValue unknown2Event;
	PlugInTypeTaggedValue unknown3Event;
	PlugInTypeTaggedValue unknown4Null;
	PlugInTypeTaggedValue unknown5Label;
	PlugInTypeTaggedValue unknown6Int;
	PlugInTypeTaggedValue unknown7Int;
	PlugInTypeTaggedValue unknown8UniversalTime;
	PlugInTypeTaggedValue unknown9Bool;
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct PrintModifier : public PlugInModifierData {
	PlugInTypeTaggedValue unknown1Event;
	PlugInTypeTaggedValue unknown2Bool;
	PlugInTypeTaggedValue unknown3Bool;
	PlugInTypeTaggedValue unknown4Bool;
	PlugInTypeTaggedValue unknown5String;
	PlugInTypeTaggedValue unknown6Bool;
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct PainterModifier : public PlugInModifierData {
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct MotionModifier : public PlugInModifierData {
	PlugInTypeTaggedValue unknown1Event;
	PlugInTypeTaggedValue unknown2Event;
	PlugInTypeTaggedValue unknown3Point;
	PlugInTypeTaggedValue unknown4Bool;
	PlugInTypeTaggedValue unknown5Point;
	PlugInTypeTaggedValue unknown6Int;
	PlugInTypeTaggedValue unknown7Float;
	PlugInTypeTaggedValue unknown8Int;
	PlugInTypeTaggedValue unknown9Event;
	PlugInTypeTaggedValue unknown10Label;
	PlugInTypeTaggedValue unknown11Null;
	PlugInTypeTaggedValue unknown12Int;
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct SparkleModifier : public PlugInModifierData {
	PlugInTypeTaggedValue unknown1Event;
	PlugInTypeTaggedValue unknown2Event;
	PlugInTypeTaggedValue unknown3Int;
	PlugInTypeTaggedValue unknown4Int;
	PlugInTypeTaggedValue unknown5Int;
	PlugInTypeTaggedValue unknown6Int;
	PlugInTypeTaggedValue unknown7Int;
	PlugInTypeTaggedValue unknown8Int;
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct StrUtilModifier : public PlugInModifierData {
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct AlienWriterModifier : public PlugInModifierData {
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct MlineLauncherModifier : public PlugInModifierData {
	PlugInTypeTaggedValue unknown1String;
	PlugInTypeTaggedValue unknown2String;
	PlugInTypeTaggedValue unknown3String;
	PlugInTypeTaggedValue unknown4Int;
	PlugInTypeTaggedValue unknown5Int;
	PlugInTypeTaggedValue unknown6Bool;
	PlugInTypeTaggedValue unknown7Event;
	PlugInTypeTaggedValue unknown8Bool;
	PlugInTypeTaggedValue unknown9Bool;
	PlugInTypeTaggedValue unknown10Bool;
	PlugInTypeTaggedValue unknown11Bool;
	PlugInTypeTaggedValue unknown12Bool;
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct GoThereModifier : public PlugInModifierData {
	PlugInTypeTaggedValue unknown1Event;
	PlugInTypeTaggedValue unknown2Event;
	PlugInTypeTaggedValue unknown3Point;
	PlugInTypeTaggedValue unknown4UniversalTime;
	PlugInTypeTaggedValue unknown5Event;
	PlugInTypeTaggedValue unknown6Label;
	PlugInTypeTaggedValue unknown7Null;
	PlugInTypeTaggedValue unknown8Int;
	PlugInTypeTaggedValue unknown9Int;
	PlugInTypeTaggedValue unknown10Bool;
	PlugInTypeTaggedValue unknown11Bool;
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct RandomizerModifier : public PlugInModifierData {
	PlugInTypeTaggedValue unknown1Int;
	PlugInTypeTaggedValue unknown2Int;
	PlugInTypeTaggedValue unknown3Int;
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct ThighBlasterModifier : public PlugInModifierData {
	PlugInTypeTaggedValue unknown1Int;
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

} // End of namespace Standard

} // End of namespace Data

} // End of namespace MTropolis

#endif
