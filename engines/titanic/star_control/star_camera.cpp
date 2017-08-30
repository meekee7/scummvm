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

#include "titanic/star_control/star_camera.h"
#include "titanic/debugger.h"
#include "titanic/star_control/camera_mover.h"
#include "titanic/star_control/daffine.h"
#include "titanic/star_control/fmatrix.h"
#include "titanic/star_control/fpoint.h"
#include "titanic/star_control/marked_camera_mover.h"
#include "titanic/star_control/unmarked_camera_mover.h"
#include "titanic/star_control/error_code.h"
#include "titanic/support/simple_file.h"
#include "titanic/titanic.h"

namespace Titanic {

const double rowScale1 = 100000.0;
const double rowScale2 = 1000000.0;

FMatrix *CStarCamera::_priorOrientation;
FMatrix *CStarCamera::_newOrientation;

CStarCamera::CStarCamera(const CNavigationInfo *data) :
		_starLockState(ZERO_LOCKED), _mover(nullptr), _isMoved(false) {
	setupHandler(data);
}

CStarCamera::CStarCamera(CViewport *src) :
		_starLockState(ZERO_LOCKED), _mover(nullptr), _isMoved(false), _viewport(src) {
}

void CStarCamera::init() {
	_priorOrientation = nullptr;
	_newOrientation = nullptr;
}

void CStarCamera::deinit() {
	delete _priorOrientation;
	delete _newOrientation;
	_priorOrientation = nullptr;
	_newOrientation = nullptr;
}

bool CStarCamera::isLocked() { 
	return _mover->isLocked();
}

CStarCamera::~CStarCamera() {
	deleteHandler();
}

void CStarCamera::proc2(const CViewport *src) {
	_viewport.copyFrom(src);
}

void CStarCamera::proc3(const CNavigationInfo *src) {
	_mover->copyFrom(src);
}

void CStarCamera::setPosition(const FVector &v) {
	if (!isLocked()) {
		_viewport.setPosition(v);
		setIsMoved();
	}
}

void CStarCamera::setOrientation(const FVector &v) {
	if (!isLocked())
		_viewport.setOrientation(v);
}

// This never gets called
void CStarCamera::proc6(int v) {
	if (!isLocked())
		_viewport.setC(v);
}

// This never gets called
void CStarCamera::proc7(int v) {
	if (!isLocked())
		_viewport.set10(v);
}

// This never gets called
void CStarCamera::proc8(int v) {
	if (!isLocked())
		_viewport.set14(v);
}

// This never gets called
void CStarCamera::setCenterYAngle(int v) {
	if (!isLocked())
		_viewport.setCenterYAngle(v);
}

// This never gets called
void CStarCamera::setCenterZAngle(int v) {
	if (!isLocked())
		_viewport.setCenterZAngle(v);
}

void CStarCamera::randomizeOrientation() {
	if (!isLocked())
		_viewport.randomizeOrientation();
}

void CStarCamera::proc12(StarMode mode, double v2) {
	if (!isLocked())
		_viewport.changeStarColorPixel(mode, v2);
}

void CStarCamera::proc13(CViewport *dest) {
	*dest = _viewport;
}

void CStarCamera::setDestination(const FVector &v) {
	FMatrix orientation = _viewport.getOrientation();
	FVector oldPos = _viewport._position;

	_mover->moveTo(oldPos, v, orientation);
}

void CStarCamera::updatePosition(CErrorCode *errorCode) {
	if (!_priorOrientation)
		_priorOrientation = new FMatrix();
	if (!_newOrientation)
		_newOrientation = new FMatrix();

	*_priorOrientation = _viewport.getOrientation();
	*_newOrientation = *_priorOrientation;

	FVector priorPos = _viewport._position;
	FVector newPos = _viewport._position;
	_mover->updatePosition(*errorCode, newPos, *_newOrientation);

	if (newPos != priorPos) {
		_viewport.setPosition(newPos);
		setIsMoved();
	}

	if (*_priorOrientation != *_newOrientation) {
		_viewport.setOrientation(*_newOrientation);
	}
}

void CStarCamera::increaseForwardSpeed() {
	_mover->increaseForwardSpeed();
}

void CStarCamera::increaseBackwardSpeed() {
	_mover->increaseBackwardSpeed();
}

void CStarCamera::fullSpeed() {
	_mover->fullSpeed();
}

void CStarCamera::stop() {
	_mover->stop();
}

void CStarCamera::reposition(double factor) {
	if (!isLocked())
		_viewport.reposition(factor);
}

void CStarCamera::setPosition(const FPose &pose) {
	if (!isLocked()) {
		_viewport.setPosition(pose);
		setIsMoved();
	}
}

void CStarCamera::changeOrientation(FMatrix &m) {
	if (!isLocked())
		_viewport.changeOrientation(m);
}

FPose CStarCamera::getPose() {
	return _viewport.getPose();
}

FPose CStarCamera::getRawPose() {
	return _viewport.getRawPose();
}

double CStarCamera::getThreshold() const {
	return _viewport._field10;
}

double CStarCamera::proc26() const {
	return _viewport._field14;
}

StarColor CStarCamera::getStarColor() const {
	return _viewport._starColor;
}

// Similar to CViewport::fn17/fn18
FVector CStarCamera::getRelativePos(int index, const FVector &src) {
	FVector dest;

	double val;
	if (index == 2) {
		val = _viewport._isZero;
	}
	else {
		val = _viewport._valArray[index];
	}

	dest._x = ((val + src._x) * _viewport._centerVector._x)
		/ (_viewport._centerVector._y * src._z);
	dest._y = src._y * _viewport._centerVector._x / (_viewport._centerVector._z * src._z);
	dest._z = src._z;
	return dest;
}

FVector CStarCamera::getRelativePosNoCentering(int index, const FVector &src) {
	return _viewport.getRelativePosNoCentering(index, src);
}

FVector CStarCamera::proc30(int index, const FVector &v) {
	return _viewport.getRelativePosCentering(index, v);
}

FVector CStarCamera::proc31(int index, const FVector &v) {
	return _viewport.getRelativePosCentering2(index, v);
}

void CStarCamera::setViewportAngle(const FPoint &angles) {
	debug(DEBUG_DETAILED, "setViewportAngle %f %f", angles._x, angles._y);

	if (isLocked())
		return;

	switch(_starLockState) {
	case ZERO_LOCKED: {
		FPose subX(X_AXIS, angles._y);
		FPose subY(Y_AXIS, -angles._x); // needs to be negative or looking left will cause the view to go right
		FPose sub(subX, subY);
		changeOrientation(sub);
		break;
	}

	case ONE_LOCKED: {
		FVector row1 = _lockedStarsPos._row1;
		FPose poseX(X_AXIS, angles._y);
		FPose poseY(Y_AXIS, -angles._x); // needs to be negative or looking left will cause the view to go right
		FPose pose(poseX, poseY);

		FMatrix m1 = _viewport.getOrientation();
		FVector tempV1 = _viewport._position;
		FVector tempV2 = m1._row1 * rowScale1;
		FVector tempV3 = tempV2 + tempV1;
		FVector tempV4 = tempV3;

		tempV2 = m1._row2 * rowScale1;
		FVector tempV5 = m1._row3 * rowScale1;
		FVector tempV6 = tempV2 + tempV1;

		FVector tempV7 = tempV5 + tempV1;
		tempV5 = tempV6;
		tempV6 = tempV7;

		tempV1 -= row1;
		tempV4 -= row1;
		tempV5 -= row1;
		tempV6 -= row1;

		tempV1 = tempV1.MatProdRowVect(pose);
		tempV4 = tempV4.MatProdRowVect(pose);
		tempV5 = tempV5.MatProdRowVect(pose);
		tempV6 = tempV6.MatProdRowVect(pose);

		tempV4 -= tempV1;
		tempV5 -= tempV1;
		tempV6 -= tempV1;

		float unusedScale = 0.0;
		if (!tempV4.normalize(unusedScale) ||
				!tempV5.normalize(unusedScale) ||
				!tempV6.normalize(unusedScale)) {
			// Do the normalization, put the scale amount in unusedScale,
			// but if it is unsuccessful, crash
			assert(unusedScale);
		}

		tempV1 += row1;
		m1.set(tempV4, tempV5, tempV6);
		_viewport.setOrientation(m1);
		_viewport.setPosition(tempV1);
		break;
	}

	case TWO_LOCKED: {
		FVector tempV2;
		DAffine m1, m2, sub;
		DVector mrow1, mrow2, mrow3;
		DVector tempV1, diffV, multV, multV2, tempV3, tempV4, tempV5, tempV6, tempV7;
		DVector tempV8, tempV9, tempV10, tempV11, tempV12;
		DVector tempV13, tempV14, tempV15, tempV16;

		DAffine subX(0, _lockedStarsPos._row1);
		DAffine subY(Y_AXIS, angles._y);

		tempV1 = _lockedStarsPos._row2 - _lockedStarsPos._row1;
		diffV = tempV1;
		m1 = diffV.rotXY();
		m1 = m1.compose(subX);
		subX = m1.inverseTransform();
		subX = subX.compose(subY);

		FMatrix m3 = _viewport.getOrientation();
		tempV2 = _viewport._position;
		multV._x = m3._row1._x * rowScale2;
		multV._y = m3._row1._y * rowScale2;
		multV._z = m3._row1._z * rowScale2;
		tempV3._x = tempV2._x;
		tempV3._y = tempV2._y;
		tempV3._z = tempV2._z;
		multV2._z = m3._row2._z * rowScale2;

		tempV1._x = multV._x + tempV3._x;
		tempV1._y = multV._y + tempV3._y;
		tempV1._z = multV._z + tempV3._z;
		mrow3._z = 0.0;
		mrow3._y = 0.0;
		mrow3._x = 0.0;
		multV2._x = m3._row2._x * rowScale2;
		multV2._y = m3._row2._y * rowScale2;
		mrow1 = tempV1;
		multV = multV2 + tempV3;
		mrow2 = multV;

		tempV7._z = m3._row3._z * rowScale2 + tempV3._z;
		tempV7._y = m3._row3._y * rowScale2 + tempV3._y;
		tempV7._x = m3._row3._x * rowScale2 + tempV3._x;

		mrow3 = tempV8 = tempV7;
		tempV3 = tempV3.dAffMatrixProdVec(subX);
		mrow1 = mrow1.dAffMatrixProdVec(subX);
		mrow2 = mrow2.dAffMatrixProdVec(subX);
		mrow3 = mrow3.dAffMatrixProdVec(subX);

		tempV3 = tempV3.dAffMatrixProdVec(m1);
		mrow1 = mrow1.dAffMatrixProdVec(m1);
		mrow2 = mrow2.dAffMatrixProdVec(m1);
		mrow3 = mrow3.dAffMatrixProdVec(m1);

		mrow1 -= tempV3;
		mrow2 -= tempV3;
		mrow3 -= tempV3;

		double unusedScale=0.0;
		if (!mrow1.normalize(unusedScale) ||
				!mrow2.normalize(unusedScale) ||
				!mrow3.normalize(unusedScale)) {
			// Do the normalization, put the scale amount in unusedScale,
			// but if it is unsuccessful, crash
			assert(unusedScale);
		}

		tempV16 = tempV3;

		m3.set(mrow1, mrow2, mrow3);
		_viewport.setOrientation(m3);
		_viewport.setPosition(tempV16);
		break;
	}

	// TODO: should three stars locked do anything in this function? Error?
	case THREE_LOCKED:
		break;
	}
}

bool CStarCamera::addLockedStar(const FVector v) {
	if (_starLockState == THREE_LOCKED)
		return false;

	CNavigationInfo data;
	_mover->copyTo(&data);
	deleteHandler();

	FVector &row = _lockedStarsPos[(int)_starLockState];
	_starLockState = StarLockState((int)_starLockState + 1);
	row = v;
	setupHandler(&data);
	return true;
}

bool CStarCamera::removeLockedStar() {
	if (_starLockState == ZERO_LOCKED)
		return false;

	CNavigationInfo data;
	_mover->copyTo(&data);
	deleteHandler();

	_starLockState = StarLockState((int)_starLockState - 1);
	setupHandler(&data);
	return true;
}

void CStarCamera::getRelativeXCenterPixels(double *v1, double *v2, double *v3, double *v4) {
	_viewport.getRelativeXCenterPixels(v1, v2, v3, v4);
}

void CStarCamera::load(SimpleFile *file, int param) {
	_viewport.load(file, param);
}

void CStarCamera::save(SimpleFile *file, int indent) {
	_viewport.save(file, indent);
}

bool CStarCamera::setupHandler(const CNavigationInfo *src) {
	CCameraMover *mover = nullptr;

	switch (_starLockState) {
	case ZERO_LOCKED:
		mover = new CUnmarkedCameraMover(src);
		break;

	case ONE_LOCKED:
	case TWO_LOCKED:
	case THREE_LOCKED:
		mover = new CMarkedCameraMover(src);
		break;

	default:
		break;
	}

	if (mover) {
		assert(!_mover);
		_mover = mover;
		return true;
	} else {
		return false;
	}
}

void CStarCamera::deleteHandler() {
	if (_mover) {
		delete _mover;
		_mover = nullptr;
	}
}

bool CStarCamera::lockMarker1(FVector v1, FVector firstStarPosition, FVector v3) {
	if (_starLockState != ZERO_LOCKED)
		return true;

	FVector tempV;
	double val1, val2, val3, val4, val5;
	double val6, val7, val8, val9;

	val1 = _viewport._centerVector._y * v1._x;
	tempV._z = _viewport._field10;
	val2 = _viewport._centerVector._y * tempV._z * v3._x;
	val3 = _viewport._centerVector._z * v1._y;
	val4 = _viewport._centerVector._z * tempV._z;
	val5 = val1 * v1._z / _viewport._centerVector._x;
	v3._z = v1._z;
	val6 = val4 * v3._y;
	val7 = val3 * v1._z / _viewport._centerVector._x;
	val8 = val6 / _viewport._centerVector._x;
	val9 = val2 / _viewport._centerVector._x;
	v3._x = val5 - _viewport._isZero; // TODO: _viewport._isZero is always zero
	v3._y = val7;
	tempV._x = val9 - _viewport._isZero; // TODO: _viewport._isZero is always zero
	tempV._y = val8;

	float unusedScale = 0.0;
	if (!v3.normalize(unusedScale) || !tempV.normalize(unusedScale)) {
		// Do the normalization, put the scale amount in unusedScale,
		// but if it is unsuccessful, crash
		assert(unusedScale);
	}

	FMatrix matrix = _viewport.getOrientation();
	const FVector &pos = _viewport._position;
	_mover->transitionBetweenOrientations(v3, tempV, pos, matrix); // TODO: pos does not get used in this function

	CStarVector *sv = new CStarVector(this, firstStarPosition);
	_mover->setVector(sv);
	return	true;
}

bool CStarCamera::lockMarker2(CViewport *viewport, const FVector &secondStarPosition) {
	if (_starLockState != ONE_LOCKED)
		return true;
	FVector firstStarPosition = _lockedStarsPos._row1;
	DAffine m2(0, firstStarPosition); // Identity matrix and col4 as the 1st stars position
	DVector tempV1 = secondStarPosition - firstStarPosition;
	DAffine m1 = tempV1.rotXY();
	m1 = m1.compose(m2);
	m2 = m1.inverseTransform();
	
	DVector tempV2 = _viewport._position;
	DAffine m4;
	m4._col1 = viewport->_position;
	m4._col2 = DVector(0.0, 0.0, 0.0);
	m4._col3 = DVector(0.0, 0.0, 0.0);
	m4._col4 = DVector(0.0, 0.0, 0.0);

	FMatrix m5 = viewport->getOrientation();
	double yVal1 = m5._row1._y * rowScale2;
	double zVal1 = m5._row1._z * rowScale2;
	double xVal1 = m5._row2._x * rowScale2;
	double yVal2 = m5._row2._y * rowScale2;
	double zVal2 = m5._row2._z * rowScale2;
	double zVal3 = zVal1 + m4._col1._z;
	double yVal3 = yVal1 + m4._col1._y;
	double xVal2 = m5._row1._x * rowScale2 + m4._col1._x;
	double zVal4 = zVal2 + m4._col1._z;
	double yVal4 = yVal2 + m4._col1._y;
	double xVal3 = xVal1 + m4._col1._x;

	DVector tempV4(xVal2, yVal3, zVal3);
	DVector tempV3(xVal3, yVal4, zVal4);
	m4._col3 = tempV4;

	FVector tempV5;
	tempV5._x = m5._row3._x * rowScale2;
	tempV5._y = m5._row3._y * rowScale2;
	m4._col2 = tempV3;

	tempV3._x = tempV5._x + m4._col1._x;
	tempV3._y = tempV5._y + m4._col1._y;
	tempV3._z = m5._row3._z * rowScale2 + m4._col1._z;
	m4._col4 = tempV3;

	tempV2 = tempV2.dAffMatrixProdVec(m2);
	m4._col1 = m4._col1.dAffMatrixProdVec(m2);
	m4._col3 = m4._col3.dAffMatrixProdVec(m2);
	m4._col2 = m4._col2.dAffMatrixProdVec(m2);
	m4._col4 = m4._col4.dAffMatrixProdVec(m2);

	double minDistance;
	// Find the angle of rotation for m4._col1 that gives the minimum distance to tempV2
	double minDegree = calcAngleForMinDist(tempV2,m4._col1,minDistance);

	m4._col1.rotVectAxisY((double)minDegree);
	m4._col2.rotVectAxisY((double)minDegree);
	m4._col3.rotVectAxisY((double)minDegree);
	m4._col4.rotVectAxisY((double)minDegree);
	m4._col1 = m4._col1.dAffMatrixProdVec(m1);
	m4._col2 = m4._col2.dAffMatrixProdVec(m1);
	m4._col3 = m4._col3.dAffMatrixProdVec(m1);
	m4._col4 = m4._col4.dAffMatrixProdVec(m1);

	m4._col3 -= m4._col1;
	m4._col2 -= m4._col1;
	m4._col4 -= m4._col1;



	double unusedScale=0.0;
	if (!m4._col2.normalize(unusedScale) ||
			!m4._col3.normalize(unusedScale) ||
			!m4._col4.normalize(unusedScale) ) {
		// Do the normalizations, put the scale amount in unusedScale,
		// but if any of the normalizations are unsuccessful, crash
		assert(unusedScale);
	}

	m5.set(m4._col3, m4._col2, m4._col4);

	FVector newPos = m4._col1;
	FMatrix m6 = _viewport.getOrientation();

	if (minDistance > 1.0e8) {
		// The transition will do poorly in this case.
		_mover->transitionBetweenPosOrients(_viewport._position, _viewport._position, m6, m6);
		return	false;
	}	
	else {
		_mover->transitionBetweenPosOrients(_viewport._position, newPos, m6, m5);
		CStarVector *sv = new CStarVector(this, secondStarPosition);
		_mover->setVector(sv);
	}
	return	true;
}

bool CStarCamera::lockMarker3(CViewport *viewport, const FVector &thirdStarPosition) {
	if (_starLockState != TWO_LOCKED)
		return true;

	FMatrix newOr = viewport->getOrientation();
	FMatrix oldOr = _viewport.getOrientation();
	FVector newPos = viewport->_position;
	FVector oldPos = _viewport._position;

	_mover->transitionBetweenPosOrients(oldPos, newPos, oldOr, newOr);

	CStarVector *sv = new CStarVector(this, thirdStarPosition);
	_mover->setVector(sv);
	return true;
}

double CStarCamera::calcAngleForMinDist(DVector &x, DVector &y, double &minDistance) {
	DVector tempPos;
	minDistance = 1.0e20;
	double minDegree = 0.0;
	double degInc = 1.0; // one degree steps
	int nDegrees = floor(360.0/degInc);
	for (int i = 0; i < nDegrees; ++i) {
		tempPos = y;
		tempPos.rotVectAxisY((double)degInc*i);
		double distance = x.getDistance(tempPos);

		if (distance < minDistance) {
			minDistance = distance;
			minDegree = (double) degInc*i;
		}
	}
	return minDegree;
}

} // End of namespace Titanic
