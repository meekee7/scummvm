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

namespace Arcatera {

const PlainGameDescriptor arcateraGames[] = {
	{ "arcatera", "Arcatera: The Dark Brotherhood" },
	{ "famousfive1", "The Famous 5: Treasure Island" },
	{ "famousfive2", "The Famous 5: Silver Tower" },
	{ "famousfive3", "The Famous 5: Kidnapped" },
	{ "wendy1", "Wendy: Geheimnis um Sören" },
	{ "wendy2", "Wendy: Sarahs großer Traum" },
	{ "radau", "Radau im Bau" },
	{ "schreck1", "Burg Schreckenstein: Der Dieb von Burg Schreckenstein" },
	{ "schreck1", "Burg Schreckenstein 2: Streiche auf Burg Schreckenstein" },
	{ nullptr, nullptr }
};

const ADGameDescription gameDescriptions[] = {
	{
		"famousfive3",
		nullptr,
		AD_ENTRY1s("ff3.exe", "a0015b2e223735cf8ef9211ccb6f677b", 1896496),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	AD_TABLE_END_MARKER
};

} // End of namespace Arcatera
