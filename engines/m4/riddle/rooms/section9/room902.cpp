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
 * along with this program.  If not, see <http://www.gnu.org/licenses/ },.
 *
 */

#include "m4/riddle/rooms/section9/room902.h"
#include "m4/riddle/vars.h"
#include "m4/graphics/gr_series.h"
#include "m4/gui/gui_sys.h"
#include "m4/platform/keys.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room902::preload() {
	_G(player).walker_type = 1;
	_G(player).shadow_type = 1;
	_G(player).walker_in_this_scene = false;
}

void Room902::init() {
	player_set_commands_allowed(false);
	digi_preload("gunshot1", 902);
	digi_preload("gunshot2", 902);
	digi_preload("introcu-", 901);
	digi_play("introcu-", 1, 255, -1, 901);
	series_load("902 one frame hold");
	kernel_timing_trigger(1, 1);
	interface_hide();

	AddSystemHotkey(KEY_ESCAPE, escapePressed);
	_G(kernel).call_daemon_every_loop = true;
}

void Room902::daemon() {
	// TODO
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
