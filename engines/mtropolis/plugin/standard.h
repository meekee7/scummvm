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

#ifndef MTROPOLIS_PLUGIN_STANDARD_H
#define MTROPOLIS_PLUGIN_STANDARD_H

#include "mtropolis/modifiers.h"
#include "mtropolis/modifier_factory.h"
#include "mtropolis/runtime.h"
#include "mtropolis/plugin/standard_data.h"

namespace MTropolis {

class Runtime;

namespace Standard {

class StandardPlugIn;

class CursorModifier : public Modifier {
public:
	CursorModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::CursorModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;
	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Cursor Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	Event _applyWhen;
	Event _removeWhen;
	uint32 _cursorID;
};

// This appears to be basically a duplicate of scene transition modifier, except unlike that,
// its parameters are controllable via script, and the duration scaling appears to be different
// (probably 600000 max rate instead of 6000000)
class STransCtModifier : public Modifier {
public:
	static const int32 kMaxDuration = 600000;

	STransCtModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::STransCtModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;
	void disable(Runtime *runtime) override;

	bool readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) override;
	MiniscriptInstructionOutcome writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "STransCt Scene Transition Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	MiniscriptInstructionOutcome scriptSetRate(MiniscriptThread *thread, const DynamicValue &value);
	MiniscriptInstructionOutcome scriptSetSteps(MiniscriptThread *thread, const DynamicValue &value);

	Event _enableWhen;
	Event _disableWhen;

	int32 _transitionType;
	int32 _transitionDirection;
	int32 _steps;
	int32 _duration;
	bool _fullScreen;
};

class MediaCueMessengerModifier : public Modifier, public IMediaCueModifier {
public:
	MediaCueMessengerModifier();
	~MediaCueMessengerModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::MediaCueMessengerModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;
	void disable(Runtime *runtime) override;

	Modifier *getMediaCueModifier() override;
	Common::WeakPtr<Modifier> getMediaCueTriggerSource() const override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Media Cue Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
#endif

private:
	enum CueSourceType {
		kCueSourceInteger,
		kCueSourceIntegerRange,
		kCueSourceVariableReference,
		kCueSourceLabel,
		kCueSourceString,

		kCueSourceInvalid = -1,
	};

	struct CueSourceUnion {
		CueSourceUnion();
		~CueSourceUnion();

		union {
			int32 asInt;
			IntRange asIntRange;
			uint32 asVarRefGUID;
			Label asLabel;
			uint64 asUnset;
		};
		Common::String asString; //String object in the union would prevent copyability

		template<class T, T (CueSourceUnion::*TMember)>
		void construct(const T &value);

		template<class T, T (CueSourceUnion::*TMember)>
		void destruct();
	};

	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	void linkInternalReferences(ObjectLinkingScope *scope) override;
	void visitInternalReferences(IStructuralReferenceVisitor *visitor) override;

	CueSourceType _cueSourceType;
	CueSourceUnion _cueSource;

	Common::WeakPtr<Modifier> _cueSourceModifier;

	Event _enableWhen;
	Event _disableWhen;

	MediaCueState _mediaCue;
	bool _isActive;
};

class ObjectReferenceVariableModifier : public VariableModifier {
public:
	ObjectReferenceVariableModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::ObjectReferenceVariableModifier &data);

	bool varSetValue(MiniscriptThread *thread, const DynamicValue &value) override;
	void varGetValue(DynamicValue &dest) const override;

	bool readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) override;
	MiniscriptInstructionOutcome writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Object Reference Variable Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	struct ObjectWriteInterface {
		static MiniscriptInstructionOutcome write(MiniscriptThread *thread, const DynamicValue &dest, void *objectRef, uintptr ptrOrOffset);
		static MiniscriptInstructionOutcome refAttrib(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib);
		static MiniscriptInstructionOutcome refAttribIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib, const DynamicValue &index);
	};

	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	MiniscriptInstructionOutcome scriptSetPath(MiniscriptThread *thread, const DynamicValue &value);
	MiniscriptInstructionOutcome scriptSetObject(MiniscriptThread *thread, const DynamicValue &value);
	MiniscriptInstructionOutcome scriptObjectRefAttrib(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, const Common::String &attrib);
	MiniscriptInstructionOutcome scriptObjectRefAttribIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, const Common::String &attrib, const DynamicValue &index);

	void resolve(Runtime *runtime);
	void resolveRelativePath(RuntimeObject *obj, const Common::String &path, size_t startPos);
	void resolveAbsolutePath(Runtime *runtime);

	static bool computeObjectPath(RuntimeObject *obj, Common::String &outPath);
	static RuntimeObject *getObjectParent(RuntimeObject *obj);

	Event _setToSourceParentWhen;
};

class ObjectReferenceVariableStorage : public VariableStorage {
public:
	friend class ObjectReferenceVariableModifier;

	ObjectReferenceVariableStorage();

	Common::SharedPtr<ModifierSaveLoad> getSaveLoad(Runtime *runtime) override;

	Common::SharedPtr<VariableStorage> clone() const override;

private:
	class SaveLoad : public ModifierSaveLoad {
	public:
		explicit SaveLoad(ObjectReferenceVariableStorage *storage);

	private:
		void commitLoad() const override;
		void saveInternal(Common::WriteStream *stream) const override;
		bool loadInternal(Common::ReadStream *stream, uint32 saveFileVersion) override;

		ObjectReferenceVariableStorage *_storage;
		Common::String _objectPath;
	};

	Common::String _fullPath;
	Common::String _objectPath;
	mutable ObjectReference _object;
};

class ListVariableStorage : public VariableStorage {
public:
	friend class ListVariableModifier;

	ListVariableStorage();

	Common::SharedPtr<ModifierSaveLoad> getSaveLoad(Runtime *runtime) override;

	Common::SharedPtr<VariableStorage> clone() const override;

private:
	class SaveLoad : public ModifierSaveLoad {
	public:
		explicit SaveLoad(ListVariableStorage *storage);

	private:
		void commitLoad() const override;
		void saveInternal(Common::WriteStream *stream) const override;
		bool loadInternal(Common::ReadStream *stream, uint32 saveFileVersion) override;

		static void recursiveWriteList(DynamicList *list, Common::WriteStream *stream);
		static Common::SharedPtr<DynamicList> recursiveReadList(Common::ReadStream *stream);

		ListVariableStorage *_storage;
		Common::SharedPtr<DynamicList> _list;
	};

	Common::SharedPtr<DynamicList> _list;
	DynamicValueTypes::DynamicValueType _preferredContentType;
};

class ListVariableModifier : public VariableModifier {
public:
	ListVariableModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::ListVariableModifier &data);

	bool varSetValue(MiniscriptThread *thread, const DynamicValue &value) override;
	void varGetValue(DynamicValue &dest) const override;

	bool isListVariable() const override;

	bool readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) override;
	bool readAttributeIndexed(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib, const DynamicValue &index) override;
	MiniscriptInstructionOutcome writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib) override;
	MiniscriptInstructionOutcome writeRefAttributeIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib, const DynamicValue &index) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "List Variable Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	MiniscriptInstructionOutcome scriptSetCount(MiniscriptThread *thread, const DynamicValue &value);

	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class SysInfoModifier : public Modifier {
public:
	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::SysInfoModifier &data);

	bool readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) override;

	void disable(Runtime *runtime) override {}

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "System Info Modifier"; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

struct StandardPlugInHacks {
	StandardPlugInHacks();

	// If list mod values are illegible, just ignore them and flag it as garbled.
	// Necessary to load object 00788ab9 (olL437Check) in Obsidian, which is supposed to be a list of
	// 4 lists that are 3-size each, in the persistent data, but actually contains 4 identical values
	// that appear to be garbage.
	bool allowGarbledListModData;
};

class PanningModifier : public Modifier {
public:
	PanningModifier();
	~PanningModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::PanningModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Panning Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class FadeModifier : public Modifier {
public:
	FadeModifier();
	~FadeModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::FadeModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

	//bool isVariable() const override { return true; }

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Fade Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class OpenTitleModifier : public Modifier {
public:
	OpenTitleModifier();
	~OpenTitleModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::OpenTitleModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

	// bool isVariable() const override { return true; }

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Open Title Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class NavigateModifier : public Modifier {
public:
	NavigateModifier();
	~NavigateModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::NavigateModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Navigate Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class ConductorModifier : public Modifier {
public:
	ConductorModifier();
	~ConductorModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::ConductorModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Conductor Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class AlphaMaticModifier : public Modifier {
public:
	AlphaMaticModifier();
	~AlphaMaticModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::AlphaMaticModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "AlphaMatic Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class KeyStateModifier : public Modifier {
public:
	KeyStateModifier();
	~KeyStateModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::KeyStateModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

	// bool isVariable() const override { return true; }

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "KeyState Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class PrintModifier : public Modifier {
public:
	PrintModifier();
	~PrintModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::PrintModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

	// bool isVariable() const override { return true; }

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Print Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class RotatorModifier : public Modifier {
public:
	RotatorModifier();
	~RotatorModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::RotatorModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Rotator Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class TrackerModifier : public Modifier {
public:
	TrackerModifier();
	~TrackerModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::TrackerModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Tracker Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class MouseTrapModifier : public Modifier {
public:
	MouseTrapModifier();
	~MouseTrapModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::MouseTrapModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "MouseTrap Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class DoubleClickModifier : public Modifier {
public:
	DoubleClickModifier();
	~DoubleClickModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::DoubleClickModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "DoubleClick Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class WrapAroundModifier : public Modifier {
public:
	WrapAroundModifier();
	~WrapAroundModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::WrapAroundModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "WrapAround Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class EasyScrollerModifier : public Modifier {
public:
	EasyScrollerModifier();
	~EasyScrollerModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::EasyScrollerModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "EasyScroller Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class GoThereModifier : public Modifier {
public:
	GoThereModifier();
	~GoThereModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::GoThereModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "GoThere Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class RandomizerModifier : public Modifier {
public:
	RandomizerModifier();
	~RandomizerModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::RandomizerModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Randomizer Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class TimeLoopModifier : public Modifier {
public:
	TimeLoopModifier();
	~TimeLoopModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::TimeLoopModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "TimeLoop Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class BitmapVariableModifier : public Modifier {
public:
	BitmapVariableModifier();
	~BitmapVariableModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::BitmapVariableModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "BitmapVariable Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class CaptureBitmapModifier : public Modifier {
public:
	CaptureBitmapModifier();
	~CaptureBitmapModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::CaptureBitmapModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "CaptureBitmap Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class ImportBitmapModifier : public Modifier {
public:
	ImportBitmapModifier();
	~ImportBitmapModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::ImportBitmapModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "ImportBitmap Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class DisplayBitmapModifier : public Modifier {
public:
	DisplayBitmapModifier();
	~DisplayBitmapModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::DisplayBitmapModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "DisplayBitmap Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class ScaleBitmapModifier : public Modifier {
public:
	ScaleBitmapModifier();
	~ScaleBitmapModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::ScaleBitmapModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "ScaleBitmap Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class SaveBitmapModifier : public Modifier {
public:
	SaveBitmapModifier();
	~SaveBitmapModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::SaveBitmapModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "SaveBitmap Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class PrintBitmapModifier : public Modifier {
public:
	PrintBitmapModifier();
	~PrintBitmapModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::PrintBitmapModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "PrintBitmap Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class PainterModifier : public Modifier {
public:
	PainterModifier();
	~PainterModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::PainterModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Painter Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class MotionModifier : public Modifier {
public:
	MotionModifier();
	~MotionModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::MotionModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Motion Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class SparkleModifier : public Modifier {
public:
	SparkleModifier();
	~SparkleModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::SparkleModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Sparkle Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class StrUtilModifier : public Modifier {
public:
	StrUtilModifier();
	~StrUtilModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::StrUtilModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "StrUtil Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class ThighBlasterModifier : public Modifier {
public:
	ThighBlasterModifier();
	~ThighBlasterModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::ThighBlasterModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "ThighBlaster Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class AlienWriterModifier : public Modifier {
public:
	AlienWriterModifier();
	~AlienWriterModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::AlienWriterModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "AlienWriter Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class MlineLauncherModifier : public Modifier {
public:
	MlineLauncherModifier();
	~MlineLauncherModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::MlineLauncherModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "mLine Launcher Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};


class StandardPlugIn : public MTropolis::PlugIn {
public:
	StandardPlugIn();
	~StandardPlugIn();

	void registerModifiers(IPlugInModifierRegistrar *registrar) const override;

	const StandardPlugInHacks &getHacks() const;
	StandardPlugInHacks &getHacks();

private:
	PlugInModifierFactory<CursorModifier, Data::Standard::CursorModifier> _cursorModifierFactory;
	PlugInModifierFactory<STransCtModifier, Data::Standard::STransCtModifier> _sTransCtModifierFactory;
	PlugInModifierFactory<MediaCueMessengerModifier, Data::Standard::MediaCueMessengerModifier> _mediaCueModifierFactory;
	PlugInModifierFactory<ObjectReferenceVariableModifier, Data::Standard::ObjectReferenceVariableModifier> _objRefVarModifierFactory;
	PlugInModifierFactory<ListVariableModifier, Data::Standard::ListVariableModifier> _listVarModifierFactory;
	PlugInModifierFactory<SysInfoModifier, Data::Standard::SysInfoModifier> _sysInfoModifierFactory;
	PlugInModifierFactory<PrintModifier, Data::Standard::PrintModifier> _printModifierFactory;
	PlugInModifierFactory<PanningModifier, Data::Standard::PanningModifier> _panningModifierFactory;
	PlugInModifierFactory<FadeModifier, Data::Standard::FadeModifier> _fadeModifierFactory;
	PlugInModifierFactory<OpenTitleModifier, Data::Standard::OpenTitleModifier> _openTitleModifierFactory;
	PlugInModifierFactory<NavigateModifier, Data::Standard::NavigateModifier> _navigateModifierFactory;
	PlugInModifierFactory<ConductorModifier, Data::Standard::ConductorModifier> _conductorModifierFactory;
	PlugInModifierFactory<AlphaMaticModifier, Data::Standard::AlphaMaticModifier> _alphaMaticModifierFactory;
	PlugInModifierFactory<KeyStateModifier, Data::Standard::KeyStateModifier> _keyStateModifierFactory;
	PlugInModifierFactory<RotatorModifier, Data::Standard::RotatorModifier> _rotatorModifierFactory;
	PlugInModifierFactory<TrackerModifier, Data::Standard::TrackerModifier> _trackerModifierFactory;
	PlugInModifierFactory<DoubleClickModifier, Data::Standard::DoubleClickModifier> _doubleClickModifierFactory;
	PlugInModifierFactory<MouseTrapModifier, Data::Standard::MouseTrapModifier> _mouseTrapModifierFactory;
	PlugInModifierFactory<WrapAroundModifier, Data::Standard::WrapAroundModifier> _wrapAroundModifierFactory;
	PlugInModifierFactory<EasyScrollerModifier, Data::Standard::EasyScrollerModifier> _easyScrollerModifierFactory;
	PlugInModifierFactory<GoThereModifier, Data::Standard::GoThereModifier> _goThereModifierFactory;
	PlugInModifierFactory<RandomizerModifier, Data::Standard::RandomizerModifier> _randomizerModifierFactory;
	PlugInModifierFactory<TimeLoopModifier, Data::Standard::TimeLoopModifier> _timeLoopModifierFactory;
	PlugInModifierFactory<BitmapVariableModifier, Data::Standard::BitmapVariableModifier> _bitmapVariableModifierFactory;
	PlugInModifierFactory<CaptureBitmapModifier, Data::Standard::CaptureBitmapModifier> _captureBitmapModifierFactory;
	PlugInModifierFactory<ImportBitmapModifier, Data::Standard::ImportBitmapModifier> _importBitmapModifierFactory;
	PlugInModifierFactory<DisplayBitmapModifier, Data::Standard::DisplayBitmapModifier> _displayBitmapModifierFactory;
	PlugInModifierFactory<ScaleBitmapModifier, Data::Standard::ScaleBitmapModifier> _scaleBitmapModifierFactory;
	PlugInModifierFactory<SaveBitmapModifier, Data::Standard::SaveBitmapModifier> _saveBitmapModifierFactory;
	PlugInModifierFactory<PrintBitmapModifier, Data::Standard::PrintBitmapModifier> _printBitmapModifierFactory;
	PlugInModifierFactory<PainterModifier, Data::Standard::PainterModifier> _painterModifierFactory;
	PlugInModifierFactory<MotionModifier, Data::Standard::MotionModifier> _motionModifierFactory;
	PlugInModifierFactory<SparkleModifier, Data::Standard::SparkleModifier> _sparkleModifierFactory;
	PlugInModifierFactory<StrUtilModifier, Data::Standard::StrUtilModifier> _strUtilModifierFactory;
	PlugInModifierFactory<ThighBlasterModifier, Data::Standard::ThighBlasterModifier> _thighBlasterModifierFactory;
	PlugInModifierFactory<AlienWriterModifier, Data::Standard::AlienWriterModifier> _alienWriterModifierFactory;
	PlugInModifierFactory<MlineLauncherModifier, Data::Standard::MlineLauncherModifier> _mlineLauncherModifierFactory;

	StandardPlugInHacks _hacks;
};

} // End of namespace Standard

} // End of namespace MTropolis

#endif
