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

#include "common/system.h"
#include "graphics/paletteman.h"
#include "arcatera/view1.h"
#include "video/smk_decoder.h"
#include "common/file.h"
#include "arcatera/brgparser.h"

namespace Arcatera {

View1::View1() : View("View1") {
	Common::File* file = new Common::File;
	Common::Path path("res\\Locs\\common\\introd.smk", '\\');
	bool open = file->open(path);
	assert(open);
	decoder.loadStream(file);
	decoder.start();
}

bool View1::msgFocus(const FocusMessage &msg) {
	Common::fill(&_pal[0], &_pal[256 * 3], 0);
		
	for (int j = 0; j < 256; j++) {
		_pal[j] = (byte) 256;
		_pal[j + 256] = (byte) 256;
		_pal[j + 256 * 2] = (byte) 256;
	}
	
	return true;
}

bool View1::msgKeypress(const KeypressMessage &msg) {
	// Any keypress to close the view
	close();
	return true;
}

void View1::draw() {
	Graphics::ManagedSurface s = getSurface();

	
	if (true) {
		auto *frame = decoder.decodeNextFrame();
		Graphics::Palette palette;
		if (decoder.getPalette())
			palette = Graphics::Palette(decoder.getPalette(), 256 * 3);
		else
			palette = Graphics::Palette(_pal, 256 * 3);

		if (frame)
			s.blitFrom(*frame, &palette);
	} else {
		Common::File brg;
		brg.open("res/Locs/common/char.brg");

		auto frame = readBrgFile(brg);
		s.blitFrom(frame);
	}
}

bool View1::tick() {
	redraw();

	return true;
}

} // namespace Arcatera
