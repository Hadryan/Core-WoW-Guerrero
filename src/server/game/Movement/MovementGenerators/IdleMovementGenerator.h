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

#ifndef monster_IDLEMOVEMENTGENERATOR_H
#define monster_IDLEMOVEMENTGENERATOR_H

#include "MovementGenerator.h"

class IdleMovementGenerator : public MovementGenerator
{
    public:

        void Initialize(Unit*);
        void Finalize(Unit*) {  }
        void Reset(Unit*);
        bool Update(Unit*, uint32) { return true; }
        MovementGeneratorType GetMovementGeneratorType() { return IDLE_MOTION_TYPE; }
};

extern IdleMovementGenerator si_idleMovement;

class RotateMovementGenerator : public MovementGenerator
{
    public:
        explicit RotateMovementGenerator(uint32 time, RotateDirection direction) : m_duration(time), m_maxDuration(time), m_direction(direction) {}

        void Initialize(Unit*);
        void Finalize(Unit*);
        void Reset(Unit* owner) { Initialize(owner); }
        bool Update(Unit*, uint32);
        MovementGeneratorType GetMovementGeneratorType() { return ROTATE_MOTION_TYPE; }

    private:
        uint32 m_duration, m_maxDuration;
        RotateDirection m_direction;
};

class DistractMovementGenerator : public MovementGenerator
{
    public:
        explicit DistractMovementGenerator(uint32 timer) : m_timer(timer) {}

        void Initialize(Unit*);
        void Finalize(Unit*);
        void Reset(Unit* owner) { Initialize(owner); }
        bool Update(Unit*, uint32);
        MovementGeneratorType GetMovementGeneratorType() { return DISTRACT_MOTION_TYPE; }

    private:
        uint32 m_timer;
};

class AssistanceDistractMovementGenerator : public DistractMovementGenerator
{
    public:
        AssistanceDistractMovementGenerator(uint32 timer) :
            DistractMovementGenerator(timer) {}

        MovementGeneratorType GetMovementGeneratorType() { return ASSISTANCE_DISTRACT_MOTION_TYPE; }
        void Finalize(Unit*);
};

template <typename T>
class CircleMovementGenerator : public MovementGeneratorMedium< T, CircleMovementGenerator<T> >
{
public:
	CircleMovementGenerator(Position const& centerPos, float radius, bool clockwise, uint8 numberOfPoints) : centerPos(centerPos), radius(radius), clockwise(clockwise), numberOfPoints(numberOfPoints) {}

	void DoInitialize(T*);
	void DoFinalize(T*);
	void DoReset(T*);
	bool DoUpdate(T*, const uint32);

	void _setCirclePath(T*);
	MovementGeneratorType GetMovementGeneratorType() { return CIRCLE_MOTION_TYPE; }
private:
	bool clockwise;
	float radius;
	uint8 numberOfPoints;
	Position centerPos;
};
#endif
