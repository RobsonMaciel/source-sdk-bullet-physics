/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2003-2006 Erwin Coumans  http://continuousphysics.com/Bullet/

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#ifndef BT_SOLVER_BODY_H
#define BT_SOLVER_BODY_H

class	btRigidBody;
class	btCollisionObject;	

#include "LinearMath/btVector3.h"
#include "LinearMath/btMatrix3x3.h"

#include "LinearMath/btAlignedAllocator.h"
#include "LinearMath/btTransformUtil.h"

///The btSolverBody is an internal datastructure for the constraint solver. Only necessary data is packed to increase cache coherence/performance.
ATTRIBUTE_ALIGNED16 (struct)	btSolverBody
{
	BT_DECLARE_ALIGNED_ALLOCATOR();
	btTransform		m_worldTransform;
	btVector3		m_deltaLinearVelocity;
	btVector3		m_deltaAngularVelocity;
	btVector3		m_angularFactor;
	btVector3		m_linearFactor;
	btVector3		m_invMass;
	btVector3		m_pushVelocity;
	btVector3		m_turnVelocity;
	btVector3		m_linearVelocity;
	btVector3		m_angularVelocity;
	btVector3		m_externalForceImpulse;
	btVector3		m_externalTorqueImpulse;

	btRigidBody*	m_originalBody;
	btCollisionObject* m_originalColObj;
	bool			m_bFixed;

	void	setWorldTransform(const btTransform& worldTransform)
	{
		m_worldTransform = worldTransform;
	}

	const btTransform& getWorldTransform() const
	{
		return m_worldTransform;
	}
	
	

	SIMD_FORCE_INLINE void	getVelocityInLocalPointNoDelta(const btVector3& rel_pos, btVector3& velocity ) const
	{
		if (m_originalBody)
			velocity = m_linearVelocity + m_externalForceImpulse + (m_angularVelocity+m_externalTorqueImpulse).cross(rel_pos);
		else
			velocity.setValue(0,0,0);
	}


	SIMD_FORCE_INLINE void	getVelocityInLocalPointObsolete(const btVector3& rel_pos, btVector3& velocity ) const
	{
		if (m_originalBody)
			velocity = m_linearVelocity+m_deltaLinearVelocity + (m_angularVelocity+m_deltaAngularVelocity).cross(rel_pos);
		else
			velocity.setValue(0,0,0);
	}

	SIMD_FORCE_INLINE void	getAngularVelocity(btVector3& angVel) const
	{
		if (m_originalBody)
			angVel =m_angularVelocity+m_deltaAngularVelocity;
		else
			angVel.setValue(0,0,0);
	}


	//Optimization for the iterative solver: avoid calculating constant terms involving inertia, normal, relative position
	SIMD_FORCE_INLINE void applyImpulse(const btVector3& linearComponent, const btVector3& angularComponent, const btScalar impulseMagnitude)
	{
		if (m_originalBody)
		{
			m_deltaLinearVelocity += linearComponent*impulseMagnitude*m_linearFactor;
			m_deltaAngularVelocity += angularComponent*(impulseMagnitude*m_angularFactor);
		}
	}

	SIMD_FORCE_INLINE void internalApplyPushImpulse(const btVector3& linearComponent, const btVector3& angularComponent, btScalar impulseMagnitude)
	{
		if (m_originalBody)
		{
			m_pushVelocity += linearComponent*impulseMagnitude*m_linearFactor;
			m_turnVelocity += angularComponent*(impulseMagnitude*m_angularFactor);
		}
	}



	const btVector3& getDeltaLinearVelocity() const
	{
		return m_deltaLinearVelocity;
	}

	const btVector3& getDeltaAngularVelocity() const
	{
		return m_deltaAngularVelocity;
	}

	const btVector3& getPushVelocity() const 
	{
		return m_pushVelocity;
	}

	const btVector3& getTurnVelocity() const 
	{
		return m_turnVelocity;
	}


	////////////////////////////////////////////////
	///some internal methods, don't use them
		
	btVector3& internalGetDeltaLinearVelocity()
	{
		return m_deltaLinearVelocity;
	}

	btVector3& internalGetDeltaAngularVelocity()
	{
		return m_deltaAngularVelocity;
	}

	const btVector3& internalGetAngularFactor() const
	{
		return m_angularFactor;
	}

	const btVector3& internalGetInvMass() const
	{
		return m_invMass;
	}

	void internalSetInvMass(const btVector3& invMass)
	{
		m_invMass = invMass;
	}
	
	btVector3& internalGetPushVelocity()
	{
		return m_pushVelocity;
	}

	btVector3& internalGetTurnVelocity()
	{
		return m_turnVelocity;
	}

	SIMD_FORCE_INLINE void	internalGetVelocityInLocalPointObsolete(const btVector3& rel_pos, btVector3& velocity ) const
	{
		velocity = m_linearVelocity+m_deltaLinearVelocity + (m_angularVelocity+m_deltaAngularVelocity).cross(rel_pos);
	}

	SIMD_FORCE_INLINE void	internalGetAngularVelocity(btVector3& angVel) const
	{
		angVel = m_angularVelocity+m_deltaAngularVelocity;
	}


	//Optimization for the iterative solver: avoid calculating constant terms involving inertia, normal, relative position
	SIMD_FORCE_INLINE void internalApplyImpulse(const btVector3& linearComponent, const btVector3& angularComponent, const btScalar impulseMagnitude)
	{
		if (m_originalBody)
		{
			m_deltaLinearVelocity += linearComponent*impulseMagnitude*m_linearFactor;
			m_deltaAngularVelocity += angularComponent*(impulseMagnitude*m_angularFactor);
		}
	}
		
	
	

	void	writebackVelocity()
	{
		if (m_originalBody)
		{
			m_linearVelocity +=m_deltaLinearVelocity;
			m_angularVelocity += m_deltaAngularVelocity;
			
			//m_originalBody->setCompanionId(-1);
		}
	}


	void	writebackVelocityAndTransform(btScalar timeStep, btScalar splitImpulseTurnErp)
	{
		(void) timeStep;
		if (m_originalBody)
		{
			m_linearVelocity += m_deltaLinearVelocity;
			m_angularVelocity += m_deltaAngularVelocity;
			
			//correct the position/orientation based on push/turn recovery
			btTransform newTransform;
			if (!m_pushVelocity.fuzzyZero() || !m_turnVelocity.fuzzyZero())
			{
			//	btQuaternion orn = m_worldTransform.getRotation();
				btTransformUtil::integrateTransform(m_worldTransform, m_pushVelocity, m_turnVelocity*splitImpulseTurnErp, timeStep, newTransform);
				m_worldTransform = newTransform;
			}
			//m_worldTransform.setRotation(orn);
			//m_originalBody->setCompanionId(-1);
		}
	}
	


};

#endif //BT_SOLVER_BODY_H


