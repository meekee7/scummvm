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

#include "mtropolis/plugin/rwc.h"
#include "mtropolis/plugins.h"

namespace MTropolis {

namespace RWC {

ThighBlasterModifier::ThighBlasterModifier() {
}

ThighBlasterModifier::~ThighBlasterModifier() {
}

bool ThighBlasterModifier::load(const PlugInModifierLoaderContext &context, const Data::RWC::ThighBlasterModifier &data) {
	return true;
}

bool ThighBlasterModifier::respondsToEvent(const Event &evt) const {
	return false;
}

VThreadState ThighBlasterModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	return kVThreadReturn;
}

void ThighBlasterModifier::disable(Runtime *runtime) {
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void ThighBlasterModifier::debugInspect(IDebugInspectionReport *report) const {
	Modifier::debugInspect(report);
}
#endif

Common::SharedPtr<Modifier> ThighBlasterModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new ThighBlasterModifier(*this));
}

const char *ThighBlasterModifier::getDefaultName() const {
	return "mLine Launcher Modifier"; // ???
}

RWCPlugIn::RWCPlugIn()
	: _thighBlasterModifierFactory(this) {
}

RWCPlugIn::~RWCPlugIn() {
}

void RWCPlugIn::registerModifiers(IPlugInModifierRegistrar *registrar) const {
	registrar->registerPlugInModifier("ThighBlaster", &_thighBlasterModifierFactory);
}

} // End of namespace RWC

namespace PlugIns {

Common::SharedPtr<PlugIn> createRWC() {
	return Common::SharedPtr<PlugIn>(new RWC::RWCPlugIn());
}

} // End of namespace PlugIns

} // End of namespace MTropolis
