/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/stark/formats/biffmesh.h"
#include "engines/stark/formats/tm.h"
#include "engines/stark/formats/xrc.h"

#include "engines/stark/gfx/driver.h"
#include "engines/stark/resources/anim.h"
#include "engines/stark/resources/animscript.h"
#include "engines/stark/resources/bonesmesh.h"
#include "engines/stark/resources/direction.h"
#include "engines/stark/resources/image.h"
#include "engines/stark/resources/item.h"
#include "engines/stark/resources/textureset.h"

#include "engines/stark/services/archiveloader.h"
#include "engines/stark/services/global.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/stateprovider.h"

#include "engines/stark/model/skeleton_anim.h"
#include "engines/stark/visual/actor.h"
#include "engines/stark/visual/prop.h"
#include "engines/stark/visual/smacker.h"

namespace Stark {
namespace Resources {

Object *Anim::construct(Object *parent, byte subType, uint16 index, const Common::String &name) {
	switch (subType) {
	case kAnimImages:
		return new AnimImages(parent, subType, index, name);
	case kAnimProp:
		return new AnimProp(parent, subType, index, name);
	case kAnimVideo:
		return new AnimVideo(parent, subType, index, name);
	case kAnimSkeleton:
		return new AnimSkeleton(parent, subType, index, name);
	default:
		error("Unknown anim subtype %d", subType);
	}
}

Anim::~Anim() {
}

Anim::Anim(Object *parent, byte subType, uint16 index, const Common::String &name) :
		Object(parent, subType, index, name),
		_usage(0),
		_currentFrame(0),
		_numFrames(0),
		_refCount(0) {
	_type = TYPE;
}

void Anim::readData(Formats::XRCReadStream *stream) {
	_usage = stream->readUint32LE();
	_numFrames = stream->readUint32LE();
}

void Anim::selectFrame(uint32 frameIndex) {
}

uint32 Anim::getUsage() const {
	return _usage;
}

void Anim::applyToItem(Item *item) {
	_refCount++;
}
void Anim::removeFromItem(Item *item) {
	_refCount--;
}

bool Anim::isInUse() {
	return _refCount > 0;
}

int Anim::getPointHotspotIndex(const Common::Point &point) const {
	// Most anim types only have one hotspot
	return 0;
}

uint32 Anim::getDuration() const {
	// TODO: Implement for each anim type
	warning("Anim::getDuration is not implemented");
	return 0;
}

void Anim::playAsAction(ItemVisual *item) {
	AnimScript *animScript = findChild<AnimScript>();
	animScript->goToScriptItem(0);
}

bool Anim::isAtTime(uint32 time) const {
	// TODO: Implement for each anim type
	warning("Anim::isAtTime is not implemented");
	return true;
}

uint32 Anim::getRemainingTime() const {
	// TODO: Implement for each anim type
	warning("Anim::getRemainingTime is not implemented");
	return 0;
}

uint32 Anim::getMovementSpeed() const {
	return 100;
}

uint32 Anim::getIdleActionFrequency() const {
	return 1;
}

void Anim::printData() {
	debug("usage: %d", _usage);
	debug("numFrames: %d", _numFrames);
}

AnimImages::~AnimImages() {
}

AnimImages::AnimImages(Object *parent, byte subType, uint16 index, const Common::String &name) :
		Anim(parent, subType, index, name),
		_field_3C(0),
		_currentDirection(0),
		_currentFrameImage(nullptr) {
}

void AnimImages::readData(Formats::XRCReadStream *stream) {
	Anim::readData(stream);

	_field_3C = stream->readFloat();
}

void AnimImages::onAllLoaded() {
	Anim::onAllLoaded();

	_directions = listChildren<Direction>();
}

void AnimImages::selectFrame(uint32 frameIndex) {
	if (frameIndex > _numFrames) {
		// The original silently ignores this as well
		warning("Request for frame %d for anim '%s' has been ignored, it is above max frame %d", frameIndex, getName().c_str(), _numFrames);
		_currentFrame = 0;
	}

	_currentFrame = frameIndex;
}

Visual *AnimImages::getVisual() {
	Direction *direction = _directions[_currentDirection];
	_currentFrameImage = direction->findChildWithIndex<Image>(_currentFrame);
	return _currentFrameImage->getVisual();
}

void AnimImages::printData() {
	Anim::printData();

	debug("field_3C: %f", _field_3C);
}

int AnimImages::getPointHotspotIndex(const Common::Point &point) const {
	if (_currentFrameImage) {
		return _currentFrameImage->indexForPoint(point);
	}
	return -1;
}

void AnimImages::saveLoad(ResourceSerializer *serializer) {
	Anim::saveLoad(serializer);

	serializer->syncAsUint32LE(_currentFrame);

	if (serializer->isLoading()) {
		selectFrame(_currentFrame);
	}
}

AnimProp::~AnimProp() {
	delete _visual;
}

AnimProp::AnimProp(Object *parent, byte subType, uint16 index, const Common::String &name) :
		Anim(parent, subType, index, name),
		_movementSpeed(100) {
	_visual = StarkGfx->createPropRenderer();
}

Visual *AnimProp::getVisual() {
	return _visual;
}

uint32 AnimProp::getMovementSpeed() const {
	return _movementSpeed;
}

void AnimProp::readData(Formats::XRCReadStream *stream) {
	Anim::readData(stream);

	_field_3C = stream->readString();

	uint32 meshCount = stream->readUint32LE();
	for (uint i = 0; i < meshCount; i++) {
		_meshFilenames.push_back(stream->readString());
	}

	_textureFilename = stream->readString();
	_movementSpeed = stream->readUint32LE();
	_archiveName = stream->getArchiveName();
}

void AnimProp::onPostRead() {
	if (_meshFilenames.size() != 1) {
		error("Unexpected mesh count in prop anim: '%d'", _meshFilenames.size());
	}

	ArchiveReadStream *stream = StarkArchiveLoader->getFile(_meshFilenames[0], _archiveName);
	_visual->setModel(Formats::BiffMeshReader::read(stream));
	delete stream;

	stream = StarkArchiveLoader->getFile(_textureFilename, _archiveName);
	_visual->setTexture(Formats::TextureSetReader::read(stream));
	delete stream;
}

void AnimProp::printData() {
	Anim::printData();

	debug("field_3C: %s", _field_3C.c_str());

	Common::String description;
	for (uint32 i = 0; i < _meshFilenames.size(); i++) {
		debug("meshFilename[%d]: %s", i, _meshFilenames[i].c_str());
	}
	debug("textureFilename: %s", _textureFilename.c_str());
	debug("movementSpeed: %d", _movementSpeed);
}

AnimVideo::~AnimVideo() {
	delete _smacker;
}

AnimVideo::AnimVideo(Object *parent, byte subType, uint16 index, const Common::String &name) :
		Anim(parent, subType, index, name),
		_width(0),
		_height(0),
		_smacker(nullptr),
		_frameRateOverride(-1),
		_preload(false),
		_loop(false),
		_actionItem(nullptr) {
}

void AnimVideo::readData(Formats::XRCReadStream *stream) {
	Anim::readData(stream);
	_smackerFile = stream->readString();
	_width = stream->readUint32LE();
	_height = stream->readUint32LE();

	_positions.clear();
	_sizes.clear();

	uint32 size = stream->readUint32LE();
	for (uint i = 0; i < size; i++) {
		_positions.push_back(stream->readPoint());
		_sizes.push_back(stream->readRect());
	}

	_loop = stream->readBool();
	_frameRateOverride = stream->readUint32LE();

	if (stream->isDataLeft()) {
		_preload = stream->readBool();
	}

	_archiveName = stream->getArchiveName();
}

void AnimVideo::onAllLoaded() {
	if (!_smacker) {
		Common::SeekableReadStream *stream = StarkArchiveLoader->getExternalFile(_smackerFile, _archiveName);

		_smacker = new VisualSmacker(StarkGfx);
		_smacker->load(stream);
		_smacker->overrideFrameRate(_frameRateOverride);
		updateSmackerPosition();
	}
}

void AnimVideo::onGameLoop() {
	if (!_smacker || !isInUse()) {
		return; // Animation not in use, no need to update the movie
	}

	if (_smacker->isDone()) {
		// The last frame has been reached
		if (!_loop && _actionItem) {
			// Reset our item if needed
			_actionItem->resetActionAnim();
			_actionItem = nullptr;
		}

		if (_loop) {
			_smacker->rewind();
		}
	}

	if (!_smacker->isDone()) {
		_smacker->update();
		updateSmackerPosition();
	}
}

void AnimVideo::onEnginePause(bool pause) {
	Object::onEnginePause(pause);

	if (_smacker && isInUse()) {
		_smacker->pause(pause);
	}
}


Visual *AnimVideo::getVisual() {
	return _smacker;
}

void AnimVideo::updateSmackerPosition() {
	int frame = _smacker->getFrameNumber();
	if (frame != -1 && frame < (int) _positions.size()) {
		_smacker->setPosition(_positions[frame]);
	}
}

uint32 AnimVideo::getDuration() const {
	return _smacker->getDuration();
}

void AnimVideo::playAsAction(ItemVisual *item) {
	_actionItem = item;

	if (!_loop) {
		_smacker->rewind();
	}
}

bool AnimVideo::isAtTime(uint32 time) const {
	uint32 currentTime = _smacker->getCurrentTime();
	return currentTime >= time;
}

void AnimVideo::saveLoadCurrent(ResourceSerializer *serializer) {
	Anim::saveLoadCurrent(serializer);

	int32 frameNumber = _smacker->getFrameNumber();
	serializer->syncAsSint32LE(frameNumber);
	serializer->syncAsSint32LE(_refCount);

	// TODO: Seek to the saved frame number when loading
}

void AnimVideo::printData() {
	Anim::printData();

	debug("smackerFile: %s", _smackerFile.c_str());
	debug("size: x %d, y %d", _width, _height);

	Common::String description;
	for (uint32 i = 0; i < _positions.size(); i++) {
		description += Common::String::format("(x %d, y %d) ", _positions[i].x, _positions[i].y);
	}
	debug("positions: %s", description.c_str());

	description.clear();
	for (uint32 i = 0; i < _sizes.size(); i++) {
		description += Common::String::format("(l %d, t %d, r %d, b %d) ",
				_sizes[i].left, _sizes[i].top, _sizes[i].right, _sizes[i].bottom);
	}
	debug("sizes: %s", description.c_str());

	debug("frameRateOverride: %d", _frameRateOverride);
	debug("preload: %d", _preload);
	debug("loop: %d", _loop);
}

AnimSkeleton::~AnimSkeleton() {
	delete _visual;
	delete _seletonAnim;
}

AnimSkeleton::AnimSkeleton(Object *parent, byte subType, uint16 index, const Common::String &name) :
		Anim(parent, subType, index, name),
		_castsShadow(true),
		_loop(false),
		_movementSpeed(100),
		_idleActionFrequency(1),
		_seletonAnim(nullptr),
		_currentTime(0),
		_totalTime(0),
		_actionItem(nullptr) {
	_visual = StarkGfx->createActorRenderer();
}

void AnimSkeleton::applyToItem(Item *item) {
	Anim::applyToItem(item);

	if (!_loop) {
		_currentTime = 0;
	}

	if (_currentTime > _totalTime) {
		_currentTime = 0;
	}

	ModelItem *modelItem = Object::cast<ModelItem>(item);

	BonesMesh *mesh = modelItem->findBonesMesh();
	TextureSet *texture = modelItem->findTextureSet(TextureSet::kTextureNormal);

	_visual->setModel(mesh->getModel());
	_visual->setAnimHandler(mesh->getAnimHandler());
	_visual->setTexture(texture->getTexture());
	_visual->setTextureFacial(nullptr);
	_visual->setAnim(_seletonAnim);
	_visual->setTime(_currentTime);
}

void AnimSkeleton::removeFromItem(Item *item) {
	Anim::removeFromItem(item);

	_actionItem = nullptr;
}

Visual *AnimSkeleton::getVisual() {
	return _visual;
}

void AnimSkeleton::readData(Formats::XRCReadStream *stream) {
	Anim::readData(stream);

	_animFilename = stream->readString();
	stream->readString(); // Skipped in the original
	stream->readString(); // Skipped in the original
	stream->readString(); // Skipped in the original

	_loop = stream->readBool();
	_movementSpeed = stream->readUint32LE();

	  if (_movementSpeed < 1) {
		_movementSpeed = 100;
	}

	if (stream->isDataLeft()) {
		_castsShadow = stream->readBool();
	} else {
		_castsShadow = true;
	}

	if (stream->isDataLeft()) {
		_idleActionFrequency = stream->readUint32LE();
	} else {
		_idleActionFrequency = 1;
	}

	_archiveName = stream->getArchiveName();
}

void AnimSkeleton::onPostRead() {
	ArchiveReadStream *stream = StarkArchiveLoader->getFile(_animFilename, _archiveName);

	_seletonAnim = new SkeletonAnim();
	_seletonAnim->createFromStream(stream);

	delete stream;
}

void AnimSkeleton::onAllLoaded() {
	Anim::onAllLoaded();

	_totalTime = _seletonAnim->getLength();
	_currentTime = 0;
}

void AnimSkeleton::onGameLoop() {
	Anim::onGameLoop();

	if (isInUse() && _totalTime) {
		uint32 newTime = _currentTime + StarkGlobal->getMillisecondsPerGameloop();

		if (!_loop && newTime >= _totalTime) {
			if (_actionItem) {
				_actionItem->resetActionAnim();
				_actionItem = nullptr;
			}
		} else {
			_currentTime = newTime % _totalTime;
			_visual->setTime(_currentTime);
		}
	}
}

void AnimSkeleton::onExitLocation() {
	Anim::onExitLocation();

	_visual->resetBlending();
}

void AnimSkeleton::onPreDestroy() {
	if (_actionItem) {
		_actionItem->resetActionAnim();
		_actionItem = nullptr;
	}

	Anim::onPreDestroy();
}

uint32 AnimSkeleton::getMovementSpeed() const {
	return _movementSpeed;
}

uint32 AnimSkeleton::getDuration() const {
	return _totalTime;
}

uint32 AnimSkeleton::getCurrentTime() const {
	return _currentTime;
}

uint32 AnimSkeleton::getRemainingTime() const {
	int32 remainingTime = _totalTime - _currentTime;
	return CLIP<int32>(remainingTime, 0, _totalTime);
}

void AnimSkeleton::playAsAction(ItemVisual *item) {
	_actionItem = item;

	if (!_loop) {
		_currentTime = 0;
	}
}

bool AnimSkeleton::isAtTime(uint32 time) const {
	return _currentTime >= time;
}

uint32 AnimSkeleton::getIdleActionFrequency() const {
	return _idleActionFrequency;
}

void AnimSkeleton::printData() {
	Anim::printData();

	debug("filename: %s", _animFilename.c_str());
	debug("castsShadow: %d", _castsShadow);
	debug("loop: %d", _loop);
	debug("movementSpeed: %d", _movementSpeed);
	debug("idleActionFrequency: %d", _idleActionFrequency);
}

} // End of namespace Resources
} // End of namespace Stark
