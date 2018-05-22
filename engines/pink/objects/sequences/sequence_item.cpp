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

#include "common/debug.h"

#include "pink/archive.h"
#include "pink/objects/actions/action.h"
#include "pink/objects/actors/actor.h"
#include "pink/objects/pages/game_page.h"
#include "pink/objects/sequences/sequence_item.h"
#include "pink/objects/sequences/sequence.h"
#include "pink/objects/sequences/sequencer.h"
#include "pink/objects/sequences/sequence_context.h"

namespace Pink {

void SequenceItem::deserialize(Archive &archive) {
    _actor = archive.readString();
    _action = archive.readString();
}

void SequenceItem::toConsole() {
    debug("\t\t\t\tSequenceItem: _actor=%s, _action=%s", _actor.c_str(), _action.c_str());
}

const Common::String &SequenceItem::getActor() const {
    return _actor;
}

const Common::String &SequenceItem::getAction() const {
    return _action;
}

bool SequenceItem::execute(int index, Sequence *sequence, bool unk2) {
    Actor *actor;
    Action *action;
    if (!(actor = sequence->_sequencer->_page->findActor(_actor)) ||
        !(action = actor->findAction(_action))) {
        return false;
    }

    actor->setAction(action, unk2);

    SequenceActorState *state = sequence->_sequencer->findSequenceActorState(_actor);
    state->_index = index;
    sequence->_context->_actor = isLeader() ? actor : sequence->_context->_actor;

    return true;
}

bool SequenceItem::isLeader() {
    return false;
}

bool SequenceItemLeader::isLeader() {
    return true;
}

void SequenceItemLeader::toConsole() {
    debug("\t\t\t\tSequenceItemLeader: _actor=%s, _action=%s", _actor.c_str(), _action.c_str());
}


void SequenceItemLeaderAudio::deserialize(Archive &archive) {
    SequenceItem::deserialize(archive);
    _sample = archive.readDWORD();
}

void SequenceItemLeaderAudio::toConsole() {
    debug("\t\t\t\tSequenceItemLeaderAudio: _actor=%s, _action=%s _sample=%d", _actor.c_str(), _action.c_str(), _sample);
}

uint32 SequenceItemLeaderAudio::getSample() {
    return _sample;
}

bool SequenceItemDefaultAction::execute(int index, Sequence *sequence, bool unk2) {
    SequenceActorState *state = sequence->_sequencer->findSequenceActorState(_actor);
    state->_actionName = _action;
    return true;
}

void SequenceItemDefaultAction::toConsole() {
    debug("\t\t\t\tSequenceItemDefaultAction: _actor=%s, _action=%s", _actor.c_str(), _action.c_str());
}

void SequenceItemDefaultAction::skip(Sequence *sequence) {
    execute(0, sequence, 1);
}

} // End of namespace Pink
