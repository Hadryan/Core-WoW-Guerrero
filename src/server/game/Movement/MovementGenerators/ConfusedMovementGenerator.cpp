/*
* Copyright (C)
* Copyright (C)
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

/*The whole scrip is converted from SC*/

#include "Creature.h"
#include "MapManager.h"
#include "ConfusedMovementGenerator.h"
#include "VMapFactory.h"
#include "MoveSplineInit.h"
#include "MoveSpline.h"
#include "Player.h"

#ifdef MAP_BASED_RAND_GEN
#define rand_norm() unit.rand_norm()
#define urand(a, b) unit.urand(a, b)
#endif

template<class T>
void ConfusedMovementGenerator<T>::DoInitialize(T* unit)
{
	unit->StopMoving();
	float wander_distance = 4;

	float x = unit->GetPositionX();
	float y = unit->GetPositionY();
	float z = unit->GetPositionZ();

	Map const* map = unit->GetBaseMap();

	bool is_water_ok, is_land_ok;
	_InitSpecific(unit, is_water_ok, is_land_ok);

	for (uint8 idx = 0; idx < MAX_CONF_WAYPOINTS + 1; ++idx)
	{
		float wanderX = x + (wander_distance * (float)rand_norm() - wander_distance / 2);
		float wanderY = y + (wander_distance * (float)rand_norm() - wander_distance / 2);

		// prevent invalid coordinates generation
		monster::NormalizeMapCoord(wanderX);
		monster::NormalizeMapCoord(wanderY);

		float new_z = map->GetHeight(unit->GetPhaseMask(), wanderX, wanderY, z, true);
		if (new_z <= INVALID_HEIGHT || fabs(z - new_z) > 3.0f) // pussywizard
		{
			i_waypoints[idx][0] = idx > 0 ? i_waypoints[idx - 1][0] : x;
			i_waypoints[idx][1] = idx > 0 ? i_waypoints[idx - 1][1] : y;
			i_waypoints[idx][2] = idx > 0 ? i_waypoints[idx - 1][2] : z;
			continue;
		}
		else if (unit->IsWithinLOS(wanderX, wanderY, z))
		{
			bool is_water = map->IsInWater(wanderX, wanderY, z);

			if ((is_water && !is_water_ok) || (!is_water && !is_land_ok))
			{
				//! Cannot use coordinates outside our InhabitType. Use the current or previous position.
				i_waypoints[idx][0] = idx > 0 ? i_waypoints[idx - 1][0] : x;
				i_waypoints[idx][1] = idx > 0 ? i_waypoints[idx - 1][1] : y;
				i_waypoints[idx][2] = idx > 0 ? i_waypoints[idx - 1][2] : z;
				continue;
			}

		}
		else
		{
			//! Trying to access path outside line of sight. Skip this by using the current or previous position.
			i_waypoints[idx][0] = idx > 0 ? i_waypoints[idx - 1][0] : x;
			i_waypoints[idx][1] = idx > 0 ? i_waypoints[idx - 1][1] : y;
			i_waypoints[idx][2] = idx > 0 ? i_waypoints[idx - 1][2] : z;
			continue;
		}

		//unit->UpdateAllowedPositionZ(wanderX, wanderY, z);

		//! Positions are fine - apply them to this waypoint
		i_waypoints[idx][0] = wanderX;
		i_waypoints[idx][1] = wanderY;
		i_waypoints[idx][2] = new_z;
	}

	//Call movement immediately to broadcast movement packet
	//Initial timer is set to 1 so update with 1
	i_nextMove = urand(1, MAX_CONF_WAYPOINTS);
	DoUpdate(unit, 1);

	unit->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_CONFUSED);
	unit->AddUnitState(UNIT_STATE_CONFUSED | UNIT_STATE_CONFUSED_MOVE);
}

template<>
void ConfusedMovementGenerator<Creature>::_InitSpecific(Creature* creature, bool &is_water_ok, bool &is_land_ok)
{
	is_water_ok = creature->CanSwim();
	is_land_ok = creature->CanWalk();
}

template<>
void ConfusedMovementGenerator<Player>::_InitSpecific(Player*, bool &is_water_ok, bool &is_land_ok)
{
	is_water_ok = true;
	is_land_ok = true;
}

template<class T>
void ConfusedMovementGenerator<T>::DoReset(T* unit)
{
	DoInitialize(unit);
}

template<class T>
bool ConfusedMovementGenerator<T>::DoUpdate(T* unit, uint32 diff)
{
	if (unit->HasUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED | UNIT_STATE_DISTRACTED))
		return true;

	if (i_nextMoveTime.Passed())
	{
		// currently moving, update location
		unit->AddUnitState(UNIT_STATE_CONFUSED_MOVE);

		if (unit->movespline->Finalized())
		{
			//Mainaz: We wanna have 2 checks. So the movement for the blind it should be way more intense compared to other effects
			// and thats the reason theres a different check for it
			if (unit->HasAura(2094))
			{
				i_nextMove = urand(1, MAX_CONF_WAYPOINTS);
				i_nextMoveTime.Reset(urand(150, 300)); // Guessed
			}
			else
			{
				i_nextMove = urand(1, MAX_CONF_WAYPOINTS);
				i_nextMoveTime.Reset(urand(600, 1200)); // Guessed
			}
		}
	}
	else
	{
		// waiting for next move
		i_nextMoveTime.Update(diff);
		if (i_nextMoveTime.Passed())
		{
			// start moving
			unit->AddUnitState(UNIT_STATE_CONFUSED_MOVE);

			ASSERT(i_nextMove <= MAX_CONF_WAYPOINTS);
			float x = i_waypoints[i_nextMove][0];
			float y = i_waypoints[i_nextMove][1];
			float z = i_waypoints[i_nextMove][2];
			Movement::MoveSplineInit init(unit);
			init.MoveTo(x, y, z);
			init.Launch();
		}
	}

	return true;
}

template<>
void ConfusedMovementGenerator<Player>::DoFinalize(Player* unit)
{
	unit->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_CONFUSED);
	unit->ClearUnitState(UNIT_STATE_CONFUSED | UNIT_STATE_CONFUSED_MOVE);
	if (!unit->HasBreakableByDamageCrowdControlAura())
	{
		unit->StopMoving();
	}
}

template<>
void ConfusedMovementGenerator<Creature>::DoFinalize(Creature* unit)
{
	unit->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_CONFUSED);
	unit->ClearUnitState(UNIT_STATE_CONFUSED | UNIT_STATE_CONFUSED_MOVE);
	if (unit->getVictim())
		unit->SetTarget(unit->getVictim()->GetGUID());
}

template void ConfusedMovementGenerator<Player>::DoInitialize(Player*);
template void ConfusedMovementGenerator<Creature>::DoInitialize(Creature*);
template void ConfusedMovementGenerator<Player>::DoReset(Player*);
template void ConfusedMovementGenerator<Creature>::DoReset(Creature*);
template bool ConfusedMovementGenerator<Player>::DoUpdate(Player*, uint32 diff);
template bool ConfusedMovementGenerator<Creature>::DoUpdate(Creature*, uint32 diff);
