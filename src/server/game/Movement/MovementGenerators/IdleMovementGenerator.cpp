/*
 * Copyright (C) 2008-2013 monsterCore <http://www.monstercore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "IdleMovementGenerator.h"
#include "CreatureAI.h"
#include "Creature.h"
#include "MoveSpline.h"
#include "MoveSplineInit.h"
#include <G3D/g3dmath.h>

IdleMovementGenerator si_idleMovement;

// StopMoving is needed to make unit stop if its last movement generator expires
// But it should not be sent otherwise there are many redundent packets
void IdleMovementGenerator::Initialize(Unit* owner)
{
    Reset(owner);
}

void IdleMovementGenerator::Reset(Unit* owner)
{
    if (!owner->IsStopped())
        owner->StopMoving();
}

void RotateMovementGenerator::Initialize(Unit* owner)
{
    if (!owner->IsStopped())
        owner->StopMoving();

    if (owner->getVictim())
        owner->SetInFront(owner->getVictim());

    owner->AddUnitState(UNIT_STATE_ROTATING);

    owner->AttackStop();
}

bool RotateMovementGenerator::Update(Unit* owner, uint32 diff)
{
	float angle = owner->GetOrientation();
	if (m_direction == ROTATE_DIRECTION_LEFT)
	{
		angle += float(diff) * float(M_PI) * 2.f / float(m_maxDuration);
		while (angle >= float(M_PI) * 2.f)
			angle -= float(M_PI) * 2.f;
	}
	else
	{
		angle -= float(diff) * float(M_PI) * 2.f / float(m_maxDuration);
		while (angle < 0.f)
			angle += float(M_PI) * 2.f;
	}

	owner->SetFacingTo(angle);

    if (m_duration > diff)
        m_duration -= diff;
    else
        return false;

    return true;
}

void RotateMovementGenerator::Finalize(Unit* unit)
{
    unit->ClearUnitState(UNIT_STATE_ROTATING);
    if (unit->GetTypeId() == TYPEID_UNIT)
      unit->ToCreature()->AI()->MovementInform(ROTATE_MOTION_TYPE, 0);
}

void DistractMovementGenerator::Initialize(Unit* owner)
{
	// Distracted creatures stand up if not standing
	if (!owner->IsStandState())
		owner->SetStandState(UNIT_STAND_STATE_STAND);

    owner->AddUnitState(UNIT_STATE_DISTRACTED);
}

void DistractMovementGenerator::Finalize(Unit* owner)
{
    owner->ClearUnitState(UNIT_STATE_DISTRACTED);

	// If this is a creature, then return orientation to original position (for idle movement creatures)
	if (owner->GetTypeId() == TYPEID_UNIT && owner->ToCreature())
	{
		float angle = owner->ToCreature()->GetHomePosition().GetOrientation();
		owner->SetFacingTo(angle);
	}
}

bool DistractMovementGenerator::Update(Unit* /*owner*/, uint32 time_diff)
{
    if (time_diff > m_timer)
        return false;

    m_timer -= time_diff;
    return true;
}

void AssistanceDistractMovementGenerator::Finalize(Unit* unit)
{
    unit->ClearUnitState(UNIT_STATE_DISTRACTED);
    unit->ToCreature()->SetReactState(REACT_AGGRESSIVE);
}


template<>
void CircleMovementGenerator<Creature>::DoInitialize(Creature* creature)
{
	if (!creature->isAlive())
		return;


	creature->AddUnitState(UNIT_STATE_ROAMING | UNIT_STATE_ROAMING_MOVE);
	creature->SetReactState(REACT_PASSIVE);
}

template<>
void CircleMovementGenerator<Creature>::DoReset(Creature* creature)
{
	DoInitialize(creature);
}

template<>
void CircleMovementGenerator<Creature>::DoFinalize(Creature* creature)
{
	creature->ClearUnitState(UNIT_STATE_ROAMING | UNIT_STATE_ROAMING_MOVE);
	creature->SetReactState(REACT_AGGRESSIVE);
}

template<>
void CircleMovementGenerator<Creature>::_setCirclePath(Creature* creature)
{

	float step = clockwise ? -M_PI / ((float)numberOfPoints / 2.0f) : M_PI / ((float)numberOfPoints / 2.0f);
	float angle = centerPos.GetAngle(creature->GetPositionX(), creature->GetPositionY());

	Movement::MoveSplineInit init(creature);
	Movement::PointsArray& path = init.Path();
	for (uint8 i = 0; i < numberOfPoints; angle += step, ++i)
	{
		G3D::Vector3 point;
		point.x = centerPos.GetPositionX() + radius * cosf(angle);
		point.y = centerPos.GetPositionY() + radius * sinf(angle);
		point.z = creature->GetPositionZ();
		path.push_back(point);
	}
	init.SetFly();
	init.SetCyclic();
	init.Launch();
}

template<>
bool CircleMovementGenerator<Creature>::DoUpdate(Creature* creature, const uint32 diff)
{
	if (creature->movespline->Finalized())
	{
		_setCirclePath(creature);
	}
	return true;
}