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

#ifndef monster_RANDOMMOTIONGENERATOR_H
#define monster_RANDOMMOTIONGENERATOR_H

#include "MovementGenerator.h"

template<class T>
class RandomMovementGenerator : public MovementGeneratorMedium< T, RandomMovementGenerator<T> >
{
    public:
        RandomMovementGenerator(float spawn_dist = 0.0f, bool p_ShouldRun = false, bool p_SkipWaitChance = false)
            : i_nextMoveTime(0),
            wander_distance(spawn_dist), m_ShouldRun(p_ShouldRun), m_SkipWaitChance(p_SkipWaitChance) { }

        void _setRandomLocation(T*);
        void DoInitialize(T*);
        void DoFinalize(T*);
        void DoReset(T*);
        bool DoUpdate(T*, const uint32);
        bool GetResetPos(T*, float& x, float& y, float& z);
        MovementGeneratorType GetMovementGeneratorType() { return RANDOM_MOTION_TYPE; }
    private:
        TimeTrackerSmall i_nextMoveTime;

        uint32 i_nextMove;
        float wander_distance;
        bool m_ShouldRun;
        bool m_SkipWaitChance;
};
#endif
