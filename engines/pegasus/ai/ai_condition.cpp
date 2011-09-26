/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/stream.h"

#include "pegasus/energymonitor.h"
#include "pegasus/gamestate.h"
#include "pegasus/ai/ai_condition.h"
#include "pegasus/items/itemlist.h"
#include "pegasus/items/biochips/biochipitem.h"
#include "pegasus/items/inventory/inventoryitem.h"

namespace Pegasus {

AIOneChildCondition::AIOneChildCondition(AICondition *child) {
	_child = child;
}

AIOneChildCondition::~AIOneChildCondition() {
	delete _child;
}

void AIOneChildCondition::writeAICondition(Common::WriteStream *stream) {
	if (_child)
		_child->writeAICondition(stream);
}

void AIOneChildCondition::readAICondition(Common::ReadStream *stream) {
	if (_child)
		_child->readAICondition(stream);
}

AITwoChildrenCondition::AITwoChildrenCondition(AICondition *leftChild, AICondition *rightChild) {
	_leftChild = leftChild;
	_rightChild = rightChild;
}

AITwoChildrenCondition::~AITwoChildrenCondition() {
	delete _leftChild;
	delete _rightChild;
}

void AITwoChildrenCondition::writeAICondition(Common::WriteStream *stream) {
	if (_leftChild)
		_leftChild->writeAICondition(stream);

	if (_rightChild) 
		_rightChild->writeAICondition(stream);
}

void AITwoChildrenCondition::readAICondition(Common::ReadStream *stream) {
	if (_leftChild)
		_leftChild->readAICondition(stream);

	if (_rightChild)
		_rightChild->readAICondition(stream);
}

AINotCondition::AINotCondition(AICondition* child) : AIOneChildCondition(child) {
}
	
bool AINotCondition::fireCondition() {
	return _child && !_child->fireCondition();
}

AIAndCondition::AIAndCondition(AICondition *leftChild, AICondition *rightChild) : AITwoChildrenCondition(leftChild, rightChild) {
}
	
bool AIAndCondition::fireCondition() {
	return _leftChild && _leftChild->fireCondition() && _rightChild && _rightChild->fireCondition();
}

AIOrCondition::AIOrCondition(AICondition *leftChild, AICondition *rightChild) : AITwoChildrenCondition(leftChild, rightChild) {
}

bool AIOrCondition::fireCondition() {
	return (_leftChild && _leftChild->fireCondition()) || (_rightChild && _rightChild->fireCondition());
}

AITimerCondition::AITimerCondition(const TimeValue time, const TimeScale scale, const bool shouldStartTimer) {
	_timerFuse.primeFuse(time, scale);
	_timerFuse.setFunctionPtr((tFunctionPtr)&AITimerFunction, (void *)this);
	_fired = false;

	if (shouldStartTimer)
		startTimer();
}

void AITimerCondition::startTimer() {
	_fired = false;
	_timerFuse.lightFuse();
}

void AITimerCondition::stopTimer() {
	_timerFuse.stopFuse();
}

void AITimerCondition::writeAICondition(Common::WriteStream *stream) {
	stream->writeByte(_timerFuse.isFuseLit());
	stream->writeByte(_fired);
	stream->writeUint32BE(_timerFuse.getTimeRemaining());
	stream->writeUint32BE(_timerFuse.getFuseScale());
}

void AITimerCondition::readAICondition(Common::ReadStream *stream) {
	bool running = stream->readByte();
	_fired = stream->readByte();
	TimeValue time = stream->readUint32BE();
	TimeScale scale = stream->readUint32BE();

	_timerFuse.stopFuse();
	_timerFuse.primeFuse(time, scale);

	if (running)
		_timerFuse.lightFuse();
}

void AITimerCondition::AITimerFunction(FunctionPtr *, AITimerCondition *condition) {
	condition->_fired = true;
}

bool AITimerCondition::fireCondition() {
	return _fired;
}

AILocationCondition::AILocationCondition(uint32 maxLocations) {
	_numLocations = 0;
	_maxLocations = maxLocations;
	_locations = new tRoomViewID[maxLocations];
}

AILocationCondition::~AILocationCondition() {
	delete[] _locations;
}

void AILocationCondition::addLocation(const tRoomViewID location) {
	if (_numLocations < _maxLocations)
		_locations[_numLocations++] = location;
}

bool AILocationCondition::fireCondition() {
	tRoomViewID test = GameState.getCurrentRoomAndView(), *p;
	uint32 i;

	for (i = 0, p = _locations; i < _numLocations; i++, p++) {
		if (test == *p) {
			*p = MakeRoomView(kNoRoomID, kNoDirection);
			return true;
		}
	}

	return false;
}

void AILocationCondition::writeAICondition(Common::WriteStream *stream) {
	stream->writeUint32BE(_maxLocations);
	stream->writeUint32BE(_numLocations);

	uint32 i;
	tRoomViewID *p;
	for (i = 0, p = _locations; i < _numLocations; i++, p++)
		stream->writeUint32BE(*p);
}

void AILocationCondition::readAICondition(Common::ReadStream *stream) {
	uint32 maxLocations = stream->readUint32BE();

	if (_maxLocations != maxLocations) {
		delete[] _locations;
		_locations = new tRoomViewID[maxLocations];
		_maxLocations = maxLocations;
	}

	_numLocations = stream->readUint32BE();

	uint32 i;
	tRoomViewID *p;
	for (i = 0, p = _locations; i < _numLocations; i++, p++)
		*p = stream->readUint32BE();
}

AIDoorOpenedCondition::AIDoorOpenedCondition(tRoomViewID doorLocation) {
	_doorLocation = doorLocation;
}

bool AIDoorOpenedCondition::fireCondition() {
	return GameState.getCurrentRoomAndView() == _doorLocation && GameState.isCurrentDoorOpen();
}

AIHasItemCondition::AIHasItemCondition(const tItemID item) {
	_item = item;
}

bool AIHasItemCondition::fireCondition() {
	return _item == kNoItemID || GameState.isTakenItemID(_item);
}

AIDoesntHaveItemCondition::AIDoesntHaveItemCondition(const tItemID item) {
	_item = item;
}

bool AIDoesntHaveItemCondition::fireCondition() {
	return _item == kNoItemID || !GameState.isTakenItemID(_item);
}

AICurrentItemCondition::AICurrentItemCondition(const tItemID item) {
	_item = item;
}
	
bool AICurrentItemCondition::fireCondition() {
	// TODO
	//InventoryItem *item = ((PegasusEngine *)g_engine)->getCurrentInventoryItem();
	InventoryItem *item = 0;

	if (_item == kNoItemID)
		return item == 0;

	return item != 0 && item->getObjectID() == _item;
}

AICurrentBiochipCondition::AICurrentBiochipCondition(const tItemID biochip)  {
	_biochip = biochip;
}

bool AICurrentBiochipCondition::fireCondition() {
	// TODO
	///BiochipItem *biochip = ((PegasusEngine *)g_engine)->getCurrentBiochip();
	BiochipItem *biochip = 0;
	
	if (_biochip == kNoItemID)
		return biochip == 0;

	return biochip != 0 && biochip->getObjectID() == _biochip;
}

AIItemStateCondition::AIItemStateCondition(const tItemID item, const tItemState state) {
	_item = item;
	_state = state;
}

bool AIItemStateCondition::fireCondition() {
	Item *item = g_allItems.findItemByID(_item);
	return item != 0 && item->getItemState() == _state;
}

AIEnergyMonitorCondition::AIEnergyMonitorCondition(const int32 energyThreshold) {
	_energyThreshold = energyThreshold;
}

bool AIEnergyMonitorCondition::fireCondition() {
	return g_energyMonitor != 0 && g_energyMonitor->getCurrentEnergy() < _energyThreshold;
}

AILastExtraCondition::AILastExtraCondition(const tExtraID lastExtra) {
	_lastExtra = lastExtra;
}

bool AILastExtraCondition::fireCondition() {
	// TODO
	return false;
}

AICondition *makeLocationAndDoesntHaveItemCondition(const tRoomID room, const tDirectionConstant direction, const tItemID item) {
	AILocationCondition *location = new AILocationCondition(1);
	location->addLocation(MakeRoomView(room, direction));
	
	AIDoesntHaveItemCondition *doesntHaveItem = new AIDoesntHaveItemCondition(item);

	return new AIAndCondition(location, doesntHaveItem);
}

} // End of namespace Pegasus
