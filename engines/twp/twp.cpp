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

#include "common/scummsys.h"
#include "common/system.h"
#include "common/stream.h"
#include "common/file.h"
#include "common/config-manager.h"
#include "common/events.h"
#include "engines/util.h"
#include "graphics/palette.h"
#include "graphics/opengl/system_headers.h"
#include "twp/twp.h"
#include "twp/detection.h"
#include "twp/console.h"
#include "twp/vm.h"
#include "twp/ggpack.h"
#include "twp/gfx.h"
#include "twp/lighting.h"
#include "twp/font.h"
#include "twp/thread.h"
#include "twp/scenegraph.h"
#include "twp/squtil.h"
#include "twp/object.h"
#include "twp/ids.h"

namespace Twp {

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

TwpEngine *g_engine;

TwpEngine::TwpEngine(OSystem *syst, const ADGameDescription *gameDesc)
	: Engine(syst),
	  _gameDescription(gameDesc),
	  _randomSource("Twp") {
	g_engine = this;
}

TwpEngine::~TwpEngine() {
	delete _screen;
}

uint32 TwpEngine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String TwpEngine::getGameId() const {
	return _gameDescription->gameId;
}

void TwpEngine::update(float elapsedMs) {
	// update threads
	for (int i = 0; i < _threads.size(); i++) {
		Thread *thread = _threads[i];
		if (thread->update(elapsedMs)) {
			// TODO: delete it
		}
	}
}

void TwpEngine::draw() {
	if (_room) {
		Math::Vector2d screenSize = _room->getScreenSize();
		_gfx.camera(screenSize);
	}
	_gfx.clear(Color(0, 0, 0));
	_gfx.use(NULL);
	_scene->draw();

	g_system->updateScreen();
}

Common::Error TwpEngine::run() {
	initGraphics3d(SCREEN_WIDTH, SCREEN_HEIGHT);
	_screen = new Graphics::Screen(SCREEN_WIDTH, SCREEN_HEIGHT);

	// Set the engine's debugger console
	setDebugger(new Console());

	_gfx.init();
	_lighting = new Lighting();
	_scene = new Scene();

	XorKey key{{0x4F, 0xD0, 0xA0, 0xAC, 0x4A, 0x56, 0xB9, 0xE5, 0x93, 0x79, 0x45, 0xA5, 0xC1, 0xCB, 0x31, 0x93}, 0xAD};
	// XorKey key{{0x4F, 0xD0, 0xA0, 0xAC, 0x4A, 0x56, 0xB9, 0xE5, 0x93, 0x79, 0x45, 0xA5, 0xC1, 0xCB, 0x31, 0x93}, 0x6D};
	// XorKey key{{0x4F, 0xD0, 0xA0, 0xAC, 0x4A, 0x5B, 0xB9, 0xE5, 0x93, 0x79, 0x45, 0xA5, 0xC1, 0xCB, 0x31, 0x93}, 0x6D};
	// XorKey key{{0x4F, 0xD0, 0xA0, 0xAC, 0x4A, 0x5B, 0xB9, 0xE5, 0x93, 0x79, 0x45, 0xA5, 0xC1, 0xCB, 0x31, 0x93}, 0xAD};

	Common::File f;
	f.open("ThimbleweedPark.ggpack1");
	_pack.open(&f, key);

	// If a savegame was selected from the launcher, load it
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1)
		(void)loadGameState(saveSlot);

	const SQChar *code = R"(
	MainStreet <- {
		background = "MainStreet"
		enter = function(enter_door) {
		}
	}
	defineRoom(MainStreet);
	cameraInRoom(MainStreet);
	)";

	_vm.exec(code);

	// Simple event handling loop
	Common::Event e;
	uint deltaTimeMs = 0;
	while (!shouldQuit()) {
		const int dx = 4;
		const int dy = 4;
		Math::Vector2d camPos = _gfx.cameraPos();
		while (g_system->getEventManager()->pollEvent(e)) {
			switch (e.type) {
			case Common::EVENT_KEYDOWN:
				switch (e.kbd.keycode) {
				case Common::KEYCODE_LEFT:
					camPos.setX(camPos.getX() - dx);
					break;
				case Common::KEYCODE_RIGHT:
					camPos.setX(camPos.getX() + dx);
					break;
				case Common::KEYCODE_UP:
					camPos.setY(camPos.getY() + dy);
					break;
				case Common::KEYCODE_DOWN:
					camPos.setY(camPos.getY() - dy);
					break;
				default:
					break;
				}
			default:
				break;
			}
		}

		_gfx.cameraPos(camPos);

		// update threads
		update(deltaTimeMs);
		draw();

		// Delay for a bit. All events loops should have a delay
		// to prevent the system being unduly loaded
		deltaTimeMs = 10;
		g_system->delayMillis(deltaTimeMs);
	}

	return Common::kNoError;
}

Common::Error TwpEngine::syncGame(Common::Serializer &s) {
	// The Serializer has methods isLoading() and isSaving()
	// if you need to specific steps; for example setting
	// an array size after reading it's length, whereas
	// for saving it would write the existing array's length
	int dummy = 0;
	s.syncAsUint32LE(dummy);

	return Common::kNoError;
}

Math::Vector2d TwpEngine::roomToScreen(Math::Vector2d pos) {
	Math::Vector2d screenSize = _room->getScreenSize();
	return Math::Vector2d(SCREEN_WIDTH, SCREEN_HEIGHT) * (pos - _gfx.cameraPos()) / screenSize;
}

Math::Vector2d TwpEngine::screenToRoom(Math::Vector2d pos) {
	Math::Vector2d screenSize = _room->getScreenSize();
	return (pos * screenSize) / Math::Vector2d(SCREEN_WIDTH, SCREEN_HEIGHT) + _gfx.cameraPos();
}

Room *TwpEngine::defineRoom(const Common::String &name, HSQOBJECT table, bool pseudo) {
	HSQUIRRELVM v = _vm.get();
	debug("Load room: %s", name.c_str());
	Room *result;
	if (name == "Void") {
		error("TODO: room Void");
		// result = new Room(name, table);
		// result.scene = new Scene();
		// Layer* layer = new Layer("background", Math::Vector2d(1.f, 1.f), 0);
		// layer->_node = new ParallaxNode(Math::Vector2d(1.f, 1.f), "background");
		// result->_layers.push_back(layer);
		// // TODO:; result->scene.addChild(layer->node);
		// sqsetf(sqrootTbl(v), name, result->table);
	} else {
		result = new Room(name, table);
		Common::String background;
		sqgetf(table, "background", background);
		GGPackEntryReader entry;
		entry.open(_pack, background + ".wimpy");
		result->load(entry);
		result->_pseudo = pseudo;
		for (int i = 0; i < result->_layers.size(); i++) {
			Layer *layer = result->_layers[i];
			// create layer node
			ParallaxNode *layerNode = new ParallaxNode(layer->_parallax, result->_sheet, layer->_names);
			layerNode->setZSort(layer->_zsort);
			layerNode->setName(Common::String::format("Layer %s(%d)", layer->_names[0].c_str(), layer->_zsort));
			layer->_node = layerNode;
			result->_scene->addChild(layerNode);

			for (int j = 0; j < layer->_objects.size(); j++) {
				Object *obj = layer->_objects[j];
				if (!sqrawexists(table, obj->_key)) {
					// this object does not exist, so create it
					sq_newtable(v);
					sq_getstackobj(v, -1, &obj->_table);
					sq_addref(v, &obj->_table);
					sq_pop(v, 1);

					// assign an id
					setId(obj->_table, newObjId());
					// info fmt"Create object with new table: {obj.name} #{obj.id}"

					// adds the object to the room table
					sqsetf(result->_table, obj->_key, obj->_table);
					obj->setRoom(result);
					obj->setState(0, true);

					if (obj->_objType == otNone)
						obj->_touchable = false;
				} else if (obj->_objType == otNone) {
					obj->_touchable = true;
				}

				layerNode->addChild(obj->_node);
			}
		}
	}

	// assign parent node
	for (int i = 0; i < result->_layers.size(); i++) {
		Layer *layer = result->_layers[i];
		for (int j = 0; j < layer->_objects.size(); j++) {
			Object *obj = layer->_objects[j];
			if (obj->_parent.size() > 0) {
				Object *parent = result->getObj(obj->_parent);
				if (!parent) {
					warning("parent: '%s' not found", obj->_parent.c_str());
				} else {
					parent->_node->addChild(obj->_node);
				}
			}
		}
	}

	sqgetpairs(result->_table, [&](const Common::String &k, HSQOBJECT &oTable) {
		if (oTable._type == OT_TABLE) {
			if (pseudo) {
				// if it's a pseudo room we need to clone each object
				sq_pushobject(v, oTable);
				sq_clone(v, -1);
				sq_getstackobj(v, -1, &oTable);
				sq_addref(v, &oTable);
				sq_pop(v, 2);
				sqsetf(result->_table, k, oTable);
			}

			if (sqrawexists(oTable, "icon")) {
				// Add inventory object to root table
				debug("Add %s to inventory", k.c_str());
				sqsetf(sqrootTbl(v), k, oTable);

				// set room as delegate
				sqsetdelegate(oTable, table);

				// declare flags if does not exist
				if (!sqrawexists(oTable, "flags"))
					sqsetf(oTable, "flags", 0);
				Object *obj = new Object(oTable, k);
				setId(obj->_table, newObjId());
				obj->_node = new Node(k);
				obj->_nodeAnim = new Anim(obj);
				obj->_node->addChild(obj->_nodeAnim);
				obj->setRoom(result);
				// set room as delegate
				sqsetdelegate(obj->_table, table);
			} else {
				Object *obj = result->getObj(k);
				if (!obj) {
					debug("object: %s not found in wimpy", k.c_str());
					if (sqrawexists(oTable, "name")) {
						obj = new Object();
						obj->_key = k;
						obj->_layer = result->layer(0);
						result->layer(0)->_objects.push_back(obj);
					} else {
						return;
					}
				}

				sqgetf(result->_table, k, obj->_table);
				setId(obj->_table, newObjId());
				debug("Create object: %s #%d", k.c_str(), obj->getId());

				// add it to the root table if not a pseudo room
				if (!pseudo)
					sqsetf(sqrootTbl(v), k, obj->_table);

				if (sqrawexists(obj->_table, "initState")) {
					// info fmt"initState {obj.key}"
					int state;
					sqgetf(obj->_table, "initState", state);
					obj->setState(state, true);
				} else {
					obj->setState(0, true);
				}
				obj->setRoom(result);

				// set room as delegate
				sqsetdelegate(obj->_table, table);

				// declare flags if does not exist
				if (!sqrawexists(obj->_table, "flags"))
					sqsetf(obj->_table, "flags", 0);
			}
		}
	});

	// declare the room in the root table
	setId(result->_table, newRoomId());
	sqsetf(sqrootTbl(v), name, result->_table);

	return result;
}

void TwpEngine::enterRoom(Room *room, Object *door) {
	HSQUIRRELVM v = getVm();
	// Called when the room is entered.
	debug("call enter room function of %s", room->_name.c_str());

	// exit current room
	// exitRoom(_room);
	//_fadeEffect.effect = None;

	// sets the current room for scripts
	sqsetf(sqrootTbl(v), "currentRoom", room->_table);

	if (_room)
		_room->_scene->remove();
	_room = room;
	_scene->addChild(_room->_scene);
	//   _room->numLights = 0;
	//   _room->overlay = Transparent;
	//   _camera.bounds = rectFromMinMax(vec2(0f,0f), vec2f(room.roomSize));
	//   if (_actor)
	//     _hud.verb = _hud.actorSlot(_actor).verbs[0];

	//   // move current actor to the new room
	//   var camPos: Vec2f
	//   if not gEngine.actor.isNil:
	//     self.cancelSentence(nil)
	//     if not door.isNil:
	//       let facing = getOppositeFacing(door.getDoorFacing())
	//       gEngine.actor.room = room
	//       if not door.isNil:
	//         gEngine.actor.setFacing(facing)
	//         gEngine.actor.node.pos = door.getUsePos
	//       camPos = gEngine.actor.node.pos

	//   self.camera.room = room
	//   self.camera.at = camPos

	//   // call actor enter function and objects enter function
	//   self.actorEnter()
	//   for layer in room.layers:
	//     for obj in layer.objects:
	//       if rawExists(obj.table, "enter"):
	//         call(self.v, obj.table, "enter")

	//   // call room enter function with the door as a parameter if requested
	//   let nparams = paramCount(self.v, self.room.table, "enter")
	//   if nparams == 2:
	//     if door.isNil:
	//       var doorTable: HSQOBJECT
	//       sq_resetobject(doorTable)
	//       call(self.v, self.room.table, "enter", [doorTable])
	//     else:
	//       call(self.v, self.room.table, "enter", [door.table])
	//   else:
	//     call(self.v, self.room.table, "enter")

	//   # call global function enteredRoom with the room as argument
	//   call("enteredRoom", [room.table])
}

void TwpEngine::exitRoom(Room *nextRoom) {
	HSQUIRRELVM v = getVm();
	// TODO: _audio.stopAll()
	if (_room) {
		_room->_triggers.clear();

		actorExit();

		// call room exit function with the next room as a parameter if requested
		int nparams = sqparamCount(v, _room->_table, "exit");
		HSQOBJECT args[] = {nextRoom->_table};
		if (nparams == 2) {
			sqcall(_room->_table, "exit", 1, args);
		} else {
			sqcall(_room->_table, "exit");
		}

		// delete all temporary objects
		for (int i = 0; i < _room->_layers.size(); i++) {
			Layer *layer = _room->_layers[i];
			for (int j = 0; j < _room->_layers.size(); j++) {
				Object *obj = layer->_objects[i];
				if (obj->_temporary) {
					obj->delObject();
				} else if (isActor(obj->getId()) && _actor != obj) {
					obj->stopObjectMotors();
				}
			}
		}

		// call global function enteredRoom with the room as argument
		sqcall("exitedRoom", 1, args);

		// stop all local threads
		for (int i = 0; i < _threads.size(); i++) {
			Thread *thread = _threads[i];
			if (!thread->_global) {
				thread->stop();
			}
		}

		// stop all lights
		_room->_lights._numLights = 0;
	}
}

void TwpEngine::setRoom(Room *room) {
	if (room && _room != room)
		enterRoom(room);
}

void TwpEngine::actorExit() {
	if (!_actor && _room) {
		if (sqrawexists(_room->_table, "actorExit")) {
			HSQOBJECT args[] = {_actor->_table};
			sqcall(_room->_table, "actorExit", 1, args);
		}
	}
}

} // End of namespace Twp
