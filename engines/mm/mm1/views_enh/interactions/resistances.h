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

#ifndef MM1_VIEWS_ENH_INTERACTIONS_RESISTANCES_H
#define MM1_VIEWS_ENH_INTERACTIONS_RESISTANCES_H

#include "mm/mm1/views_enh/interactions/interaction.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {
namespace Interactions {

class Resistances : public Interaction {
protected:
	/**
	 * Handles any action/press
	 */
	void viewAction() override;

	/**
	 * Called when the selected character has been switched
	 */
	void charSwitched(Character *priorChar) override;

public:
	Resistances();

	/**
	 * Handles focus
	 */
	bool msgFocus(const FocusMessage &msg) override;

	/**
	 * Draw the location
	 */
	void draw() override;
};

} // namespace Interactions
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM

#endif
