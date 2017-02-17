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

#ifndef SCI_ENGINE_OBJECT_H
#define SCI_ENGINE_OBJECT_H

#include "common/array.h"
#include "common/serializer.h"
#include "common/textconsole.h"

#include "sci/sci.h"			// for the SCI versions
#include "sci/engine/vm_types.h"	// for reg_t
#include "sci/util.h"

namespace Sci {

class SegManager;

/** Clone has been marked as 'freed' */
enum {
	OBJECT_FLAG_FREED = (1 << 0)
};

enum infoSelectorFlags {
	kInfoFlagClone        = 0x0001,
#ifdef ENABLE_SCI32
	/**
	 * When set, indicates to game scripts that a screen
	 * item can be updated.
	 */
	kInfoFlagViewVisible  = 0x0008,

	/**
	 * When set, the VM object has an associated ScreenItem in
	 * the rendering tree.
	 */
	kInfoFlagViewInserted = 0x0010,
#endif
	kInfoFlagClass        = 0x8000
};

enum ObjectOffsets {
	kOffsetHeaderSize = 6,
	kOffsetHeaderLocalVariables = 0,
	kOffsetHeaderFunctionArea = 2,
	kOffsetHeaderSelectorCounter = 4,

	kOffsetSelectorSegment = 0,
	kOffsetInfoSelectorSci0 = 4,
	kOffsetNamePointerSci0 = 6,
	kOffsetInfoSelectorSci11 = 14,
	kOffsetNamePointerSci11 = 16
};

class Object {
public:
	Object() :
		_offset(getSciVersion() < SCI_VERSION_1_1 ? 0 : 5),
		_flags(0),
		_baseObj(),
		_baseVars(),
		_methodCount(0),
		_propertyOffsetsSci3() {}

	Object &operator=(const Object &other) {
		_baseObj = other._baseObj;
		_baseMethod = other._baseMethod;
		_variables = other._variables;
		_methodCount = other._methodCount;
		_flags = other._flags;
		_offset = other._offset;
		_pos = other._pos;
		_baseVars = other._baseVars;

		if (getSciVersion() == SCI_VERSION_3) {
			_propertyOffsetsSci3 = other._propertyOffsetsSci3;
			_superClassPosSci3 = other._superClassPosSci3;
			_speciesSelectorSci3 = other._speciesSelectorSci3;
			_infoSelectorSci3 = other._infoSelectorSci3;
			_mustSetViewVisible = other._mustSetViewVisible;
		}

		return *this;
	}

	reg_t getSpeciesSelector() const {
		if (getSciVersion() < SCI_VERSION_3)
			return _variables[_offset];
		else	// SCI3
			return _speciesSelectorSci3;
	}

	void setSpeciesSelector(reg_t value) {
		if (getSciVersion() < SCI_VERSION_3)
			_variables[_offset] = value;
		else	// SCI3
			_speciesSelectorSci3 = value;
	}

	reg_t getSuperClassSelector() const {
		if (getSciVersion() < SCI_VERSION_3)
			return _variables[_offset + 1];
		else	// SCI3
			return _superClassPosSci3;
	}

	void setSuperClassSelector(reg_t value) {
		if (getSciVersion() < SCI_VERSION_3)
			_variables[_offset + 1] = value;
		else	// SCI3
			_superClassPosSci3 = value;
	}

	reg_t getInfoSelector() const {
		if (getSciVersion() < SCI_VERSION_3)
			return _variables[_offset + 2];
		else	// SCI3
			return _infoSelectorSci3;
	}

	void setInfoSelector(reg_t info) {
		if (getSciVersion() < SCI_VERSION_3)
			_variables[_offset + 2] = info;
		else	// SCI3
			_infoSelectorSci3 = info;
	}

#ifdef ENABLE_SCI32
	void setInfoSelectorFlag(infoSelectorFlags flag) {
		if (getSciVersion() < SCI_VERSION_3) {
			_variables[_offset + 2] |= flag;
		} else {
			_infoSelectorSci3 |= flag;
		}
	}

	// NOTE: In real engine, -info- is treated as byte size
	void clearInfoSelectorFlag(infoSelectorFlags flag) {
		if (getSciVersion() < SCI_VERSION_3) {
			_variables[_offset + 2] &= ~flag;
		} else {
			_infoSelectorSci3 &= ~flag;
		}
	}

	bool isInserted() const {
		return getInfoSelector().toUint16() & kInfoFlagViewInserted;
	}
#endif

	reg_t getNameSelector() const {
		if (getSciVersion() < SCI_VERSION_3)
			return _offset + 3 < (uint16)_variables.size() ? _variables[_offset + 3] : NULL_REG;
		else	// SCI3
			return _variables.size() ? _variables[0] : NULL_REG;
	}

	// No setter for the name selector

	reg_t getPropDictSelector() const {
		if (getSciVersion() < SCI_VERSION_3)
			return _variables[2];
		else
			// This should never occur, this is called from a SCI1.1 - SCI2.1 only function
			error("getPropDictSelector called for SCI3");
	}

	void setPropDictSelector(reg_t value) {
		if (getSciVersion() < SCI_VERSION_3)
			_variables[2] = value;
		else
			// This should never occur, this is called from a SCI1.1 - SCI2.1 only function
			error("setPropDictSelector called for SCI3");
	}

	reg_t getClassScriptSelector() const {
		if (getSciVersion() < SCI_VERSION_3)
			return _variables[4];
		else	// SCI3
			return make_reg(0, _baseObj.getUint16SEAt(6));
	}

	void setClassScriptSelector(reg_t value) {
		if (getSciVersion() < SCI_VERSION_3)
			_variables[4] = value;
		else	// SCI3
			// This should never occur, this is called from a SCI1.1 - SCI2.1 only function
			error("setClassScriptSelector called for SCI3");
	}

	Selector getVarSelector(uint16 i) const { return _baseVars[i]; }

	reg_t getFunction(uint16 i) const {
		uint16 offset = (getSciVersion() < SCI_VERSION_1_1) ? _methodCount + 1 + i : i * 2 + 2;
		if (getSciVersion() == SCI_VERSION_3)
			offset--;
		return make_reg(_pos.getSegment(), _baseMethod[offset]);
	}

	Selector getFuncSelector(uint16 i) const {
		uint16 offset = (getSciVersion() < SCI_VERSION_1_1) ? i : i * 2 + 1;
		if (getSciVersion() == SCI_VERSION_3)
			offset--;
		return _baseMethod[offset];
	}

	/**
	 * Determines if this object is a class and explicitly defines the
	 * selector as a funcselector. Does NOT say anything about the object's
	 * superclasses, i.e. failure may be returned even if one of the
	 * superclasses defines the funcselector
	 */
	int funcSelectorPosition(Selector sel) const {
		for (uint i = 0; i < _methodCount; i++)
			if (getFuncSelector(i) == sel)
				return i;

		return -1;
	}

	/**
	 * Determines if the object explicitly defines slc as a varselector.
	 * Returns -1 if not found.
	 */
	int locateVarSelector(SegManager *segMan, Selector slc) const;

	bool isClass() const { return (getInfoSelector().getOffset() & kInfoFlagClass); }
	const Object *getClass(SegManager *segMan) const;

	void markAsFreed() { _flags |= OBJECT_FLAG_FREED; }
	bool isFreed() const { return _flags & OBJECT_FLAG_FREED; }

	uint getVarCount() const { return _variables.size(); }

	void init(const SciSpan<const byte> &buf, reg_t obj_pos, bool initVariables = true);

	reg_t getVariable(uint var) const { return _variables[var]; }
	reg_t &getVariableRef(uint var) { return _variables[var]; }

	uint16 getMethodCount() const { return _methodCount; }
	reg_t getPos() const { return _pos; }

	void saveLoadWithSerializer(Common::Serializer &ser);

	void cloneFromObject(const Object *obj) {
		_baseObj = obj ? obj->_baseObj : SciSpan<const byte>();
		_baseMethod = obj ? obj->_baseMethod : Common::Array<uint16>();
		_baseVars = obj ? obj->_baseVars : Common::Array<uint16>();
	}

	bool relocateSci0Sci21(SegmentId segment, int location, size_t scriptSize);
	bool relocateSci3(SegmentId segment, uint32 location, int offset, size_t scriptSize);

	int propertyOffsetToId(SegManager *segMan, int propertyOffset) const;

	void initSpecies(SegManager *segMan, reg_t addr);
	void initSuperClass(SegManager *segMan, reg_t addr);
	bool initBaseObject(SegManager *segMan, reg_t addr, bool doInitSuperClass = true);
	void syncBaseObject(const SciSpan<const byte> &ptr) { _baseObj = ptr; }

#ifdef ENABLE_SCI32
	bool mustSetViewVisible(int selector) const;
#endif

private:
	void initSelectorsSci3(const SciSpan<const byte> &buf, const bool initVariables);

	/**
	 * A pointer to the raw object data within the object's owner script.
	 */
	SciSpan<const byte> _baseObj;

	/**
	 * A lookup table from a property index to its corresponding selector number.
	 */
	Common::Array<uint16> _baseVars;

	/**
	 * A lookup table from a method index to its corresponding selector number.
	 */
	Common::Array<uint16> _baseMethod;

	/**
	 * A lookup table from a property index to the property's current value.
	 */
	Common::Array<reg_t> _variables;

	/**
	 * A lookup table from a property index to the property's original absolute
	 * offset within the raw script data. This absolute offset is coded into the
	 * script's relocation table, and is used to look up 32-bit values for
	 * properties in the relocation table (which is used to break the 16-bit
	 * barrier, since the script format still only holds 16-bit values inline).
	 */
	Common::Array<uint32> _propertyOffsetsSci3;

	/**
	 * The number of methods on the object.
	 */
	uint16 _methodCount;
	int _flags;

	/**
	 * For SCI0 through SCI2.1, an extra index offset used when looking up
	 * special object properties -species-, -super-, -info-, and name.
	 */
	uint16 _offset;

	reg_t _pos; /**< Object offset within its script; for clones, this is their base */
	reg_t _superClassPosSci3; /**< reg_t pointing to superclass for SCI3 */
	reg_t _speciesSelectorSci3;	/**< reg_t containing species "selector" for SCI3 */
	reg_t _infoSelectorSci3; /**< reg_t containing info "selector" for SCI3 */
	Common::Array<bool> _mustSetViewVisible; /** cached bit of info to make lookup fast, SCI3 only */
};


} // End of namespace Sci

#endif // SCI_ENGINE_OBJECT_H
