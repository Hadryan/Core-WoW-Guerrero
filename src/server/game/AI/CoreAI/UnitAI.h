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

#ifndef monster_UNITAI_H
#define monster_UNITAI_H

#include "Define.h"
#include "Unit.h"
#include "Containers.h"
#include <list>

class Player;
class Quest;
class Unit;
struct AISpellInfoType;

//Selection method used by SelectTarget
enum SelectAggroTarget
{
    SELECT_TARGET_RANDOM = 0,                               //Just selects a random target
    SELECT_TARGET_TOPAGGRO,                                 //Selects targes from top aggro to bottom
    SELECT_TARGET_BOTTOMAGGRO,                              //Selects targets from bottom aggro to top
    SELECT_TARGET_NEAREST,
    SELECT_TARGET_FARTHEST,
    SELECT_TARGET_RANDOM_AT_THIS_FLOOR
};

// default predicate function to select target based on distance, player and/or aura criteria
struct DefaultTargetSelector : public std::unary_function<Unit*, bool>
{
    const Unit* me;
    float m_dist;
    bool m_playerOnly;
    int32 m_aura;

    // unit: the reference unit
    // dist: if 0: ignored, if > 0: maximum distance to the reference unit, if < 0: minimum distance to the reference unit
    // playerOnly: self explaining
    // aura: if 0: ignored, if > 0: the target shall have the aura, if < 0, the target shall NOT have the aura
    DefaultTargetSelector(Unit const* unit, float dist, bool playerOnly, int32 aura) : me(unit), m_dist(dist), m_playerOnly(playerOnly), m_aura(aura) { }

    bool operator()(Unit const* target) const
    {
        if (!me)
            return false;

        if (!target)
            return false;

        if (m_playerOnly && (target->GetTypeId() != TYPEID_PLAYER))
            return false;

        if (m_dist > 0.0f && !me->IsWithinCombatRange(target, m_dist))
            return false;

        if (m_dist < 0.0f && me->IsWithinCombatRange(target, -m_dist))
            return false;

        if (m_aura)
        {
            if (m_aura > 0)
            {
                if (!target->HasAura(m_aura))
                    return false;
            }
            else
            {
                if (target->HasAura(-m_aura))
                    return false;
            }
        }

        return true;
    }
};

// Target selector for spell casts checking range, auras and attributes
// TODO: Add more checks from Spell::CheckCast
struct SpellTargetSelector : public std::unary_function<Unit*, bool>
{
    public:
        SpellTargetSelector(Unit* caster, uint32 spellId);
        bool operator()(Unit const* target) const;

    private:
        Unit const* _caster;
        SpellInfo const* _spellInfo;
};

// Very simple target selector, will just skip main target
// NOTE: When passing to UnitAI::SelectTarget remember to use 0 as position for random selection
//       because tank will not be in the temporary list
struct NonTankTargetSelector : public std::unary_function<Unit*, bool>
{
    public:
        NonTankTargetSelector(Creature* source, bool playerOnly = true) : _source(source), _playerOnly(playerOnly) { }
        bool operator()(Unit const* target) const;

    private:
        Creature const* _source;
        bool _playerOnly;
};

struct NonSpecificTargetSelector : public std::unary_function<Unit*, bool>
{
    public:
        NonSpecificTargetSelector(uint64 guid, bool playerOnly = true) : _excludeGUID(guid), _playerOnly(playerOnly) { }
        bool operator()(Unit const* target) const;

    private:
        uint64 _excludeGUID;
        bool _playerOnly;
};

struct CasterSpecTargetSelector :public std::unary_function<uint32, bool>
{
public:
    CasterSpecTargetSelector(uint32 spellId = 0) : _spellId(spellId) { }

    bool operator ()(WorldObject* target) const;

private:
    uint32 _spellId;
};

struct MeeleSpecTargetSelector :public std::unary_function<uint32, bool>
{
public:
    MeeleSpecTargetSelector(uint32 spellId = 0) : _spellId(spellId) { }

    bool operator ()(WorldObject* target) const;

private:
    uint32 _spellId;
};

struct DpsSpecTargetSelector :public std::unary_function<uint32, bool>
{
public:
    DpsSpecTargetSelector(uint32 spellId = 0) : _spellId(spellId) { }

    bool operator ()(WorldObject* target) const;

private:
    uint32 _spellId;
};

struct TankSpecTargetSelector :public std::unary_function<uint32, bool>
{
public:
    TankSpecTargetSelector(uint32 spellId = 0) : _spellId(spellId) { }

    bool operator ()(WorldObject* target) const;

private:
    uint32 _spellId;
};

struct HealerSpecTargetSelector :public std::unary_function<uint32, bool>
{
public:
    HealerSpecTargetSelector(uint32 spellId = 0) : _spellId(spellId) { }

    bool operator ()(WorldObject* target) const;

private:
    uint32 _spellId;
};

struct NonTankSpecTargetSelector :public std::unary_function<uint32, bool>
{
public:
    NonTankSpecTargetSelector(uint32 spellId = 0) : _spellId(spellId) { }

    bool operator ()(WorldObject* target) const;

private:
    uint32 _spellId;
};

class UnitAI
{
    protected:
        Unit* const me;
    public:
        explicit UnitAI(Unit* unit) : me(unit) {}
        virtual ~UnitAI() {}

        virtual bool CanAIAttack(Unit const* /*target*/) const { return true; }
        virtual void AttackStart(Unit* /*target*/);
        virtual void AttackStart(Unit* /*target*/, uint32 spellId);
        virtual void UpdateAI(uint32 const diff) = 0;
        virtual bool CanAttackTarget(Unit const* /*p_Target*/) const { return true; }

        virtual void InitializeAI() { if (!me->isDead()) Reset(); }

        virtual void Reset() {};

        // Called when unit is charmed
        virtual void OnCharmed(bool apply) = 0;

        // Pass parameters between AI
        virtual void DoAction(int32 const /*param*/) {}
        virtual uint32 GetData(uint32 /*id = 0*/) const { return 0; }
        virtual void SetData(uint32 /*id*/, uint32 /*value*/) {}
        virtual void SetGUID(uint64 /*guid*/, int32 /*id*/ = 0) {}
        virtual uint64 GetGUID(int32 /*id*/ = 0) const { return 0; }

        Unit* SelectTarget(SelectAggroTarget targetType, uint32 position = 0, float dist = 0.0f, bool playerOnly = false, int32 aura = 0);
        // Select the targets satifying the predicate.
        // predicate shall extend std::unary_function<Unit*, bool>
        template <class PREDICATE> Unit* SelectTarget(SelectAggroTarget targetType, uint32 position, PREDICATE const& predicate)
        {
            ThreatContainer::StorageType const& threatlist = me->getThreatManager().getThreatList();
            if (position >= threatlist.size())
                return NULL;

            std::list<Unit*> targetList;
            for (ThreatContainer::StorageType::const_iterator itr = threatlist.begin(); itr != threatlist.end(); ++itr)
                if (predicate((*itr)->getTarget()))
                    targetList.push_back((*itr)->getTarget());

            if (position >= targetList.size())
                return NULL;

            if (targetType == SELECT_TARGET_NEAREST || targetType == SELECT_TARGET_FARTHEST)
                targetList.sort(monster::ObjectDistanceOrderPred(me));

            switch (targetType)
            {
                case SELECT_TARGET_NEAREST:
                case SELECT_TARGET_TOPAGGRO:
                {
                    std::list<Unit*>::iterator itr = targetList.begin();
                    std::advance(itr, position);
                    return *itr;
                }
                case SELECT_TARGET_FARTHEST:
                case SELECT_TARGET_BOTTOMAGGRO:
                {
                    std::list<Unit*>::reverse_iterator ritr = targetList.rbegin();
                    std::advance(ritr, position);
                    return *ritr;
                }
                case SELECT_TARGET_RANDOM:
                {
                    std::list<Unit*>::iterator itr = targetList.begin();
                    std::advance(itr, urand(position, targetList.size() - 1));
                    return *itr;
                }
                case SELECT_TARGET_RANDOM_AT_THIS_FLOOR:
                {
                    // clone current threat list..
                    std::list<Unit*> filteredTargetList;
                    for (ThreatContainer::StorageType::const_iterator itr = threatlist.begin(); itr != threatlist.end(); ++itr)
                    {
                        if (predicate((*itr)->getTarget()))
                        {
                            float zDiff = (*itr)->getTarget()->GetPositionZ() - me->GetPositionZ();
                            if (zDiff < 0.0f)
                                zDiff *= -1;

                            // Just few tollerance..
                            if (zDiff <= 2.0)
                                filteredTargetList.push_back((*itr)->getTarget());
                        }
                    }

                    // Keep one random!
                    if (position >= filteredTargetList.size())
                        return NULL;

                    std::list<Unit*>::iterator itr = filteredTargetList.begin();
                    std::advance(itr, urand(position, filteredTargetList.size() - 1));
                    return *itr;
                }
                default:
                    break;
            }

            return NULL;
        }

        void SelectTargetList(std::list<Unit*>& targetList, uint32 num, SelectAggroTarget targetType, float dist = 0.0f, bool playerOnly = false, int32 aura = 0);

        // Select the targets satifying the predicate.
        // predicate shall extend std::unary_function<Unit*, bool>
        template <class PREDICATE> void SelectTargetList(std::list<Unit*>& targetList, PREDICATE const& predicate, uint32 maxTargets, SelectAggroTarget targetType)
        {
            ThreatContainer::StorageType const& threatlist = me->getThreatManager().getThreatList();
            if (threatlist.empty())
                return;

            for (ThreatContainer::StorageType::const_iterator itr = threatlist.begin(); itr != threatlist.end(); ++itr)
                if (predicate((*itr)->getTarget()))
                    targetList.push_back((*itr)->getTarget());

            if (targetList.size() < maxTargets)
                maxTargets = targetList.size();

            if (targetType == SELECT_TARGET_NEAREST || targetType == SELECT_TARGET_FARTHEST)
                targetList.sort(monster::ObjectDistanceOrderPred(me));

            if (targetType == SELECT_TARGET_FARTHEST || targetType == SELECT_TARGET_BOTTOMAGGRO)
                targetList.reverse();

            if (targetType == SELECT_TARGET_RANDOM || targetType == SELECT_TARGET_RANDOM_AT_THIS_FLOOR)
                monster::Containers::RandomResizeList(targetList, maxTargets);
            else
                targetList.resize(maxTargets);
        }

        virtual void HandleReturnMovement() {}

        // Called at any Damage to any victim (before damage apply)
        virtual void DamageDealt(Unit* /*victim*/, uint32& /*damage*/, DamageEffectType /*damageType*/) { }

        // Called at any Damage from any attacker (before damage apply)
        // Note: it for recalculation damage or special reaction at damage
        // for attack reaction use AttackedBy called for not DOT damage in Unit::DealDamage also
        virtual void DamageTaken(Unit* /*attacker*/, uint32& /*damage*/) {}

        // Called when the creature receives heal
        virtual void HealReceived(Unit* /*done_by*/, uint32& /*addhealth*/) {}

        // Called when the unit heals
        virtual void HealDone(Unit* /*done_to*/, uint32& /*addhealth*/) {}

        /// Called when a spell is interrupted by Spell::EffectInterruptCast
        /// Use to reschedule next planned cast of spell.
        virtual void SpellInterrupted(uint32 /*spellId*/, uint32 /*unTimeMs*/) {}

        /// Called when this unit enters a vehicle
        virtual void OnEnterVehicle(Unit* /*p_Unit*/) { }

        /// Called when this unit leaves a vehicle
        virtual void OnLeaveVehicle(Unit* /*p_Unit*/) { }

        /// Called when this unit requests to exit a vehicle
        virtual void OnVehicleExitRequested(Unit* /*p_Unit*/) { }

        void AttackStartCaster(Unit* victim, float dist);

        void DoAddAuraToAllHostilePlayers(uint32 spellid);
        void DoCast(uint32 spellId);
        void DoCast(Unit* victim, uint32 spellId, bool triggered = false);
        void DoCastToAllHostilePlayers(uint32 spellid, bool triggered = false);
        void DoCastVictim(uint32 spellId, bool triggered = false);
        void DoCastAOE(uint32 spellId, bool triggered = false);
        void DoCastRandom(uint32 spellId, float dist, bool triggered = false, int32 aura = 0, uint32 position = 0); // player only

        float DoGetSpellMaxRange(uint32 spellId, bool positive = false);

        void DoMeleeAttackIfReady();
        bool DoSpellAttackIfReady(uint32 spell);

        static AISpellInfoType* AISpellInfo;
        static void FillAISpellInfo();

        virtual void sGossipHello(Player* /*player*/) {}
        virtual void sGossipSelect(Player* /*player*/, uint32 /*sender*/, uint32 /*action*/) {}
        virtual void sGossipSelectCode(Player* /*player*/, uint32 /*sender*/, uint32 /*action*/, char const* /*code*/) {}
        virtual bool sQuestAccept(Player* /*player*/, Quest const* /*quest*/) { return false; }
        virtual bool sQuestSelect(Player* /*player*/, Quest const* /*quest*/) { return false; }
        virtual bool sQuestObjectiveComplete(Player* /*player*/, Quest const* /*quest*/) { return false; }
        virtual bool sQuestReward(Player* /*player*/, Quest const* /*quest*/, uint32 /*opt*/) { return false; }
        virtual bool sOnDummyEffect(Unit* /*caster*/, uint32 /*spellId*/, SpellEffIndex /*effIndex*/) { return false; }
        virtual void sOnGameEvent(bool /*start*/, uint16 /*eventId*/) {}
};

class PlayerAI : public UnitAI
{
    protected:
        Player* const me;
    public:
        explicit PlayerAI(Player* player) : UnitAI((Unit*)player), me(player) {}

        void OnCharmed(bool apply);
};

class SimpleCharmedAI : public PlayerAI
{
    public:
        void UpdateAI(uint32 const diff);
        SimpleCharmedAI(Player* player): PlayerAI(player) {}
};

#endif
