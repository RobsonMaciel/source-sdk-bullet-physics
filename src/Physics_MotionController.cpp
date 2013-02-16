#include "StdAfx.h"

#include "Physics_MotionController.h"
#include "Physics_Object.h"
#include "convert.h"

#include "tier0/vprof.h"

// memdbgon must be the last include file in a .cpp file!!!
//#include "tier0/memdbgon.h"

IPhysicsMotionController *CreateMotionController(CPhysicsEnvironment *pEnv, IMotionEvent *pHandler) {
	if (!pEnv) return NULL;
	return new CPhysicsMotionController(pHandler, pEnv);
}

/***********************************
* CLASS CPhysicsMotionController
***********************************/

CPhysicsMotionController::CPhysicsMotionController(IMotionEvent *pHandler, CPhysicsEnvironment *pEnv) {
	m_handler = pHandler;
	m_pEnv = pEnv;

	//SetPriority(MEDIUM_PRIORITY);
}

void CPhysicsMotionController::Tick(float deltaTime) {
	VPROF_BUDGET("CPhysicsMotionController::Tick", VPROF_BUDGETGROUP_PHYSICS);
	if (!m_handler) return;

	for (int i = 0; i < m_objectList.Count(); i++) {
		Vector speed;
		AngularImpulse rot;
		btVector3 bullSpeed, bullRot;

		if (!m_objectList[i]) {
			m_objectList.Remove(i);
			i--;
			continue;
		}

		// Assert hit = object is DELETED!
		Assert(*(char *)m_objectList[i] != 0xDD);

		btRigidBody *body = btRigidBody::upcast(m_objectList[i]->GetObject());
		IPhysicsObject *pObject = (IPhysicsObject *)m_objectList[i];
		IMotionEvent::simresult_e ret = m_handler->Simulate(this, pObject, deltaTime, speed, rot);

		ConvertForceImpulseToBull(speed, bullSpeed);
		ConvertAngularImpulseToBull(rot, bullRot);

		switch(ret) {
			case IMotionEvent::SIM_NOTHING: {
				break;
			}
			case IMotionEvent::SIM_LOCAL_ACCELERATION: {
				btTransform transform;
				((btMassCenterMotionState *)body->getMotionState())->getGraphicTransform(transform);
				bullSpeed = transform.getBasis() * bullSpeed;

				body->setLinearVelocity(body->getLinearVelocity() + bullSpeed * deltaTime);
				body->setAngularVelocity(body->getAngularVelocity() + bullRot * deltaTime);
				break;
			}
			case IMotionEvent::SIM_LOCAL_FORCE: {
				btTransform transform;
				((btMassCenterMotionState *)body->getMotionState())->getGraphicTransform(transform);
				bullSpeed = transform.getBasis() * bullSpeed;

				body->applyCentralForce(bullSpeed * deltaTime);
				body->applyTorque(bullRot * deltaTime);
				break;
			}
			case IMotionEvent::SIM_GLOBAL_ACCELERATION: {
				body->setLinearVelocity(body->getLinearVelocity() + bullSpeed * deltaTime);
				body->setAngularVelocity(body->getAngularVelocity() + bullRot * deltaTime);
				break;
			}
			case IMotionEvent::SIM_GLOBAL_FORCE: {
				body->applyCentralForce(bullSpeed * deltaTime);
				body->applyTorque(bullRot * deltaTime);
				break;
			}
			default: {
				DevWarning("VPhysics: Invalid motion controller event type returned (%d)\n", ret);
			}
		}
	}
}

void CPhysicsMotionController::SetEventHandler(IMotionEvent *handler) {
	m_handler = handler;
}

void CPhysicsMotionController::AttachObject(IPhysicsObject *pObject, bool checkIfAlreadyAttached) {
	Assert(pObject);
	if (!pObject || pObject->IsStatic()) return;

	CPhysicsObject *pPhys = (CPhysicsObject *)pObject;

	if (m_objectList.Find(pPhys) != -1 && checkIfAlreadyAttached)
		return;

	m_objectList.AddToTail(pPhys);
}

void CPhysicsMotionController::DetachObject(IPhysicsObject *pObject) {
	CPhysicsObject *pPhys = (CPhysicsObject *)pObject;

	int index = m_objectList.Find(pPhys);
	if (!m_objectList.IsValidIndex(index)) return;
	m_objectList.Remove(index);
}

int CPhysicsMotionController::CountObjects() {
	return m_objectList.Count();
}

void CPhysicsMotionController::GetObjects(IPhysicsObject **pObjectList) {
	if (!pObjectList) return;

	for (int i = 0; i < m_objectList.Count(); i++) {
		pObjectList[i] = (IPhysicsObject *)m_objectList[i];
	}
}

void CPhysicsMotionController::ClearObjects() {
	m_objectList.Purge();
}

void CPhysicsMotionController::WakeObjects() {
	for (int i = 0; i < m_objectList.Count(); i++) {
		m_objectList[i]->GetObject()->setActivationState(ACTIVE_TAG);
	}
}

void CPhysicsMotionController::SetPriority(priority_t priority) {
	// IVP Controllers had a priority. Since bullet doesn't have controllers, this function is useless.
}
