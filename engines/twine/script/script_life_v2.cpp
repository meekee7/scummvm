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

#include "twine/script/script_life_v2.h"
#include "twine/renderer/redraw.h"
#include "twine/renderer/screens.h"
#include "twine/audio/sound.h"
#include "twine/renderer/renderer.h"
#include "twine/resources/resources.h"
#include "twine/scene/actor.h"
#include "twine/movies.h"
#include "twine/scene/movements.h"
#include "twine/script/script_move_v2.h"
#include "twine/shared.h"
#include "twine/twine.h"

namespace TwinE {

static const ScriptLifeFunction function_map[] = {
	{"END", ScriptLife::lEND},
	{"NOP", ScriptLife::lNOP},
	{"SNIF", ScriptLife::lSNIF},
	{"OFFSET", ScriptLife::lOFFSET},
	{"NEVERIF", ScriptLife::lNEVERIF},
	{"", ScriptLife::lEMPTY}, // unused
	{"NO_IF", ScriptLife::lNO_IF},
	{"", ScriptLife::lEMPTY}, // unused
	{"", ScriptLife::lEMPTY}, // unused
	{"", ScriptLife::lEMPTY}, // unused
	{"PALETTE", ScriptLifeV2::lPALETTE},
	{"RETURN", ScriptLife::lRETURN},
	{"IF", ScriptLife::lIF},
	{"SWIF", ScriptLife::lSWIF},
	{"ONEIF", ScriptLife::lONEIF},
	{"ELSE", ScriptLife::lELSE},
	{"ENDIF", ScriptLife::lEMPTY}, // End of a conditional statement (e.g. IF)
	{"BODY", ScriptLife::lBODY},
	{"BODY_OBJ", ScriptLife::lBODY_OBJ},
	{"ANIM", ScriptLife::lANIM},
	{"ANIM_OBJ", ScriptLife::lANIM_OBJ},
	{"SET_CAMERA", ScriptLifeV2::lSET_CAMERA},
	{"CAMERA_CENTER", ScriptLifeV2::lCAMERA_CENTER},
	{"SET_TRACK", ScriptLife::lSET_TRACK},
	{"SET_TRACK_OBJ", ScriptLife::lSET_TRACK_OBJ},
	{"MESSAGE", ScriptLife::lMESSAGE},
	{"FALLABLE", ScriptLife::lFALLABLE},
	{"SET_DIRMODE", ScriptLife::lSET_DIRMODE},
	{"SET_DIRMODE_OBJ", ScriptLife::lSET_DIRMODE_OBJ},
	{"CAM_FOLLOW", ScriptLife::lCAM_FOLLOW},
	{"SET_BEHAVIOUR", ScriptLife::lSET_BEHAVIOUR},
	{"SET_FLAG_CUBE", ScriptLife::lSET_FLAG_CUBE},
	{"COMPORTEMENT", ScriptLife::lCOMPORTEMENT},
	{"SET_COMPORTEMENT", ScriptLife::lSET_COMPORTEMENT},
	{"SET_COMPORTEMENT_OBJ", ScriptLife::lSET_COMPORTEMENT_OBJ},
	{"END_COMPORTEMENT", ScriptLife::lEND_COMPORTEMENT},
	{"SET_FLAG_GAME", ScriptLife::lSET_FLAG_GAME},
	{"KILL_OBJ", ScriptLife::lKILL_OBJ},
	{"SUICIDE", ScriptLife::lSUICIDE},
	{"USE_ONE_LITTLE_KEY", ScriptLife::lUSE_ONE_LITTLE_KEY},
	{"GIVE_GOLD_PIECES", ScriptLife::lGIVE_GOLD_PIECES},
	{"END_LIFE", ScriptLife::lEND_LIFE},
	{"STOP_L_TRACK", ScriptLife::lSTOP_L_TRACK},
	{"RESTORE_L_TRACK", ScriptLife::lRESTORE_L_TRACK},
	{"MESSAGE_OBJ", ScriptLife::lMESSAGE_OBJ},
	{"INC_CHAPTER", ScriptLife::lINC_CHAPTER},
	{"FOUND_OBJECT", ScriptLife::lFOUND_OBJECT},
	{"SET_DOOR_LEFT", ScriptLife::lSET_DOOR_LEFT},
	{"SET_DOOR_RIGHT", ScriptLife::lSET_DOOR_RIGHT},
	{"SET_DOOR_UP", ScriptLife::lSET_DOOR_UP},
	{"SET_DOOR_DOWN", ScriptLife::lSET_DOOR_DOWN},
	{"GIVE_BONUS", ScriptLife::lGIVE_BONUS},
	{"CHANGE_CUBE", ScriptLife::lCHANGE_CUBE},
	{"OBJ_COL", ScriptLife::lOBJ_COL},
	{"BRICK_COL", ScriptLife::lBRICK_COL},
	{"OR_IF", ScriptLife::lOR_IF},
	{"INVISIBLE", ScriptLife::lINVISIBLE},
	{"SHADOW_OBJ", ScriptLifeV2::lSHADOW_OBJ},
	{"POS_POINT", ScriptLife::lPOS_POINT},
	{"SET_MAGIC_LEVEL", ScriptLife::lSET_MAGIC_LEVEL},
	{"SUB_MAGIC_POINT", ScriptLife::lSUB_MAGIC_POINT},
	{"SET_LIFE_POINT_OBJ", ScriptLife::lSET_LIFE_POINT_OBJ},
	{"SUB_LIFE_POINT_OBJ", ScriptLife::lSUB_LIFE_POINT_OBJ},
	{"HIT_OBJ", ScriptLife::lHIT_OBJ},
	{"PLAY_ACF", ScriptLifeV2::lPLAY_ACF},
	{"ECLAIR", ScriptLifeV2::lECLAIR},
	{"INC_CLOVER_BOX", ScriptLife::lINC_CLOVER_BOX},
	{"SET_USED_INVENTORY", ScriptLife::lSET_USED_INVENTORY},
	{"ADD_CHOICE", ScriptLife::lADD_CHOICE},
	{"ASK_CHOICE", ScriptLife::lASK_CHOICE},
	{"INIT_BUGGY", ScriptLifeV2::lINIT_BUGGY},
	{"MEMO_ARDOISE", ScriptLifeV2::lMEMO_ARDOISE},
	{"SET_HOLO_POS", ScriptLife::lSET_HOLO_POS},
	{"CLR_HOLO_POS", ScriptLife::lCLR_HOLO_POS},
	{"ADD_FUEL", ScriptLife::lADD_FUEL},
	{"SUB_FUEL", ScriptLife::lSUB_FUEL},
	{"SET_GRM", ScriptLife::lSET_GRM},
	{"SET_CHANGE_CUBE", ScriptLifeV2::lSET_CHANGE_CUBE},
	{"MESSAGE_ZOE", ScriptLifeV2::lMESSAGE_ZOE}, // lSAY_MESSAGE
	{"FULL_POINT", ScriptLife::lFULL_POINT},
	{"BETA", ScriptLife::lBETA},
	{"FADE_TO_PAL", ScriptLifeV2::lFADE_TO_PAL},
	{"ACTION", ScriptLifeV2::lACTION},
	{"SET_FRAME", ScriptLifeV2::lSET_FRAME},
	{"SET_SPRITE", ScriptLifeV2::lSET_SPRITE},
	{"SET_FRAME_3DS", ScriptLifeV2::lSET_FRAME_3DS},
	{"IMPACT_OBJ", ScriptLifeV2::lIMPACT_OBJ},
	{"IMPACT_POINT", ScriptLifeV2::lIMPACT_POINT},
	{"ADD_MESSAGE", ScriptLifeV2::lADD_MESSAGE},
	{"BUBBLE", ScriptLifeV2::lBUBBLE},
	{"NO_CHOC", ScriptLifeV2::lNO_CHOC},
	{"ASK_CHOICE_OBJ", ScriptLife::lASK_CHOICE_OBJ},
	{"CINEMA_MODE", ScriptLifeV2::lCINEMA_MODE},
	{"SAVE_HERO", ScriptLifeV2::lSAVE_HERO},
	{"RESTORE_HERO", ScriptLifeV2::lRESTORE_HERO},
	{"ANIM_SET", ScriptLife::lANIM_SET},
	{"RAIN", ScriptLifeV2::lRAIN}, // LM_PLUIE
	{"GAME_OVER", ScriptLife::lGAME_OVER},
	{"THE_END", ScriptLife::lTHE_END},
	{"ESCALATOR", ScriptLifeV2::lESCALATOR},
	{"PLAY_MUSIC", ScriptLifeV2::lPLAY_MUSIC},
	{"TRACK_TO_VAR_GAME", ScriptLifeV2::lTRACK_TO_VAR_GAME},
	{"VAR_GAME_TO_TRACK", ScriptLifeV2::lVAR_GAME_TO_TRACK},
	{"ANIM_TEXTURE", ScriptLifeV2::lANIM_TEXTURE},
	{"ADD_MESSAGE_OBJ", ScriptLifeV2::lADD_MESSAGE_OBJ},
	{"BRUTAL_EXIT", ScriptLife::lBRUTAL_EXIT},
	{"REM", ScriptLife::lEMPTY}, // unused
	{"LADDER", ScriptLifeV2::lLADDER},
	{"SET_ARMOR", ScriptLifeV2::lSET_ARMOR},
	{"SET_ARMOR_OBJ", ScriptLifeV2::lSET_ARMOR_OBJ},
	{"ADD_LIFE_POINT_OBJ", ScriptLifeV2::lADD_LIFE_POINT_OBJ},
	{"STATE_INVENTORY", ScriptLifeV2::lSTATE_INVENTORY},
	{"AND_IF", ScriptLifeV2::lAND_IF},
	{"SWITCH", ScriptLifeV2::lSWITCH},
	{"OR_CASE", ScriptLifeV2::lOR_CASE},
	{"CASE", ScriptLifeV2::lCASE},
	{"DEFAULT", ScriptLife::lEMPTY}, // unused
	{"BREAK", ScriptLifeV2::lBREAK},
	{"END_SWITCH", ScriptLife::lEMPTY}, // unused
	{"SET_HIT_ZONE", ScriptLifeV2::lSET_HIT_ZONE},
	{"SAVE_COMPORTEMENT", ScriptLifeV2::lSAVE_COMPORTEMENT},
	{"RESTORE_COMPORTEMENT", ScriptLifeV2::lRESTORE_COMPORTEMENT},
	{"SAMPLE", ScriptLifeV2::lSAMPLE},
	{"SAMPLE_RND", ScriptLifeV2::lSAMPLE_RND},
	{"SAMPLE_ALWAYS", ScriptLifeV2::lSAMPLE_ALWAYS},
	{"SAMPLE_STOP", ScriptLifeV2::lSAMPLE_STOP},
	{"REPEAT_SAMPLE", ScriptLifeV2::lREPEAT_SAMPLE},
	{"BACKGROUND", ScriptLifeV2::lBACKGROUND},
	{"ADD_VAR_GAME", ScriptLifeV2::lADD_VAR_GAME},
	{"SUB_VAR_GAME", ScriptLifeV2::lSUB_VAR_GAME},
	{"ADD_VAR_CUBE", ScriptLifeV2::lADD_VAR_CUBE},
	{"SUB_VAR_CUBE", ScriptLifeV2::lSUB_VAR_CUBE},
	{"NOP", ScriptLife::lEMPTY}, // unused
	{"SET_RAIL", ScriptLifeV2::lSET_RAIL},
	{"INVERSE_BETA", ScriptLifeV2::lINVERSE_BETA},
	{"NO_BODY", ScriptLifeV2::lNO_BODY},
	{"GIVE_GOLD_PIECES", ScriptLife::lGIVE_GOLD_PIECES},
	{"STOP_L_TRACK_OBJ", ScriptLifeV2::lSTOP_L_TRACK_OBJ},
	{"RESTORE_L_TRACK_OBJ", ScriptLifeV2::lRESTORE_L_TRACK_OBJ},
	{"SAVE_COMPORTEMENT_OBJ", ScriptLifeV2::lSAVE_COMPORTEMENT_OBJ},
	{"RESTORE_COMPORTEMENT_OBJ", ScriptLifeV2::lRESTORE_COMPORTEMENT_OBJ},
	{"SPY", ScriptLife::lEMPTY},       // unused
	{"DEBUG", ScriptLife::lEMPTY},     // unused
	{"DEBUG_OBJ", ScriptLife::lEMPTY}, // unused
	{"POPCORN", ScriptLife::lEMPTY},   // unused
	{"FLOW_POINT", ScriptLifeV2::lFLOW_POINT},
	{"FLOW_OBJ", ScriptLifeV2::lFLOW_OBJ},
	{"SET_ANIM_DIAL", ScriptLifeV2::lSET_ANIM_DIAL},
	{"PCX", ScriptLifeV2::lPCX},
	{"END_MESSAGE", ScriptLifeV2::lEND_MESSAGE},
	{"END_MESSAGE_OBJ", ScriptLifeV2::lEND_MESSAGE_OBJ},
	{"PARM_SAMPLE", ScriptLifeV2::lPARM_SAMPLE},
	{"NEW_SAMPLE", ScriptLifeV2::lNEW_SAMPLE},
	{"POS_OBJ_AROUND", ScriptLifeV2::lPOS_OBJ_AROUND},
	{"PCX_MESS_OBJ", ScriptLifeV2::lPCX_MESS_OBJ}
};

int32 ScriptLifeV2::lPALETTE(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 palIndex = engine->_screens->mapLba2Palette(ctx.stream.readByte());
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::PALETTE(%i)", palIndex);
	ScopedEngineFreeze scoped(engine);
	HQR::getEntry(engine->_screens->_palette, Resources::HQR_RESS_FILE, palIndex);
	engine->_screens->convertPalToRGBA(engine->_screens->_palette, engine->_screens->_paletteRGBA);
	engine->setPalette(engine->_screens->_paletteRGBA);
	engine->_screens->_useAlternatePalette = true;
	return 0;
}

int32 ScriptLifeV2::lFADE_TO_PAL(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 palIndex = engine->_screens->mapLba2Palette(ctx.stream.readByte());
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::FADE_TO_PAL(%i)", palIndex);
	// TODO: implement
	return -1;
}

int32 ScriptLifeV2::lPLAY_MUSIC(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::lPLAY_MUSIC()");
	// TODO: game var 157 is checked here
	return lPLAY_CD_TRACK(engine, ctx);
}

int32 ScriptLifeV2::lTRACK_TO_VAR_GAME(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 num = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::lTRACK_TO_VAR_GAME(%i)", (int)num);
	engine->_gameState->setGameFlag(num, MAX<int32>(0, ctx.actor->_labelIdx));
	return 0;
}

int32 ScriptLifeV2::lVAR_GAME_TO_TRACK(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lANIM_TEXTURE(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lADD_MESSAGE_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lADD_MESSAGE(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lCAMERA_CENTER(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lBUBBLE(TwinEEngine *engine, LifeScriptContext &ctx) {
	engine->_text->_showDialogueBubble = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::BUBBLE(%s)", engine->_text->_showDialogueBubble ? "true" : "false");
	return 0;
}

int32 ScriptLifeV2::lNO_CHOC(TwinEEngine *engine, LifeScriptContext &ctx) {
	const uint8 val = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::lNO_CHOC(%i)", (int)val);
	ctx.actor->_staticFlags.bNoElectricShock = val;
	return 0;
}

int32 ScriptLifeV2::lCINEMA_MODE(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lSAVE_HERO(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::SAVE_HERO()");
	engine->_actor->_saveHeroBehaviour = engine->_actor->_heroBehaviour;
	ActorStruct *actor = engine->_scene->getActor(OWN_ACTOR_SCENE_INDEX);
	actor->_saveGenBody = actor->_genBody;
	return 0;
}

int32 ScriptLifeV2::lRESTORE_HERO(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::RESTORE_HERO()");
	engine->_actor->setBehaviour(engine->_actor->_saveHeroBehaviour);
	ActorStruct *actor = engine->_scene->getActor(OWN_ACTOR_SCENE_INDEX);
	engine->_actor->initBody(actor->_saveGenBody, OWN_ACTOR_SCENE_INDEX);
	return 0;
}

int32 ScriptLifeV2::lRAIN(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 num = (int)ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::lRAIN(%i)", (int)num);
	int32 n = engine->_redraw->addOverlay(OverlayType::koRain, 0, 0, 0, 0, OverlayPosType::koNormal, 1);
	if (n != -1) {
		// Rain n/10s
		engine->_redraw->overlayList[n].lifeTime = engine->timerRef + engine->toSeconds(num / 10);
		engine->_flagRain = true;
		engine->_sound->startRainSample();
	}
	return 0;
}

int32 ScriptLifeV2::lESCALATOR(TwinEEngine *engine, LifeScriptContext &ctx) {
	const uint8 num = ctx.stream.readByte();
	const uint8 info1 = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::lESCALATOR(%i, %i)", (int)num, (int)info1);
	for (int n = 0; n < engine->_scene->_sceneNumZones; n++) {
		ZoneStruct &zone = engine->_scene->_sceneZones[n];
		if (zone.type == ZoneType::kEscalator && zone.num == num) {
			zone.infoData.generic.info1 = info1;
		}
	}
	return 0;
}

int32 ScriptLifeV2::lSET_CAMERA(TwinEEngine *engine, LifeScriptContext &ctx) {
	const uint8 num = ctx.stream.readByte();
	const uint8 info7 = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::lSET_CAMERA(%i, %i)", (int)num, (int)info7);
	for (int n = 0; n < engine->_scene->_sceneNumZones; n++) {
		ZoneStruct &zone = engine->_scene->_sceneZones[n];
		if (zone.type == ZoneType::kCamera && zone.num == num) {
			zone.infoData.generic.info7 = info7;
			if (info7)
				zone.infoData.generic.info7 |= ZONE_ON;
			else
				zone.infoData.generic.info7 &= ~(ZONE_ON);

			zone.infoData.generic.info7 &= ~(ZONE_ACTIVE);
		}
	}
	return 0;
}

int32 ScriptLifeV2::lPLAY_ACF(TwinEEngine *engine, LifeScriptContext &ctx) {
	ScopedEngineFreeze timer(engine);
	int strIdx = 0;
	char movie[64];
	do {
		const byte c = ctx.stream.readByte();
		movie[strIdx++] = c;
		if (c == '\0') {
			break;
		}
		if (strIdx >= ARRAYSIZE(movie)) {
			error("Max string size exceeded for acf name");
		}
	} while (true);
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::lPLAY_ACF(%s)", movie);

	engine->_movie->playMovie(movie);
	// TODO: lba2 is doing more stuff here - reset the cinema mode, init the scene and palette stuff
	engine->setPalette(engine->_screens->_paletteRGBA);
	engine->_redraw->_firstTime = true;

	return -1;
}

int32 ScriptLifeV2::lSHADOW_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int actorIdx = ctx.stream.readByte();
	const bool castShadow = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::SHADOW_OBJ(%i, %s)", actorIdx, castShadow ? "true" : "false");
	ActorStruct *actor = engine->_scene->getActor(actorIdx);
	if (actor->_lifePoint > 0) {
		actor->_staticFlags.bDoesntCastShadow = !castShadow;
	}
	return 0;
}

int32 ScriptLifeV2::lECLAIR(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 num = (int)ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::lECLAIR(%i)", (int)num);
	int32 n = engine->_redraw->addOverlay(OverlayType::koFlash, 0, 0, 0, 0, OverlayPosType::koNormal, 1);
	if (n != -1) {
		// Eclair n/10s
		engine->_redraw->overlayList[n].lifeTime = engine->timerRef + engine->toSeconds(num / 10);
	}
	return 0;
}

int32 ScriptLifeV2::lINIT_BUGGY(TwinEEngine *engine, LifeScriptContext &ctx) {
	const uint8 num = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::lINIT_BUGGY(%i)", (int)num);
	engine->_buggy->initBuggy(ctx.actorIdx, num);
	return 0;
}

int32 ScriptLifeV2::lMEMO_ARDOISE(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lSET_CHANGE_CUBE(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lMESSAGE_ZOE(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lACTION(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::ACTION()");
	engine->_movements->setActionNormal(true);
	return 0;
}

int32 ScriptLifeV2::lSET_FRAME(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int frame = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::lSET_FRAME(%i)", (int)frame);
	if (!ctx.actor->_staticFlags.bIsSpriteActor) {
		// TODO: ObjectSetFrame(ctx.actorIdx, frame);
	}
	return -1;
}

int32 ScriptLifeV2::lSET_SPRITE(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int16 num = ctx.stream.readSint16LE();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::lSET_SPRITE(%i)", (int)num);
	if (ctx.actor->_staticFlags.bIsSpriteActor) {
		ctx.actor->_sprite = num;
		engine->_actor->initSpriteActor(ctx.actorIdx);
	}
	return 0;
}

int32 ScriptLifeV2::lSET_FRAME_3DS(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int sprite = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::lSET_FRAME_3DS(%i)", (int)sprite);
	if (ctx.actor->_staticFlags.bHasSpriteAnim3D) {
		// TODO:
		// if (sprite > (ListAnim3DS[ptrobj->Coord.A3DS.Num].Fin - ListAnim3DS[ptrobj->Coord.A3DS.Num].Deb)) {
		// 	sprite = ListAnim3DS[ptrobj->Coord.A3DS.Num].Fin - ListAnim3DS[ptrobj->Coord.A3DS.Num].Deb;
		// }
		// sprite += ListAnim3DS[ptrobj->Coord.A3DS.Num].Deb;

		ctx.actor->_sprite = sprite;
		engine->_actor->initSpriteActor(ctx.actorIdx);
	}
	return -1;
}

int32 ScriptLifeV2::lIMPACT_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	const uint8 num = ctx.stream.readByte();
	const uint16 n = ctx.stream.readUint16LE();
	const int16 y = ctx.stream.readSint16LE();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::lIMPACT_OBJ(%i, %i, %i)", (int)num, (int)n, (int)y);
	ActorStruct *otherActor = engine->_scene->getActor(num);
	if (otherActor->_lifePoint > 0) {
		// TODO: DoImpact(n, otherActor->_pos.x, otherActor->_pos.y + y, otherActor->_pos.z, num);
	}
	return -1;
}

int32 ScriptLifeV2::lIMPACT_POINT(TwinEEngine *engine, LifeScriptContext &ctx) {
	const uint8 brickTrackId = ctx.stream.readByte();
	const uint16 n = ctx.stream.readUint16LE();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::lIMPACT_POINT(%i, %i)", (int)brickTrackId, (int)n);
	// int16 x0 = ListBrickTrack[brickTrackId].x;
	// int16 y0 = ListBrickTrack[brickTrackId].y;
	// int16 z0 = ListBrickTrack[brickTrackId].z;
	// TODO: DoImpact(n, x0, y0, z0, ctx.actorIdx);
	return -1;
}

int32 ScriptLifeV2::lLADDER(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lSET_ARMOR(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lSET_ARMOR_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lADD_LIFE_POINT_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lSTATE_INVENTORY(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lAND_IF(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lSWITCH(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lOR_CASE(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lCASE(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lDEFAULT(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lBREAK(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lEND_SWITCH(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lSET_HIT_ZONE(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lSAVE_COMPORTEMENT(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::lSAVE_COMPORTEMENT()");
	ctx.actor->_saveOffsetLife = ctx.actor->_offsetLife;
	return 0;
}

int32 ScriptLifeV2::lRESTORE_COMPORTEMENT(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::lRESTORE_COMPORTEMENT()");
	ctx.actor->_offsetLife = ctx.actor->_saveOffsetLife;
	return 0;
}

int32 ScriptLifeV2::lSAMPLE(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lSAMPLE_RND(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lSAMPLE_ALWAYS(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lSAMPLE_STOP(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lREPEAT_SAMPLE(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lBACKGROUND(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lADD_VAR_GAME(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lSUB_VAR_GAME(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lADD_VAR_CUBE(TwinEEngine *engine, LifeScriptContext &ctx) {
	const uint8 num = ctx.stream.readByte();
	const uint8 amount = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::lADD_VAR_CUBE(%i, %i)", (int)num, (int)amount);
	uint8 &current = engine->_scene->_listFlagCube[num];
	if ((int16)current + (int16)amount < 255) {
		current += amount;
	} else {
		current = 255;
	}
	return 0;
}

int32 ScriptLifeV2::lSUB_VAR_CUBE(TwinEEngine *engine, LifeScriptContext &ctx) {
	const uint8 num = ctx.stream.readByte();
	const uint8 amount = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::lSUB_VAR_CUBE(%i, %i)", (int)num, (int)amount);
	uint8 &current = engine->_scene->_listFlagCube[num];
	if ((int16)current - (int16)amount > 0) {
		current -= amount;
	} else {
		current = 0;
	}
	return 0;
}

int32 ScriptLifeV2::lSET_RAIL(TwinEEngine *engine, LifeScriptContext &ctx) {
	const uint8 num = ctx.stream.readByte();
	const uint8 info1 = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::lSET_RAIL(%i, %i)", (int)num, (int)info1);
	for (int n = 0; n < engine->_scene->_sceneNumZones; n++) {
		ZoneStruct &zone = engine->_scene->_sceneZones[n];
		if (zone.type == ZoneType::kRail && zone.num == num) {
			zone.infoData.generic.info1 = info1;
		}
	}
	return 0;
}

int32 ScriptLifeV2::lINVERSE_BETA(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lNO_BODY(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::lNO_BODY()");
	engine->_actor->initBody(BodyType::btNone, ctx.actorIdx);
	return 0;
}

int32 ScriptLifeV2::lSTOP_L_TRACK_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lRESTORE_L_TRACK_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lSAVE_COMPORTEMENT_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lRESTORE_COMPORTEMENT_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lSPY(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lDEBUG(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lDEBUG_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lPOPCORN(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::lPOPCORN()");
	// empty on purpose
	return 0;
}

int32 ScriptLifeV2::lFLOW_POINT(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lFLOW_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lSET_ANIM_DIAL(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lPCX(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lEND_MESSAGE(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::lEND_MESSAGE()");
	// empty on purpose
	return 0;
}

int32 ScriptLifeV2::lEND_MESSAGE_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	/*num = */ ctx.stream.readByte();
	return 0;
}

int32 ScriptLifeV2::lPARM_SAMPLE(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lNEW_SAMPLE(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lPOS_OBJ_AROUND(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lPCX_MESS_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

ScriptLifeV2::ScriptLifeV2(TwinEEngine *engine) : ScriptLife(engine, function_map, ARRAYSIZE(function_map)) {
}

} // namespace TwinE
