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

#include "mtropolis/plugin/standard_data.h"
#include "mtropolis/plugin/standard.h"

namespace MTropolis {

namespace Data {

namespace Standard {

CursorModifier::CursorModifier() : haveRemoveWhen(false) {
}

DataReadErrorCode CursorModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 0 && prefix.plugInRevision != 1 && prefix.plugInRevision != 2)
		return kDataReadErrorUnsupportedRevision;

	if (!applyWhen.load(reader))
		return kDataReadErrorReadFailed;

	if (prefix.plugInRevision >= 1) {
		if (!removeWhen.load(reader))
			return kDataReadErrorReadFailed;
		haveRemoveWhen = true;
	} else {
		removeWhen.type = PlugInTypeTaggedValue::kNull;
		haveRemoveWhen = false;
	}

	if (!cursorIDAsLabel.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

DataReadErrorCode STransCtModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 0)
		return kDataReadErrorUnsupportedRevision;

	if (!enableWhen.load(reader) || !disableWhen.load(reader) || !transitionType.load(reader) ||
		!transitionDirection.load(reader) || !unknown1.load(reader) || !steps.load(reader) ||
		!duration.load(reader) || !fullScreen.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

MediaCueMessengerModifier::MediaCueMessengerModifier()
	: unknown1(0), destination(0), unknown2(0) {
}

DataReadErrorCode MediaCueMessengerModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 1)
		return kDataReadErrorUnsupportedRevision;

	if (!enableWhen.load(reader) || !disableWhen.load(reader) || !sendEvent.load(reader)
		|| !nonStandardMessageFlags.load(reader) || !reader.readU16(unknown1) || !reader.readU32(destination)
		|| !reader.readU32(unknown2) || !with.load(reader) || !executeAt.load(reader)
		|| !triggerTiming.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

DataReadErrorCode ObjectReferenceVariableModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 0 && prefix.plugInRevision != 2)
		return kDataReadErrorUnsupportedRevision;

	if (!setToSourceParentWhen.load(reader))
		return kDataReadErrorReadFailed;

	if (prefix.plugInRevision == 0) {
		unknown1.type = Data::PlugInTypeTaggedValue::kNull;
		if (!objectPath.load(reader))
			return kDataReadErrorReadFailed;
	} else {
		if (!unknown1.load(reader))
			return kDataReadErrorReadFailed;

		bool hasNoPath = (unknown1.type == Data::PlugInTypeTaggedValue::kInteger && unknown1.value.asInt == 0);
		if (hasNoPath)
			objectPath.type = Data::PlugInTypeTaggedValue::kNull;
		else if (!objectPath.load(reader))
			return kDataReadErrorReadFailed;
	}

	return kDataReadErrorNone;
}

ListVariableModifier::ListVariableModifier() : unknown1(0), contentsType(0), unknown2{0, 0, 0, 0},
	havePersistentData(false), numValues(0), values(nullptr), persistentValuesGarbled(false) {
}

ListVariableModifier::~ListVariableModifier() {
	if (values)
		delete[] values;
}

DataReadErrorCode ListVariableModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision < 1 || prefix.plugInRevision > 3)
		return kDataReadErrorUnsupportedRevision;

	int64 privateDataPos = reader.tell();

	if (!reader.readU16(unknown1) || !reader.readU32(contentsType) || !reader.readBytes(unknown2))
		return kDataReadErrorReadFailed;

	persistentValuesGarbled = false;

	if (prefix.plugInRevision == 1 || prefix.plugInRevision == 3) {
		if (prefix.plugInRevision == 1) {
			havePersistentData = true;
		} else if (prefix.plugInRevision == 3) {
			PlugInTypeTaggedValue persistentFlag;
			if (!persistentFlag.load(reader) || persistentFlag.type != PlugInTypeTaggedValue::kBoolean)
				return kDataReadErrorReadFailed;

			havePersistentData = (persistentFlag.value.asBoolean != 0);
		}

		if (havePersistentData) {
			PlugInTypeTaggedValue numValuesVar;
			if (!numValuesVar.load(reader) || numValuesVar.type != PlugInTypeTaggedValue::kInteger || numValuesVar.value.asInt < 0)
				return kDataReadErrorReadFailed;

			numValues = static_cast<uint32>(numValuesVar.value.asInt);

			values = new PlugInTypeTaggedValue[numValues];
			for (size_t i = 0; i < numValues; i++) {
				if (!values[i].load(reader)) {
					if (static_cast<const MTropolis::Standard::StandardPlugIn &>(plugIn).getHacks().allowGarbledListModData) {
						persistentValuesGarbled = true;
						if (!reader.seek(privateDataPos + prefix.subObjectSize))
							return kDataReadErrorReadFailed;
						break;
					} else {
						return kDataReadErrorReadFailed;
					}
				}
			}
		} else {
			numValues = 0;
			values = nullptr;
		}
	} else {
		havePersistentData = false;
		numValues = 0;
		values = nullptr;
	}

	return kDataReadErrorNone;
}

DataReadErrorCode PanningModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 3)
		return kDataReadErrorUnsupportedRevision;

	if (!unknown1Event.load(reader) || !unknown2Event.load(reader) || !unknown3Int.load(reader) || !unknown4Int.load(reader) || !unknown5Int.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

DataReadErrorCode SysInfoModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 0)
		return kDataReadErrorUnsupportedRevision;

	return kDataReadErrorNone;
}

DataReadErrorCode FadeModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 1)
		return kDataReadErrorUnsupportedRevision;

	if (!unknown1Event.load(reader) || !unknown2Event.load(reader) || !unknown3Int.load(reader) || !unknown4Int.load(reader) || !unknown5Int.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

DataReadErrorCode OpenTitleModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 0)
		return kDataReadErrorUnsupportedRevision;

	PlugInTypeTaggedValue values[3];
	if (!unknown1Event.load(reader) || !unknown2String.load(reader) || !unknown3Integer.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

DataReadErrorCode KeyStateModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 0)
		return kDataReadErrorUnsupportedRevision;

	//Seemingly no data to load

	return kDataReadErrorNone;
}

DataReadErrorCode RotatorModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 0)
		return kDataReadErrorUnsupportedRevision;

	if (!unknown1Event.load(reader) || !unknown2Event.load(reader) || !unknown3Bool.load(reader) || !unknown4Int.load(reader) || !unknown5Float.load(reader))
		return kDataReadErrorReadFailed;

	if (!unknown6Bool.load(reader) || !unknown7Point.load(reader) || !unknown8Int.load(reader) || !unknown9Bool.load(reader) || !unknown10Int.load(reader))
		return kDataReadErrorReadFailed;

	if (!unknown11Event.load(reader) || !unknown12Label.load(reader) || !unknown13Null.load(reader) || !unknown14Int.load(reader) || !unknown15Point.load(reader))
		return kDataReadErrorReadFailed;

	if (!unknown16Point.load(reader) || !unknown17Point.load(reader) || !unknown18Bool.load(reader) || !unknown19Point.load(reader) || !unknown20Int.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

DataReadErrorCode TrackerModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 0)
		return kDataReadErrorUnsupportedRevision;

	if (!unknown1Event.load(reader) || !unknown2Event.load(reader) || !unknown3Label.load(reader) || !unknown4Int.load(reader) || !unknown5Int.load(reader))
		return kDataReadErrorReadFailed;

	if (!unknown6Label.load(reader) || !unknown7Bool.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

DataReadErrorCode BitmapVariableModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 0)
		return kDataReadErrorUnsupportedRevision;

	//Seemingly no data to load

	return kDataReadErrorNone;
}

DataReadErrorCode CaptureBitmapModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 0)
		return kDataReadErrorUnsupportedRevision;

	if (!unknown1Event.load(reader) || !unknown2VarRef.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

DataReadErrorCode ImportBitmapModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 0)
		return kDataReadErrorUnsupportedRevision;

	if (!unknown1Event.load(reader) || !unknown2Bool.load(reader) || !unknown3Bool.load(reader) || !unknown4VarRef.load(reader) || !unknown5VarRef.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

DataReadErrorCode DisplayBitmapModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 0)
		return kDataReadErrorUnsupportedRevision;

	if (!unknown1Event.load(reader) || !unknown2VarRef.load(reader) || !unknown3VarRef.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

DataReadErrorCode ScaleBitmapModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 0)
		return kDataReadErrorUnsupportedRevision;

	if (!unknown1Event.load(reader) || !unknown2VarRef.load(reader) || !unknown3IncomingData.load(reader) || !unknown4Int.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

DataReadErrorCode SaveBitmapModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 0)
		return kDataReadErrorUnsupportedRevision;

	if (!unknown1Event.load(reader) || !unknown2VarRef.load(reader) || !unknown3Bool.load(reader) || !unknown4Bool.load(reader) || !unknown5VarRef.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

DataReadErrorCode PrintBitmapModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 0)
		return kDataReadErrorUnsupportedRevision;

	if (!unknown1Event.load(reader) || !unknown2VarRef.load(reader) || !unknown3Bool.load(reader) || !unknown4Bool.load(reader) || !unknown5Int.load(reader))
		return kDataReadErrorReadFailed;

	if (!unknown6Int.load(reader) || !unknown7Null.load(reader) || !unknown8Null.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

DataReadErrorCode DoubleClickModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 0)
		return kDataReadErrorUnsupportedRevision;

	PlugInTypeTaggedValue values[7];
	for (int i = 0; i < ARRAYSIZE(values); ++i) {
		if (!values[i].load(reader))
			return kDataReadErrorReadFailed;
	}

	return kDataReadErrorNone;
}

DataReadErrorCode MouseTrapModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 0)
		return kDataReadErrorUnsupportedRevision;

	if (!unknown1UniversalTime.load(reader) || !unknown2Event.load(reader) || !unknown3Event.load(reader) || !unknown4Event.load(reader) || !unknown5Null.load(reader))
		return kDataReadErrorReadFailed;

	if (!unknown6Label.load(reader) || !unknown7Int.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

DataReadErrorCode WrapAroundModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 1)
		return kDataReadErrorUnsupportedRevision;

	if (!unknown1Event.load(reader) || !unknown2Event.load(reader) || !unknown3Point.load(reader))
		return kDataReadErrorReadFailed;

	if (!unknown4Int.load(reader) || !unknown5Bool.load(reader) || !unknown6Int.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

DataReadErrorCode EasyScrollerModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 1)
		return kDataReadErrorUnsupportedRevision;

	if (!unknown1Event.load(reader) || !unknown2Event.load(reader) || !unknown3Int.load(reader) || !unknown4Int.load(reader) || !unknown5Label.load(reader))
		return kDataReadErrorReadFailed;

	if (!unknown6Int.load(reader) || !unknown7Int.load(reader) || !unknown8Int.load(reader) || !unknown9Int.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

DataReadErrorCode TimeLoopModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 0)
		return kDataReadErrorUnsupportedRevision;

	if (!unknown1Event.load(reader) || !unknown2Event.load(reader) || !unknown3Event.load(reader) || !unknown4Null.load(reader) || !unknown5Label.load(reader))
		return kDataReadErrorReadFailed;

	if (!unknown6Int.load(reader) || !unknown7Int.load(reader) || !unknown8UniversalTime.load(reader) || !unknown9Bool.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

DataReadErrorCode PrintModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 0)
		return kDataReadErrorUnsupportedRevision;

	if (!unknown1Event.load(reader) || !unknown2Bool.load(reader) || !unknown3Bool.load(reader))
		return kDataReadErrorReadFailed;

	if (!unknown4Bool.load(reader) || !unknown5String.load(reader) || !unknown6Bool.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

DataReadErrorCode PainterModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 0)
		return kDataReadErrorUnsupportedRevision;

	//TODO figure out number of properties
	for (int i = 0; i < 9; ++i) {
		PlugInTypeTaggedValue v;
		if (!v.load(reader))
			return kDataReadErrorReadFailed;
	}

	return kDataReadErrorNone;
}

DataReadErrorCode MotionModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 0)
		return kDataReadErrorUnsupportedRevision;

	if (!unknown1Event.load(reader) || !unknown2Event.load(reader) || !unknown3Point.load(reader) || !unknown4Bool.load(reader) || !unknown5Point.load(reader))
		return kDataReadErrorReadFailed;

	if (!unknown6Int.load(reader) || !unknown7Float.load(reader) || !unknown8Int.load(reader) || !unknown9Event.load(reader) || !unknown10Label.load(reader))
		return kDataReadErrorReadFailed;

	if (!unknown11Null.load(reader) || !unknown12Int.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

DataReadErrorCode SparkleModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 0)
		return kDataReadErrorUnsupportedRevision;

	if (!unknown1Event.load(reader) || !unknown2Event.load(reader) || !unknown3Int.load(reader) || !unknown4Int.load(reader) || !unknown5Int.load(reader))
		return kDataReadErrorReadFailed;

	if (!unknown6Int.load(reader) || !unknown7Int.load(reader) || !unknown8Int.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

DataReadErrorCode StrUtilModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 0)
		return kDataReadErrorUnsupportedRevision;

	// Seemingly no data to load

	return kDataReadErrorNone;
}

DataReadErrorCode AlienWriterModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 1)
		return kDataReadErrorUnsupportedRevision;

	PlugInTypeTaggedValue values[1];
	for (int i = 0; i < ARRAYSIZE(values); ++i) {
		if (!values[i].load(reader))
			return kDataReadErrorReadFailed;
	}

	return kDataReadErrorNone;
}

DataReadErrorCode MlineLauncherModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 0)
		return kDataReadErrorUnsupportedRevision;

	if (!unknown1String.load(reader) || !unknown2String.load(reader) || !unknown3String.load(reader) || !unknown4Int.load(reader) || !unknown5Int.load(reader))
		return kDataReadErrorReadFailed;

	if (!unknown6Bool.load(reader) || !unknown7Event.load(reader) || !unknown8Bool.load(reader) || !unknown9Bool.load(reader) || !unknown10Bool.load(reader))
		return kDataReadErrorReadFailed;

	if (!unknown11Bool.load(reader) || !unknown12Bool.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

DataReadErrorCode GoThereModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 1)
		return kDataReadErrorUnsupportedRevision;

	if (!unknown1Event.load(reader) || !unknown2Event.load(reader) || !unknown3Point.load(reader))
		return kDataReadErrorReadFailed;

	if (!unknown4UniversalTime.load(reader) || !unknown5Event.load(reader) || !unknown6Label.load(reader))
		return kDataReadErrorReadFailed;

	if (!unknown7Null.load(reader) || !unknown8Int.load(reader) || !unknown9Int.load(reader))
		return kDataReadErrorReadFailed;

	if (!unknown10Bool.load(reader) || !unknown11Bool.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

DataReadErrorCode RandomizerModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 0)
		return kDataReadErrorUnsupportedRevision;

	if (!unknown1Int.load(reader) || !unknown2Int.load(reader) || !unknown3Int.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

DataReadErrorCode ThighBlasterModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 1)
		return kDataReadErrorUnsupportedRevision;

	if (!unknown1Int.load(reader))
		return kDataReadErrorReadFailed;

	return kDataReadErrorNone;
}

DataReadErrorCode NavigateModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 0)
		return kDataReadErrorUnsupportedRevision;

	PlugInTypeTaggedValue values[5];
	for (int i = 0; i < ARRAYSIZE(values); ++i) {
		if (!values[i].load(reader))
			return kDataReadErrorReadFailed;
	}

	return kDataReadErrorNone;
}

DataReadErrorCode ConductorModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 1)
		return kDataReadErrorUnsupportedRevision;

	PlugInTypeTaggedValue values[9];
	for (int i = 0; i < ARRAYSIZE(values); ++i) {
		if (!values[i].load(reader))
			return kDataReadErrorReadFailed;
	}

	return kDataReadErrorNone;
}

DataReadErrorCode AlphaMaticModifier::load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) {
	if (prefix.plugInRevision != 1)
		return kDataReadErrorUnsupportedRevision;

	PlugInTypeTaggedValue values[13];
	for (int i = 0; i < ARRAYSIZE(values); ++i) {
		if (!values[i].load(reader))
			return kDataReadErrorReadFailed;
	}

	return kDataReadErrorNone;
}

} // End of namespace Standard

} // End of namespace Data

} // End of namespace MTropolis
