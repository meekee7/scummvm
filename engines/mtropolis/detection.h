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

#ifndef MTROPOLIS_DETECTION_H
#define MTROPOLIS_DETECTION_H

#include "engines/advancedDetector.h"

#define MTROPOLIS_WIN_BOOT_SCRIPT_NAME "mtropolis_boot_win.txt"
#define MTROPOLIS_MAC_BOOT_SCRIPT_NAME "mtropolis_boot_mac.txt"

namespace MTropolis {

enum MTropolisGameID {
	GID_OBSIDIAN			= 0,
	GID_LEARNING_MTROPOLIS	= 1,
	GID_MTI    				= 2,
	GID_ALBERT1				= 3,
	GID_ALBERT2				= 4,
	GID_ALBERT3				= 5,
	GID_SPQR				= 6,
	GID_STTGS				= 7,
	GID_UNIT				= 8,
	GID_MINDGYM				= 9,
	GID_ARCHITECTURE    	= 10,
	GID_BEATRIX		    	= 11,
	GID_WTIMPACT	    	= 12,
	GID_WORLDBROKE	    	= 13,
	GID_DILBERT				= 14,
	GID_FREEWILLY			= 15,
	GID_HERCULES			= 16,
	GID_IDINO				= 17,
	GID_IDOCTOR				= 18,
	GID_MARVEL				= 19,
	GID_FAIRY				= 20,
	GID_PURPLEMOON			= 21,
	GID_CHOMP				= 22,

	GID_GENERIC				= 99,
};

// Boot IDs - These can be shared across different variants if the file list and other properties are identical.
// Cross-reference with the game table in mTropolis engine's boot.cpp
enum MTropolisGameBootID {
	MTBOOT_INVALID = 0,

	MTBOOT_USE_BOOT_SCRIPT,

	MTBOOT_OBSIDIAN_RETAIL_MAC_EN,
	MTBOOT_OBSIDIAN_RETAIL_MAC_JP,
	MTBOOT_OBSIDIAN_RETAIL_WIN_EN,
	MTBOOT_OBSIDIAN_RETAIL_WIN_DE_INSTALLED,
	MTBOOT_OBSIDIAN_RETAIL_WIN_DE_DISC,
	MTBOOT_OBSIDIAN_RETAIL_WIN_IT,
	MTBOOT_OBSIDIAN_DEMO_MAC_EN,
	MTBOOT_OBSIDIAN_DEMO_WIN_EN_1,
	MTBOOT_OBSIDIAN_DEMO_WIN_EN_2,
	MTBOOT_OBSIDIAN_DEMO_WIN_EN_3,
	MTBOOT_OBSIDIAN_DEMO_WIN_EN_4,
	MTBOOT_OBSIDIAN_DEMO_WIN_EN_5,
	MTBOOT_OBSIDIAN_DEMO_WIN_EN_6,
	MTBOOT_OBSIDIAN_DEMO_WIN_EN_7,

	MTBOOT_MTI_RETAIL_MAC,
	MTBOOT_MTI_RETAIL_WIN,
	MTBOOT_MTI_RETAIL_WIN_RU_INSTALLED,
	MTBOOT_MTI_RETAIL_WIN_RU_DISC,
	MTBOOT_MTI_DEMO_WIN,

	MTBOOT_ALBERT1_WIN_DE,
	MTBOOT_ALBERT2_WIN_DE,
	MTBOOT_ALBERT3_WIN_DE,
	MTBOOT_ALBERT1_WIN_EN,
	MTBOOT_ALBERT2_WIN_EN,
	MTBOOT_ALBERT3_WIN_EN,
	MTBOOT_ALBERT1_WIN_FR,
	MTBOOT_ALBERT2_WIN_FR,
	MTBOOT_ALBERT3_WIN_FR,

	MTBOOT_SPQR_RETAIL_WIN,
	MTBOOT_SPQR_RETAIL_MAC,

	MTBOOT_STTGS_RETAIL_WIN,
	MTBOOT_STTGS_DEMO_WIN,

	MTBOOT_UNIT_REBOOTED_WIN,

	MTBOOT_MINDGYM_WIN_EN,
	MTBOOT_MINDGYM_WIN_DE,

	MTBOOT_ARCHITECTURE_EN,
	MTBOOT_BEATRIX_EN,
	MTBOOT_WT_IMPACT_EN,
	MTBOOT_WORLDBROKE_EN,

	MTBOOT_IDINO_RETAIL_EN,
	MTBOOT_IDINO_DEMO_EN,
	MTBOOT_IDOCTOR_RETAIL_EN,
	MTBOOT_IDOCTOR_DEMO_EN,

	MTBOOT_DILBERT_WIN_EN,
	MTBOOT_FREEWILLY_WIN_EN,
	MTBOOT_HERCULES_WIN_EN,
	MTBOOT_MARVEL_WIN_EN,
	MTBOOT_FAIRY_WIN_EN,
	MTBOOT_PURPLEMOON_WIN_EN,
	MTBOOT_CHOMP_WIN_EN,
};

enum MTGameFlag {
	MTGF_WANT_MPEG_VIDEO = (1 << 0),
	MTGF_WANT_MPEG_AUDIO = (1 << 1),
};

struct MTropolisGameDescription {
	ADGameDescription desc;

	int gameID;
	int gameType;
	MTropolisGameBootID bootID;
};

#define GAMEOPTION_WIDESCREEN_MOD				GUIO_GAMEOPTIONS1
#define GAMEOPTION_DYNAMIC_MIDI					GUIO_GAMEOPTIONS2
#define GAMEOPTION_LAUNCH_DEBUG					GUIO_GAMEOPTIONS3
#define GAMEOPTION_SOUND_EFFECT_SUBTITLES		GUIO_GAMEOPTIONS4
#define GAMEOPTION_AUTO_SAVE_AT_CHECKPOINTS		GUIO_GAMEOPTIONS5
#define GAMEOPTION_ENABLE_SHORT_TRANSITIONS		GUIO_GAMEOPTIONS6

} // End of namespace MTropolis

#endif // MTROPOLIS_DETECTION_H
