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

#include "BattlegroundMgr.h"
#include "CellImpl.h"
#include "Common.h"
#include "CreatureAI.h"
#include "CreatureAISelector.h"
#include "CreatureGroups.h"
#include "Creature.h"
#include "CreatureTextMgr.h"
#include "DatabaseEnv.h"
#include "Formulas.h"
#include "GameEventMgr.h"
#include "GossipDef.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Group.h"
#include "GroupMgr.h"
#include "InstanceScript.h"
#include "Log.h"
#include "LootMgr.h"
#include "MapManager.h"
#include "MoveSpline.h"
#include "MoveSplineInit.h"
#include "ObjectMgr.h"
#include "Opcodes.h"
#include "OutdoorPvPMgr.h"
#include "Player.h"
#include "PoolMgr.h"
#include "QuestDef.h"
#include "SpellAuraEffects.h"
#include "SpellMgr.h"
#include "TemporarySummon.h"
#include "Util.h"
#include "Vehicle.h"
#include "WaypointMovementGenerator.h"
#include "World.h"
#include "WorldPacket.h"
#include "Pet.h"

// apply implementation of the singletons

// npcbot
#include "bot_ai.h"

TrainerSpell const* TrainerSpellData::Find(uint32 spell_id) const
{
    TrainerSpellMap::const_iterator itr = spellList.find(spell_id);
    if (itr != spellList.end())
        return &itr->second;

    return NULL;
}

bool VendorItemData::RemoveItem(uint32 item_id, uint8 type)
{
    bool found = false;
    for (VendorItemList::iterator i = m_items.begin(); i != m_items.end();)
    {
        if ((*i)->item == item_id && (*i)->Type == type)
        {
            i = m_items.erase(i++);
            found = true;
        }
        else
            ++i;
    }
    return found;
}

VendorItem const* VendorItemData::FindItemCostPair(uint32 item_id, uint32 extendedCost, uint8 type) const
{
    for (VendorItemList::const_iterator i = m_items.begin(); i != m_items.end(); ++i)
        if ((*i)->item == item_id && (*i)->ExtendedCost == extendedCost && (*i)->Type == type)
            return *i;
    return NULL;
}

uint32 CreatureTemplate::GetRandomValidModelId() const
{
    uint8 c = 0;
    uint32 modelIDs[4];

    if (Modelid1) modelIDs[c++] = Modelid1;
    if (Modelid2) modelIDs[c++] = Modelid2;
    if (Modelid3) modelIDs[c++] = Modelid3;
    if (Modelid4) modelIDs[c++] = Modelid4;

    return ((c>0) ? modelIDs[urand(0, c-1)] : 0);
}

uint32 CreatureTemplate::GetFirstValidModelId() const
{
    if (Modelid1) return Modelid1;
    if (Modelid2) return Modelid2;
    if (Modelid3) return Modelid3;
    if (Modelid4) return Modelid4;
    return 0;
}

bool AssistDelayEvent::Execute(uint64 /*e_time*/, uint32 /*p_time*/)
{
    if (Unit* victim = Unit::GetUnit(m_owner, m_victim))
    {
        while (!m_assistants.empty())
        {
            Creature* assistant = Unit::GetCreature(m_owner, *m_assistants.begin());
            m_assistants.pop_front();

            if (assistant && assistant->CanAssistTo(&m_owner, victim))
            {
                assistant->SetNoCallAssistance(true);
                assistant->CombatStart(victim);
                if (assistant->IsAIEnabled)
                    assistant->AI()->AttackStart(victim);
            }
        }
    }
    return true;
}

CreatureBaseStats const* CreatureBaseStats::GetBaseStats(uint8 level, uint8 unitClass)
{
    return sObjectMgr->GetCreatureBaseStats(level, unitClass);
}

bool ForcedDespawnDelayEvent::Execute(uint64 /*e_time*/, uint32 /*p_time*/)
{
    m_owner.DespawnOrUnsummon();    // since we are here, we are not TempSummon as object type cannot change during runtime
    return true;
}

Creature::Creature(bool isWorldObject): Unit(isWorldObject), MapObject(),
lootForPickPocketed(false), lootForBody(false), m_groupLootTimer(0), lootingGroupLowGUID(0),
m_PlayerDamageReq(0), m_lootRecipient(0), m_lootRecipientGroup(0), m_corpseRemoveTime(0), m_respawnTime(0),
m_respawnDelay(300), m_corpseDelay(60), m_respawnradius(0.0f), m_walkmode(0.0f), m_saiscriptflag(0.0f), m_reactState(REACT_AGGRESSIVE), m_canRegen(true),
m_defaultMovementType(IDLE_MOTION_TYPE), m_DBTableGuid(0), m_equipmentId(0), m_AlreadyCallAssistance(false),
m_AlreadySearchedAssistance(false), m_regenHealth(true), m_cannotReachTarget(false), m_cannotReachTimer(0), m_AI_locked(false), m_meleeDamageSchoolMask(SPELL_SCHOOL_MASK_NORMAL),
m_creatureInfo(NULL), m_creatureData(NULL), m_path_id(0), m_formation(NULL), tmpEnergyReg(0.0f), m_SparringIsCanceled(false), m_IgnoreUpdateMovementFlag(false)
{
    m_regenTimer = CREATURE_REGEN_INTERVAL;
    m_regenTimerEnergy = CREATURE_REGEN_ENERGY_INTERVAL;
    m_valuesCount = UNIT_END;

    for (uint8 i = 0; i < CREATURE_MAX_SPELLS; ++i)
        m_spells[i] = 0;

    DisableReputationGain = false;

    m_SightDistance = sWorld->getFloatConfig(CONFIG_SIGHT_MONSTER);
    m_CombatDistance = 0;//MELEE_RANGE;

    ResetLootMode(); // restore default loot mode
    TriggerJustRespawned = false;
    m_isTempWorldObject = false;

	m_bot_owner = NULL;
    m_creature_owner = NULL;
    m_bots_pet = NULL;
    m_bot_class = CLASS_NONE;
    bot_AI = NULL;
    m_canUpdate = true;

	// Aps: Used by summons
	InitIdleMovement();
}

Creature::~Creature()
{
    m_vendorItemCounts.clear();

    delete i_AI;
    i_AI = NULL;

    //if (m_uint32Values)
    //    TC_LOG_ERROR("entities.unit", "Deconstruct Creature Entry = %u", GetEntry());
}

void Creature::InitIdleMovement()
{
	m_idle = false;
	m_idleTimer = 0;
}

void Creature::AddToWorld()
{
    ///- Register the creature for guid lookup
    if (!IsInWorld())
    {
        if (m_zoneScript)
            m_zoneScript->OnCreatureCreate(this);
        sObjectAccessor->AddObject(this);
        Unit::AddToWorld();
        SearchFormation();
        AIM_Initialize();
        if (IsVehicle())
            GetVehicleKit()->Install();

        if (CreatureAddon const* creatureAddon = GetCreatureAddon())
             m_visibleDistance = creatureAddon->VisibilityDistanceType;
        else
             m_visibleDistance = VD_LARGE;
    }
}

void Creature::RemoveFromWorld()
{
    if (IsInWorld())
    {
        if (m_zoneScript)
            m_zoneScript->OnCreatureRemove(this);
        if (m_formation)
            sFormationMgr->RemoveCreatureFromGroup(m_formation, this);
        Unit::RemoveFromWorld();
        sObjectAccessor->RemoveObject(this);
    }
}

void Creature::DisappearAndDie()
{
    DestroyForNearbyPlayers();
    //SetVisibility(VISIBILITY_OFF);
    //ObjectAccessor::UpdateObjectVisibility(this);
    if (isAlive())
        setDeathState(JUST_DIED);
    RemoveCorpse(false);
}

void Creature::SearchFormation()
{
    if (isSummon())
        return;

    uint32 lowguid = GetDBTableGUIDLow();
    if (!lowguid)
        return;

    CreatureGroupInfoType::iterator frmdata = sFormationMgr->CreatureGroupMap.find(lowguid);
    if (frmdata != sFormationMgr->CreatureGroupMap.end())
        sFormationMgr->AddCreatureToGroup(frmdata->second->leaderGUID, this);
}

void Creature::RemoveCorpse(bool setSpawnTime)
{
    if (getDeathState() != CORPSE)
        return;

    if (bot_AI)
        return;

    m_corpseRemoveTime = time(NULL);
    setDeathState(DEAD);
    RemoveAllAuras();
 //   UpdateObjectVisibility(true);
    loot.clear();
    uint32 respawnDelay = m_respawnDelay;
    if (IsAIEnabled)
        AI()->CorpseRemoved(respawnDelay);

    // Should get removed later, just keep "compatibility" with scripts
    if (setSpawnTime)
        m_respawnTime = time(NULL) + respawnDelay;

     // if corpse was removed during falling, the falling will continue and override relocation to respawn position
     if (IsFalling())
         StopMoving();

    float x, y, z, o;
    GetRespawnPosition(x, y, z, &o);
    SetHomePosition(x, y, z, o);
    Relocate(x, y, z, o);
    GetMap()->CreatureRelocation(this, x, y, z, o);
}

/**
 * change the entry of creature until respawn
 */
bool Creature::InitEntry(uint32 Entry, uint32 /*team*/, const CreatureData* data)
{
    CreatureTemplate const* normalInfo = sObjectMgr->GetCreatureTemplate(Entry);
    if (!normalInfo)
    {
        TC_LOG_ERROR("sql.sql", "Creature::InitEntry creature entry %u does not exist.", Entry);
        return false;
    }

    // get difficulty 1 mode entry
    CreatureTemplate const* cinfo = normalInfo;
    for (uint8 diff = uint8(GetMap()->GetSpawnMode()); diff > 0;)
    {
        // we already have valid Map pointer for current creature!
        if (normalInfo->DifficultyEntry[diff - 1])
        {
            cinfo = sObjectMgr->GetCreatureTemplate(normalInfo->DifficultyEntry[diff - 1]);
            if (cinfo)
                break;                                      // template found

            // check and reported at startup, so just ignore (restore normalInfo)
            cinfo = normalInfo;
        }

        // for instances heroic to normal, other cases attempt to retrieve previous difficulty
        if (diff >= RAID_DIFFICULTY_10MAN_HEROIC && GetMap()->IsRaid())
            diff -= 2;                                      // to normal raid difficulty cases
        else
            --diff;
    }

    // Initialize loot duplicate count depending on raid difficulty
    if (GetMap()->Is25ManRaid())
        loot.maxDuplicates = 3;

    SetEntry(Entry);                                        // normal entry always
    m_creatureInfo = cinfo;                                 // map mode related always

    // equal to player Race field, but creature does not have race
    SetByteValue(UNIT_FIELD_BYTES_0, 0, 0);

    // known valid are: CLASS_WARRIOR, CLASS_PALADIN, CLASS_ROGUE, CLASS_MAGE
    SetByteValue(UNIT_FIELD_BYTES_0, 1, uint8(cinfo->unit_class));

    // Cancel load if no model defined
    if (!(cinfo->GetFirstValidModelId()))
    {
        TC_LOG_ERROR("sql.sql", "Creature (Entry: %u) has no model defined in table `creature_template`, can't load. ", Entry);
        return false;
    }

    uint32 displayID = sObjectMgr->ChooseDisplayId(0, GetCreatureTemplate(), data);
    CreatureModelInfo const* minfo = sObjectMgr->GetCreatureModelRandomGender(&displayID);
    if (!minfo)                                             // Cancel load if no model defined
    {
        TC_LOG_ERROR("sql.sql", "Creature (Entry: %u) has no model defined in table `creature_template`, can't load. ", Entry);
        return false;
    }

    SetDisplayId(displayID);
    SetNativeDisplayId(displayID);
    SetByteValue(UNIT_FIELD_BYTES_0, 2, minfo->gender);

    // Load creature equipment
    if (!data || data->equipmentId == 0)                    // use default from the template
        LoadEquipment(cinfo->equipmentId);
    else if (data && data->equipmentId != -1)               // override, -1 means no equipment
        LoadEquipment(data->equipmentId);

    SetName(normalInfo->Name);                              // at normal entry always

    SetFloatValue(UNIT_MOD_CAST_SPEED, 1.0f);
    SetFloatValue(UNIT_MOD_CAST_HASTE, 1.0f);

    SetSpeed(MOVE_WALK,     cinfo->speed_walk);
    SetSpeed(MOVE_RUN,      cinfo->speed_run);
    SetSpeed(MOVE_SWIM,     cinfo->speed_swim);
    SetSpeed(MOVE_FLIGHT,   cinfo->speed_fly);

    // Will set UNIT_FIELD_BOUNDINGRADIUS and UNIT_FIELD_COMBATREACH
    SetObjectScale(cinfo->scale);

    SetFloatValue(UNIT_FIELD_HOVERHEIGHT, cinfo->HoverHeight);

    // checked at loading
    m_defaultMovementType = MovementGeneratorType(cinfo->MovementType);
    if (!m_respawnradius && m_defaultMovementType == RANDOM_MOTION_TYPE)
        m_defaultMovementType = IDLE_MOTION_TYPE;

    for (uint8 i=0; i < CREATURE_MAX_SPELLS; ++i)
        m_spells[i] = GetCreatureTemplate()->spells[i];

    return true;
}

bool Creature::UpdateEntry(uint32 Entry, uint32 team, const CreatureData* data, bool updateLevel /* = true */)
{
    if (!InitEntry(Entry, team, data))
        return false;

    CreatureTemplate const* cInfo = GetCreatureTemplate();

    m_regenHealth = cInfo->RegenHealth;

    m_SparringIsCanceled = !sObjectMgr->DoesCreatureHaveSparringInfo(Entry);

    // creatures always have melee weapon ready if any unless specified otherwise
    if (!GetCreatureAddon())
        SetSheath(SHEATH_STATE_MELEE);

    SelectLevel(GetCreatureTemplate());
    if (team == HORDE)
        setFaction(cInfo->faction_H);
    else
        setFaction(cInfo->faction_A);

    uint32 npcflag, unit_flags, dynamicflags;
    ObjectMgr::ChooseCreatureFlags(cInfo, npcflag, unit_flags, dynamicflags, data);

    if (cInfo->flags_extra & CREATURE_FLAG_EXTRA_WORLDEVENT)
        SetUInt32Value(UNIT_NPC_FLAGS, npcflag | sGameEventMgr->GetNPCFlag(this));
    else
        SetUInt32Value(UNIT_NPC_FLAGS, npcflag);

    SetAttackTime(BASE_ATTACK,   cInfo->baseattacktime);
    SetAttackTime(OFF_ATTACK,    CalculatePct(cInfo->baseattacktime, 90));
    SetAttackTime(RANGED_ATTACK, cInfo->rangeattacktime);

    SetUInt32Value(UNIT_FIELD_FLAGS, unit_flags);
    SetUInt32Value(UNIT_FIELD_FLAGS_2, cInfo->unit_flags2);

    SetUInt32Value(UNIT_DYNAMIC_FLAGS, dynamicflags);

    RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);

    if (updateLevel)
        SelectLevel(GetCreatureTemplate());

    SetMeleeDamageSchool(SpellSchools(cInfo->dmgschool));
    CreatureBaseStats const* stats = sObjectMgr->GetCreatureBaseStats(getLevel(), cInfo->unit_class);
    float armor = (float)stats->GenerateArmor(cInfo); // TODO: Why is this treated as uint32 when it's a float?
    SetModifierValue(UNIT_MOD_ARMOR,             BASE_VALUE, armor);
    SetModifierValue(UNIT_MOD_RESISTANCE_HOLY,   BASE_VALUE, float(cInfo->resistance[SPELL_SCHOOL_HOLY]));
    SetModifierValue(UNIT_MOD_RESISTANCE_FIRE,   BASE_VALUE, float(cInfo->resistance[SPELL_SCHOOL_FIRE]));
    SetModifierValue(UNIT_MOD_RESISTANCE_NATURE, BASE_VALUE, float(cInfo->resistance[SPELL_SCHOOL_NATURE]));
    SetModifierValue(UNIT_MOD_RESISTANCE_FROST,  BASE_VALUE, float(cInfo->resistance[SPELL_SCHOOL_FROST]));
    SetModifierValue(UNIT_MOD_RESISTANCE_SHADOW, BASE_VALUE, float(cInfo->resistance[SPELL_SCHOOL_SHADOW]));
    SetModifierValue(UNIT_MOD_RESISTANCE_ARCANE, BASE_VALUE, float(cInfo->resistance[SPELL_SCHOOL_ARCANE]));

    SetCanModifyStats(true);
    UpdateAllStats();

    // checked and error show at loading templates
    if (FactionTemplateEntry const* factionTemplate = sFactionTemplateStore.LookupEntry(cInfo->faction_A))
    {
        if (factionTemplate->factionFlags & FACTION_TEMPLATE_FLAG_PVP)
            SetPvP(true);
        else
            SetPvP(false);
    }

    // updates spell bars for vehicles and set player's faction - should be called here, to overwrite faction that is set from the new template
    if (IsVehicle())
    {
        if (Player* owner = Creature::GetCharmerOrOwnerPlayerOrPlayerItself()) // this check comes in case we don't have a player
        {
            setFaction(owner->getFaction()); // vehicles should have same as owner faction
            owner->VehicleSpellInitialize();
        }
    }

    // trigger creature is always not selectable and can not be attacked
    if (isTrigger())
        SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

    /// Update react state
    InitializeReactState();

    if (cInfo->flags_extra & CREATURE_FLAG_EXTRA_NO_TAUNT)
    {
        ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
        ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, true);
    }

    if (cInfo->flags_extra & CREATURE_FLAG_EXTRA_KNOCK_BACK_IMMUNE)
    {
        ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK_DEST, true);
        ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
    }

    UpdateMovementFlag();

    // TODO: Shouldn't we check whether or not the creature is in water first?
    if (cInfo->InhabitType & INHABIT_WATER && IsInWater())
    {
        AddUnitMovementFlag(MOVEMENTFLAG_SWIMMING);
        SetDisableGravity(true);
    }
    else
        RemoveUnitMovementFlag(MOVEMENTFLAG_SWIMMING);

    return true;
}

void Creature::Update(uint32 diff)
{
    //npcbot: update helper
    if (!m_canUpdate && bot_AI)
        return;
    //end npcbot

    if (IsAIEnabled && TriggerJustRespawned)
    {
        TriggerJustRespawned = false;
        AI()->JustRespawned();
        if (m_vehicleKit)
            m_vehicleKit->Reset();
    }

    /// Artamedes: Commented, fix it a different way. this is NOT OK
    /// Causes so many client crashes due to https://gyazo.com/9cdbeab9aa3ec9ffe459f93f3d37cd2a
    ///if (IsInWater() && !GetMap()->IsBattleArena())
    ///{
    ///    if (CanSwim())
    ///    {
    ///        AddUnitMovementFlag(MOVEMENTFLAG_SWIMMING);
    ///        SetDisableGravity(true);
    ///    }
    ///}
    ///else
    ///{
    ///    if (CanWalk())
    ///        RemoveUnitMovementFlag(MOVEMENTFLAG_SWIMMING);
    ///}

    UpdateMovementFlag();

    switch (m_deathState)
    {
        case JUST_RESPAWNED:
            // Must not be called, see Creature::setDeathState JUST_RESPAWNED -> ALIVE promoting.
            TC_LOG_ERROR("entities.unit", "Creature (GUID: %u Entry: %u) in wrong state: JUST_RESPAWNED (4)", GetGUIDLow(), GetEntry());
            break;
        case JUST_DIED:
            // Must not be called, see Creature::setDeathState JUST_DIED -> CORPSE promoting.
            TC_LOG_ERROR("entities.unit", "Creature (GUID: %u Entry: %u) in wrong state: JUST_DEAD (1)", GetGUIDLow(), GetEntry());
            break;
        case DEAD:
        {
            time_t now = time(NULL);
            if (m_respawnTime <= now)
            {
                bool allowed = IsAIEnabled ? AI()->CanRespawn() : true;     // First check if there are any scripts that object to us respawning
                if (!allowed)                                               // Will be rechecked on next Update call
                    break;

                uint64 dbtableHighGuid = MAKE_NEW_GUID(m_DBTableGuid, GetEntry(), HIGHGUID_UNIT);
                time_t linkedRespawntime = GetMap()->GetLinkedRespawnTime(dbtableHighGuid);
                if (!linkedRespawntime)             // Can respawn
                    Respawn();
                else                                // the master is dead
                {
                    uint64 targetGuid = sObjectMgr->GetLinkedRespawnGuid(dbtableHighGuid);
                    if (targetGuid == dbtableHighGuid) // if linking self, never respawn (check delayed to next day)
                        SetRespawnTime(DAY);
                    else
                        m_respawnTime = (now > linkedRespawntime ? now : linkedRespawntime)+urand(5, MINUTE); // else copy time from master and add a little
                    SaveRespawnTime(); // also save to DB immediately
                }
            }
            break;
        }
        case CORPSE:
        {
            Unit::Update(diff);
            // deathstate changed on spells update, prevent problems
            if (m_deathState != CORPSE)
                break;

            if (m_groupLootTimer && lootingGroupLowGUID)
            {
                if (m_groupLootTimer <= diff)
                {
                    Group* group = sGroupMgr->GetGroupByGUID(lootingGroupLowGUID);
                    if (group)
                        group->EndRoll(&loot);
                    m_groupLootTimer = 0;
                    lootingGroupLowGUID = 0;
                }
                else m_groupLootTimer -= diff;
            }
            else if (m_corpseRemoveTime <= time(NULL))
            {
                RemoveCorpse(false);
                TC_LOG_DEBUG("entities.unit", "Removing corpse... %u ", GetUInt32Value(OBJECT_FIELD_ENTRY));
            }
            break;
        }
        case ALIVE:
        {
            Unit::Update(diff);
            // creature can be dead after Unit::Update call
            // CORPSE/DEAD state will processed at next tick (in other case death timer will be updated unexpectedly)
            if (!isAlive())
                break;
            // if creature is charmed, switch to charmed AI
            if (NeedChangeAI && !DisableChangeAI)
            {
                UpdateCharmAI();
                NeedChangeAI = false;
                IsAIEnabled = true;
                if (!IsInEvadeMode() && LastCharmerGUID)
                    if (Unit* charmer = ObjectAccessor::GetUnit(*this, LastCharmerGUID))
                        i_AI->AttackStart(charmer);
            
                LastCharmerGUID = 0;
            }
            if (!IsInEvadeMode() && IsAIEnabled)
            {
                // do not allow the AI to be changed during update
                m_AI_locked = true;
                i_AI->UpdateAI(diff);
                m_AI_locked = false;
            }
            else if (DisableChangeAI)
            {
                // do not allow the AI to be changed during update
                m_AI_locked = true;
                i_AI->UpdateAI(diff);
                m_AI_locked = false;
            }
            // creature can be dead after UpdateAI call
            // CORPSE/DEAD state will processed at next tick (in other case death timer will be updated unexpectedly)
            if (!isAlive())
                break;
            if (m_regenTimer > 0)
            {
                if (diff >= m_regenTimer)
                    m_regenTimer = 0;
                else
                    m_regenTimer -= diff;
            }

            if (HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_REGENERATE_POWER))
            {
                if (m_regenTimerEnergy <= diff)
                {
                    if (getPowerType() == POWER_ENERGY)
                        if (!IsVehicle() || GetVehicleKit()->GetVehicleInfo()->m_powerType != POWER_PYRITE)
                            Regenerate(POWER_ENERGY);
                    m_regenTimerEnergy = CREATURE_REGEN_ENERGY_INTERVAL;
                }
                else
                    m_regenTimerEnergy -= diff;
            }
			if (m_regenTimer == 0)
			{
				bool bInCombat = isInCombat() && (!getVictim() ||                                        // if isInCombat() is true and this has no victim
					!getVictim()->GetCharmerOrOwnerPlayerOrPlayerItself() ||                // or the victim/owner/charmer is not a player
					!getVictim()->GetCharmerOrOwnerPlayerOrPlayerItself()->isGameMaster()); // or the victim/owner/charmer is not a GameMaster
																											
				if (!IsInEvadeMode() && (!bInCombat || IsPolymorphed())) // regenerate health if not in combat or if polymorphed
				{
					RegenerateHealth();
				}
				if (getPowerType() != POWER_ENERGY && HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_REGENERATE_POWER))
				{
					RegenerateMana();
				}
				m_regenTimer = CREATURE_REGEN_INTERVAL;
			}
			
			if (CanNotReachTarget() && !IsInEvadeMode() && !GetMap()->IsRaid())
			{
				m_cannotReachTimer += diff;
				if (m_cannotReachTimer >= CREATURE_NOPATH_EVADE_TIME)
				{
					if (IsAIEnabled)
					{
						if(!AI()->IsIgnoringEvade())
							AI()->EnterEvadeMode();
					}
				}
			}
            break;
            
        }
        default:
            break;
    }

	// Aps: Handle idle timer of summon
	if (GetCharmerOrOwner())
	{
		if (m_idleTimer > 0)
			m_idleTimer -= diff;
		
		if (m_re_idleTimer > 0)
			m_re_idleTimer -= diff;
		
		if (m_idleTimer <= 0 || m_idleTimer > SUMMON_IDLE_TIMER)
		{
			m_idleTimer = 0;
			SetIdle(true);
		}
		
		// Aps: Re-Idle timer should automatically start when owner moves
		if (GetCharmerOrOwner()->isMoving() || GetCharmerOrOwner()->isTurning())
		{
			StartReIdleTimer();
		}
		else if (m_re_idleTimer <= 0)
		{
			m_re_idleTimer = 60 * 60 * IN_MILLISECONDS; // 1 hour
			SetReIdle(true);
		}
	}

    if (!m_ScheduledMovementInforms.empty())
    {
        for (auto l_Info : m_ScheduledMovementInforms)
        {
            if (CreatureAI* l_AI = AI())
                l_AI->MovementInform(l_Info.first, l_Info.second);
        }

        m_ScheduledMovementInforms.clear();
    }

    sScriptMgr->OnCreatureUpdate(this, diff);
}

bool Creature::OnBladesEdgeRopes()
{
	if (GetMapId() != 562)
		return false;
	
	if (GetPositionZ() < 10.0f)
		return false;
	
	if (GetPositionX() <= 6235.111328f &&
		GetPositionX() >= 6230.426270f &&
		GetPositionY() <= 257.261841f  &&
		GetPositionY() >= 251.947128f)
		return true;
	
	if (GetPositionX() <= 6247.027344f &&
		GetPositionX() >= 6242.205566f &&
		GetPositionY() <= 272.098816f  &&
		GetPositionY() >= 266.665833f)
		return true;
	
	return false;
}

void Creature::ScheduleMovementInform(uint32 p_Type, uint32 p_Id)
{
    m_ScheduledMovementInforms.push_back(std::make_pair(p_Type, p_Id));
}

void Creature::RegenerateMana()
{
    if (CreatureTemplate const* cinfo = sObjectMgr->GetCreatureTemplate(GetEntry()))
        if (cinfo->flags_extra & CREATURE_FLAG_EXTRA_DISABLE_POWER_REG)
            return;

    uint32 curValue = GetPower(POWER_MANA);
    uint32 maxValue = GetMaxPower(POWER_MANA);

    if (!m_canRegen)
        return;

    if (curValue >= maxValue)
        return;

    uint32 addvalue = 0;

    // Combat and any controlled creature
    if (isInCombat() || GetCharmerOrOwnerGUID())
    {
        float ManaIncreaseRate = sWorld->getRate(RATE_POWER_MANA);
        float Spirit = GetStat(STAT_SPIRIT);

        addvalue = uint32((Spirit / 5.0f + 17.0f) * ManaIncreaseRate);
    }
    else
        addvalue = maxValue / 3;

    // Apply modifiers (if any).
    AuraEffectList const& ModPowerRegenPCTAuras = GetAuraEffectsByType(SPELL_AURA_MOD_POWER_REGEN_PERCENT);
    for (AuraEffectList::const_iterator i = ModPowerRegenPCTAuras.begin(); i != ModPowerRegenPCTAuras.end(); ++i)
        if ((*i)->GetMiscValue() == POWER_MANA)
            AddPct(addvalue, (*i)->GetAmount());

    addvalue += GetTotalAuraModifierByMiscValue(SPELL_AURA_MOD_POWER_REGEN, POWER_MANA) * CREATURE_REGEN_INTERVAL / (5 * IN_MILLISECONDS);

    ModifyPower(POWER_MANA, addvalue);
}

void Creature::RegenerateHealth()
{
    if (!isRegeneratingHealth())
        return;

    uint32 curValue = GetHealth();
    uint32 maxValue = GetMaxHealth();

    if (curValue >= maxValue)
        return;

    uint32 addvalue = 0;

    // Not only pet, but any controlled creature
    if (GetCharmerOrOwnerGUID())
    {
        float HealthIncreaseRate = sWorld->getRate(RATE_HEALTH);

        if (isGuardian())
            addvalue = ((Guardian*)this)->OCTRegenHPPerSpirit() * HealthIncreaseRate;
        if (!addvalue)
        {
            float Spirit = GetStat(STAT_SPIRIT);

            if (GetPower(POWER_MANA) > 0)
                addvalue = uint32(Spirit * 0.25 * HealthIncreaseRate);
            else
                addvalue = uint32(Spirit * 0.80 * HealthIncreaseRate);
        }
    }
    else
        addvalue = maxValue/3;

    // Apply modifiers (if any).
    AuraEffectList const& ModPowerRegenPCTAuras = GetAuraEffectsByType(SPELL_AURA_MOD_HEALTH_REGEN_PERCENT);
    for (AuraEffectList::const_iterator i = ModPowerRegenPCTAuras.begin(); i != ModPowerRegenPCTAuras.end(); ++i)
        AddPct(addvalue, (*i)->GetAmount());

    addvalue += GetTotalAuraModifier(SPELL_AURA_MOD_REGEN) * CREATURE_REGEN_INTERVAL  / (5 * IN_MILLISECONDS);

    ModifyHealth(addvalue);
}

void Creature::DoFleeToGetAssistance()
{
    if (!getVictim())
        return;

    if (HasAuraType(SPELL_AURA_PREVENTS_FLEEING))
        return;

    float radius = sWorld->getFloatConfig(CONFIG_CREATURE_FAMILY_FLEE_ASSISTANCE_RADIUS);
    if (radius >0)
    {
        Creature* creature = NULL;

        CellCoord p(monster::ComputeCellCoord(GetPositionX(), GetPositionY()));
        Cell cell(p);
        cell.SetNoCreate();
        monster::NearestAssistCreatureInCreatureRangeCheck u_check(this, getVictim(), radius);
        monster::CreatureLastSearcher<monster::NearestAssistCreatureInCreatureRangeCheck> searcher(this, creature, u_check);

        TypeContainerVisitor<monster::CreatureLastSearcher<monster::NearestAssistCreatureInCreatureRangeCheck>, GridTypeMapContainer > grid_creature_searcher(searcher);

        cell.Visit(p, grid_creature_searcher, *GetMap(), *this, radius);

        SetNoSearchAssistance(true);
        UpdateSpeed(MOVE_RUN, false);

        if (!creature)
            //SetFeared(true, getVictim()->GetGUID(), 0, sWorld->getIntConfig(CONFIG_CREATURE_FAMILY_FLEE_DELAY));
            //TODO: use 31365
            SetControlled(true, UNIT_STATE_FLEEING);
        else
            GetMotionMaster()->MoveSeekAssistance(creature->GetPositionX(), creature->GetPositionY(), creature->GetPositionZ());
    }
}

bool Creature::AIM_Initialize(CreatureAI* ai)
{
    // make sure nothing can change the AI during AI update
    if (m_AI_locked)
    {
        TC_LOG_DEBUG("scripts", "AIM_Initialize: failed to init, locked.");
        return false;
    }

    UnitAI* oldAI = i_AI;

    Motion_Initialize();

    i_AI = ai ? ai : FactorySelector::selectAI(this);
    delete oldAI;
    IsAIEnabled = true;
    i_AI->InitializeAI();
    // Initialize vehicle
    if (GetVehicleKit())
        GetVehicleKit()->Reset();
    return true;
}

void Creature::Motion_Initialize()
{
    if (!m_formation)
        i_motionMaster.Initialize();
    else if (m_formation->getLeader() == this)
    {
        m_formation->FormationReset(false);
        i_motionMaster.Initialize();
    }
    else if (m_formation->isFormed())
        i_motionMaster.MoveIdle(); //wait the order of leader
    else
        i_motionMaster.Initialize();
}

bool Creature::Create(uint32 guidlow, Map* map, uint32 phaseMask, uint32 Entry, uint32 vehId, uint32 team, float x, float y, float z, float ang, const CreatureData* data)
{
    ASSERT(map);
    SetMap(map);
    SetPhaseMask(phaseMask, false);

    CreatureTemplate const* cinfo = sObjectMgr->GetCreatureTemplate(Entry);
    if (!cinfo)
    {
        TC_LOG_ERROR("sql.sql", "Creature::Create(): creature template (guidlow: %u, entry: %u) does not exist.", guidlow, Entry);
        return false;
    }

    //! Relocate before CreateFromProto, to initialize coords and allow
    //! returning correct zone id for selecting OutdoorPvP/Battlefield script
    Relocate(x, y, z, ang);

    //oX = x;     oY = y;    dX = x;    dY = y;    m_moveTime = 0;    m_startMove = 0;
    if (!CreateFromProto(guidlow, Entry, vehId, team, data))
        return false;

    if (!IsPositionValid())
    {
        TC_LOG_ERROR("entities.unit", "Creature::Create(): given coordinates for creature (guidlow %d, entry %d) are not valid (X: %f, Y: %f, Z: %f, O: %f)", guidlow, Entry, x, y, z, ang);
        return false;
    }

    switch (GetCreatureTemplate()->rank)
    {
        case CREATURE_ELITE_RARE:
            m_corpseDelay = sWorld->getIntConfig(CONFIG_CORPSE_DECAY_RARE);
            break;
        case CREATURE_ELITE_ELITE:
            m_corpseDelay = sWorld->getIntConfig(CONFIG_CORPSE_DECAY_ELITE);
            break;
        case CREATURE_ELITE_RAREELITE:
            m_corpseDelay = sWorld->getIntConfig(CONFIG_CORPSE_DECAY_RAREELITE);
            break;
        case CREATURE_ELITE_WORLDBOSS:
            m_corpseDelay = sWorld->getIntConfig(CONFIG_CORPSE_DECAY_WORLDBOSS);
            break;
        default:
            m_corpseDelay = sWorld->getIntConfig(CONFIG_CORPSE_DECAY_NORMAL);
            break;
    }

    LoadCreaturesAddon();

    //! Need to be called after LoadCreaturesAddon - MOVEMENTFLAG_HOVER is set there
    if (HasUnitMovementFlag(MOVEMENTFLAG_HOVER))
    {
        z += GetFloatValue(UNIT_FIELD_HOVERHEIGHT);

        //! Relocate again with updated Z coord
        Relocate(x, y, z, ang);
    }

    uint32 displayID = GetNativeDisplayId();
    CreatureModelInfo const* minfo = sObjectMgr->GetCreatureModelRandomGender(&displayID);
    if (minfo && !isTotem())                               // Cancel load if no model defined or if totem
    {
        SetDisplayId(displayID);
        SetNativeDisplayId(displayID);
        SetByteValue(UNIT_FIELD_BYTES_0, 2, minfo->gender);
    }

    LastUsedScriptID = GetCreatureTemplate()->ScriptID;

    // TODO: Replace with spell, handle from DB
    if (isSpiritHealer() || isSpiritGuide())
    {
        m_serverSideVisibility.SetValue(SERVERSIDE_VISIBILITY_GHOST, GHOST_VISIBILITY_GHOST);
        m_serverSideVisibilityDetect.SetValue(SERVERSIDE_VISIBILITY_GHOST, GHOST_VISIBILITY_GHOST);
    }

    if (Entry == VISUAL_WAYPOINT)
        SetVisible(false);

	if (GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_IGNORE_PATHFINDING)
		AddUnitState(UNIT_STATE_IGNORE_PATHFINDING);

    return true;
}

void Creature::InitializeReactState()
{
    uint32 l_TemplateReactState = GetCreatureTemplate()->ReactState;

    if (isTotem() || isTrigger() || GetCreatureType() == CREATURE_TYPE_CRITTER || isSpiritService())
        SetReactState(REACT_PASSIVE);
    else if (l_TemplateReactState == -1)
        SetReactState(REACT_AGGRESSIVE);
    else
        SetReactState(static_cast<ReactStates>(l_TemplateReactState));

    /*else if (isCivilian())
    SetReactState(REACT_DEFENSIVE);*/;
}

bool Creature::isCanTrainingOf(Player* player, bool msg) const
{
    if (!isTrainer())
        return false;

    TrainerSpellData const* trainer_spells = GetTrainerSpells();

    if ((!trainer_spells || trainer_spells->spellList.empty()) && GetCreatureTemplate()->trainer_type != TRAINER_TYPE_PETS)
    {
        TC_LOG_ERROR("sql.sql", "Creature %u (Entry: %u) have UNIT_NPC_FLAG_TRAINER but have empty trainer spell list.",
            GetGUIDLow(), GetEntry());
        return false;
    }

    switch (GetCreatureTemplate()->trainer_type)
    {
        case TRAINER_TYPE_CLASS:
            if (player->getClass() != GetCreatureTemplate()->trainer_class)
            {
                if (msg)
                {
                    player->PlayerTalkClass->ClearMenus();
                    switch (GetCreatureTemplate()->trainer_class)
                    {
                        case CLASS_DRUID:  player->PlayerTalkClass->SendGossipMenu(4913, GetGUID()); break;
                        case CLASS_HUNTER: player->PlayerTalkClass->SendGossipMenu(10090, GetGUID()); break;
                        case CLASS_MAGE:   player->PlayerTalkClass->SendGossipMenu(328, GetGUID()); break;
                        case CLASS_PALADIN:player->PlayerTalkClass->SendGossipMenu(1635, GetGUID()); break;
                        case CLASS_PRIEST: player->PlayerTalkClass->SendGossipMenu(4436, GetGUID()); break;
                        case CLASS_ROGUE:  player->PlayerTalkClass->SendGossipMenu(4797, GetGUID()); break;
                        case CLASS_SHAMAN: player->PlayerTalkClass->SendGossipMenu(5003, GetGUID()); break;
                        case CLASS_WARLOCK:player->PlayerTalkClass->SendGossipMenu(5836, GetGUID()); break;
                        case CLASS_WARRIOR:player->PlayerTalkClass->SendGossipMenu(4985, GetGUID()); break;
                    }
                }
                return false;
            }
            break;
        case TRAINER_TYPE_PETS:
            if (player->getClass() != CLASS_HUNTER)
            {
                player->PlayerTalkClass->ClearMenus();
                player->PlayerTalkClass->SendGossipMenu(3620, GetGUID());
                return false;
            }
            break;
        case TRAINER_TYPE_MOUNTS:
            if (GetCreatureTemplate()->trainer_race && player->getRace() != GetCreatureTemplate()->trainer_race)
            {
                if (msg)
                {
                    player->PlayerTalkClass->ClearMenus();
                    switch (GetCreatureTemplate()->trainer_race)
                    {
                        case RACE_DWARF:        player->PlayerTalkClass->SendGossipMenu(5865, GetGUID()); break;
                        case RACE_GNOME:        player->PlayerTalkClass->SendGossipMenu(4881, GetGUID()); break;
                        case RACE_HUMAN:        player->PlayerTalkClass->SendGossipMenu(5861, GetGUID()); break;
                        case RACE_NIGHTELF:     player->PlayerTalkClass->SendGossipMenu(5862, GetGUID()); break;
                        case RACE_ORC:          player->PlayerTalkClass->SendGossipMenu(5863, GetGUID()); break;
                        case RACE_TAUREN:       player->PlayerTalkClass->SendGossipMenu(5864, GetGUID()); break;
                        case RACE_TROLL:        player->PlayerTalkClass->SendGossipMenu(5816, GetGUID()); break;
                        case RACE_UNDEAD_PLAYER:player->PlayerTalkClass->SendGossipMenu(624, GetGUID()); break;
                        case RACE_BLOODELF:     player->PlayerTalkClass->SendGossipMenu(5862, GetGUID()); break;
                        case RACE_DRAENEI:      player->PlayerTalkClass->SendGossipMenu(5864, GetGUID()); break;
                    }
                }
                return false;
            }
            break;
        case TRAINER_TYPE_TRADESKILLS:
            // There's no Blacksmith specialization on Cataclysm, conditions are not required for tradeskills
            break;
        default:
            return false;                                   // checked and error output at creature_template loading
    }
    return true;
}

bool Creature::isCanInteractWithBattleMaster(Player* player, bool msg) const
{
    if (!isBattleMaster())
        return false;

    BattlegroundTypeId bgTypeId = sBattlegroundMgr->GetBattleMasterBG(GetEntry());
    if (!msg)
        return player->GetBGAccessByLevel(bgTypeId);

    if (!player->GetBGAccessByLevel(bgTypeId))
    {
        player->PlayerTalkClass->ClearMenus();
        switch (bgTypeId)
        {
            case BATTLEGROUND_AV:  player->PlayerTalkClass->SendGossipMenu(7616, GetGUID()); break;
            case BATTLEGROUND_WS:  player->PlayerTalkClass->SendGossipMenu(7599, GetGUID()); break;
            case BATTLEGROUND_AB:  player->PlayerTalkClass->SendGossipMenu(7642, GetGUID()); break;
            case BATTLEGROUND_EY:
            case BATTLEGROUND_NA:
            case BATTLEGROUND_BE:
            case BATTLEGROUND_AA:
            case BATTLEGROUND_RL:
            case BATTLEGROUND_SA:
            case BATTLEGROUND_DS:
            case BATTLEGROUND_RV: player->PlayerTalkClass->SendGossipMenu(10024, GetGUID()); break;
            default: break;
        }
        return false;
    }
    return true;
}

bool Creature::isCanTrainingAndResetTalentsOf(Player* player) const
{
    return player->getLevel() >= 10
        && GetCreatureTemplate()->trainer_type == TRAINER_TYPE_CLASS
        && player->getClass() == GetCreatureTemplate()->trainer_class;
}

Player* Creature::GetLootRecipient() const
{
    if (!m_lootRecipient)
        return NULL;
    return ObjectAccessor::FindPlayer(m_lootRecipient);
}

Group* Creature::GetLootRecipientGroup() const
{
    if (!m_lootRecipientGroup)
        return NULL;
    return sGroupMgr->GetGroupByGUID(m_lootRecipientGroup);
}

void Creature::SetLootRecipient(Unit* unit)
{
    // set the player whose group should receive the right
    // to loot the creature after it dies
    // should be set to NULL after the loot disappears

    if (!unit)
    {
        m_lootRecipient = 0;
        m_lootRecipientGroup = 0;
        RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE|UNIT_DYNFLAG_TAPPED);
        return;
    }

    if (unit->GetTypeId() != TYPEID_PLAYER && !unit->IsVehicle())
        return;

    Player* player = unit->GetCharmerOrOwnerPlayerOrPlayerItself();
    if (!player)                                             // normal creature, no player involved
        return;

    m_lootRecipient = player->GetGUID();
    if (Group* group = player->GetGroup())
        m_lootRecipientGroup = group->GetLowGUID();

    SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_TAPPED);
}

// return true if this creature is tapped by the player or by a member of his group.
bool Creature::isTappedBy(Player const* player) const
{
    if (player->GetGUID() == m_lootRecipient)
        return true;

    Group const* playerGroup = player->GetGroup();
    if (!playerGroup || playerGroup != GetLootRecipientGroup()) // if we dont have a group we arent the recipient
        return false;                                           // if creature doesnt have group bound it means it was solo killed by someone else

    return true;
}

void Creature::SaveToDB()
{
    // this should only be used when the creature has already been loaded
    // preferably after adding to map, because mapid may not be valid otherwise
    CreatureData const* data = sObjectMgr->GetCreatureData(m_DBTableGuid);
    if (!data)
    {
        TC_LOG_ERROR("entities.unit", "Creature::SaveToDB failed, cannot get creature data!");
        return;
    }

    SaveToDB(GetMapId(), data->spawnMask, GetPhaseMask());
}

void Creature::SaveToDB(uint32 mapid, uint8 spawnMask, uint32 phaseMask)
{
    // update in loaded data
    if (!m_DBTableGuid)
        m_DBTableGuid = GetGUIDLow();
    CreatureData& data = sObjectMgr->NewOrExistCreatureData(m_DBTableGuid);

    uint32 displayId = GetNativeDisplayId();
    uint32 npcflag = GetUInt32Value(UNIT_NPC_FLAGS);
    uint32 unit_flags = GetUInt32Value(UNIT_FIELD_FLAGS);
    uint32 dynamicflags = GetUInt32Value(UNIT_DYNAMIC_FLAGS);

    // check if it's a custom model and if not, use 0 for displayId
    CreatureTemplate const* cinfo = GetCreatureTemplate();
    if (cinfo)
    {
        if (displayId == cinfo->Modelid1 || displayId == cinfo->Modelid2 ||
            displayId == cinfo->Modelid3 || displayId == cinfo->Modelid4)
            displayId = 0;

        if (npcflag == cinfo->npcflag)
            npcflag = 0;

        if (unit_flags == cinfo->unit_flags)
            unit_flags = 0;

        if (dynamicflags == cinfo->dynamicflags)
            dynamicflags = 0;
    }

    // data->guid = guid must not be updated at save
    data.id = GetEntry();
    data.mapid = mapid;
    data.phaseMask = phaseMask;
    data.displayid = displayId;
    data.equipmentId = GetEquipmentId();
    data.posX = GetPositionX();
    data.posY = GetPositionY();
    data.posZ = GetPositionZMinusOffset();
    data.orientation = GetOrientation();
    data.spawntimesecs = m_respawnDelay;
    // prevent add data integrity problems
    data.spawndist = GetDefaultMovementType() == IDLE_MOTION_TYPE ? 0.0f : m_respawnradius;
    data.currentwaypoint = 0;
    data.curhealth = GetHealth();
    data.curmana = GetPower(POWER_MANA);
    // prevent add data integrity problems
    data.movementType = !m_respawnradius && GetDefaultMovementType() == RANDOM_MOTION_TYPE
        ? IDLE_MOTION_TYPE : GetDefaultMovementType();
    data.spawnMask = spawnMask;
    data.npcflag = npcflag;
    data.unit_flags = unit_flags;
    data.dynamicflags = dynamicflags;
    data.walkmode = m_walkmode;
    data.saiscriptflag = m_saiscriptflag;

    // update in DB
    SQLTransaction trans = WorldDatabase.BeginTransaction();

    PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_DEL_CREATURE);
    stmt->setUInt32(0, m_DBTableGuid);
    trans->Append(stmt);

    uint8 index = 0;

    stmt = WorldDatabase.GetPreparedStatement(WORLD_INS_CREATURE);
    stmt->setUInt32(index++, m_DBTableGuid);
    stmt->setUInt32(index++, GetEntry());
    stmt->setUInt16(index++, uint16(mapid));

    uint32 zoneId, areaId;
    GetZoneAndAreaId(zoneId, areaId);

    stmt->setUInt16(index++, uint16(zoneId));
    stmt->setUInt16(index++, uint16(areaId));
    stmt->setUInt8(index++, spawnMask);
    stmt->setUInt16(index++, uint16(GetPhaseMask()));
    stmt->setUInt32(index++, displayId);
    stmt->setInt32(index++, int32(GetEquipmentId()));
    stmt->setFloat(index++, GetPositionX());
    stmt->setFloat(index++, GetPositionY());
    stmt->setFloat(index++, GetPositionZ());
    stmt->setFloat(index++, GetOrientation());
    stmt->setUInt32(index++, m_respawnDelay);
    stmt->setFloat(index++, m_respawnradius);
    stmt->setUInt32(index++, 0);
    stmt->setUInt32(index++, GetHealth());
    stmt->setUInt32(index++, GetPower(POWER_MANA));
    stmt->setUInt8(index++, uint8(GetDefaultMovementType()));
    stmt->setUInt32(index++, npcflag);
    stmt->setUInt32(index++, unit_flags);
    stmt->setUInt32(index++, dynamicflags);
    stmt->setFloat(index++, m_walkmode);
    stmt->setFloat(index++, m_saiscriptflag);
    trans->Append(stmt);

    WorldDatabase.CommitTransaction(trans);
}

void Creature::SelectLevel(const CreatureTemplate* cinfo)
{
    uint32 rank = isPet()? 0 : cinfo->rank;

    // level
    uint8 minlevel = std::min(cinfo->maxlevel, cinfo->minlevel);
    uint8 maxlevel = std::max(cinfo->maxlevel, cinfo->minlevel);
    uint8 level = minlevel == maxlevel ? minlevel : urand(minlevel, maxlevel);
    SetLevel(level);

    CreatureBaseStats const* stats = sObjectMgr->GetCreatureBaseStats(level, cinfo->unit_class);

    // health
    float healthmod = _GetHealthMod(rank);

    uint32 basehp = stats->GenerateHealth(cinfo);
    uint32 health = uint32(basehp * healthmod);

    SetCreateHealth(health);
    SetMaxHealth(health);
    SetHealth(health);
    ResetPlayerDamageReq();

    // mana
    uint32 mana = stats->GenerateMana(cinfo);
    SetCreateMana(mana);

    switch (getClass())
    {
        case CLASS_WARRIOR:
            setPowerType(POWER_RAGE);
            break;
        case CLASS_ROGUE:
            setPowerType(POWER_ENERGY);
            break;
        default:
            SetMaxPower(POWER_MANA, mana);
            SetPower(POWER_MANA, mana);
            break;
    }

    SetModifierValue(UNIT_MOD_HEALTH, BASE_VALUE, (float)health);
    SetModifierValue(UNIT_MOD_MANA, BASE_VALUE, (float)mana);

    //damage
    //float damagemod = _GetDamageMod(rank);      // Set during loading templates into dmg_multiplier field

    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, cinfo->mindmg);
    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, cinfo->maxdmg);

    SetBaseWeaponDamage(OFF_ATTACK, MINDAMAGE, cinfo->mindmg);
    SetBaseWeaponDamage(OFF_ATTACK, MAXDAMAGE, cinfo->maxdmg);

    SetBaseWeaponDamage(RANGED_ATTACK, MINDAMAGE, cinfo->minrangedmg);
    SetBaseWeaponDamage(RANGED_ATTACK, MAXDAMAGE, cinfo->maxrangedmg);

    SetModifierValue(UNIT_MOD_ATTACK_POWER, BASE_VALUE, cinfo->attackpower);
}

float Creature::_GetHealthMod(int32 Rank)
{
    switch (Rank)                                           // define rates for each elite rank
    {
        case CREATURE_ELITE_NORMAL:
            return sWorld->getRate(RATE_CREATURE_NORMAL_HP);
        case CREATURE_ELITE_ELITE:
            return sWorld->getRate(RATE_CREATURE_ELITE_ELITE_HP);
        case CREATURE_ELITE_RAREELITE:
            return sWorld->getRate(RATE_CREATURE_ELITE_RAREELITE_HP);
        case CREATURE_ELITE_WORLDBOSS:
            return sWorld->getRate(RATE_CREATURE_ELITE_WORLDBOSS_HP);
        case CREATURE_ELITE_RARE:
            return sWorld->getRate(RATE_CREATURE_ELITE_RARE_HP);
        default:
            return sWorld->getRate(RATE_CREATURE_ELITE_ELITE_HP);
    }
}

bool Creature::IsDamageEnoughForLootingAndReward() const
{
    if (sObjectMgr->DoesCreatureHaveSparringInfo(GetEntry()) && m_SparringIsCanceled)
        return true;

    return m_PlayerDamageReq == 0;
}

float Creature::_GetDamageMod(int32 Rank)
{
    switch (Rank)                                           // define rates for each elite rank
    {
        case CREATURE_ELITE_NORMAL:
            return sWorld->getRate(RATE_CREATURE_NORMAL_DAMAGE);
        case CREATURE_ELITE_ELITE:
            return sWorld->getRate(RATE_CREATURE_ELITE_ELITE_DAMAGE);
        case CREATURE_ELITE_RAREELITE:
            return sWorld->getRate(RATE_CREATURE_ELITE_RAREELITE_DAMAGE);
        case CREATURE_ELITE_WORLDBOSS:
            return sWorld->getRate(RATE_CREATURE_ELITE_WORLDBOSS_DAMAGE);
        case CREATURE_ELITE_RARE:
            return sWorld->getRate(RATE_CREATURE_ELITE_RARE_DAMAGE);
        default:
            return sWorld->getRate(RATE_CREATURE_ELITE_ELITE_DAMAGE);
    }
}

float Creature::GetSpellDamageMod(int32 Rank)
{
    switch (Rank)                                           // define rates for each elite rank
    {
        case CREATURE_ELITE_NORMAL:
            return sWorld->getRate(RATE_CREATURE_NORMAL_SPELLDAMAGE);
        case CREATURE_ELITE_ELITE:
            return sWorld->getRate(RATE_CREATURE_ELITE_ELITE_SPELLDAMAGE);
        case CREATURE_ELITE_RAREELITE:
            return sWorld->getRate(RATE_CREATURE_ELITE_RAREELITE_SPELLDAMAGE);
        case CREATURE_ELITE_WORLDBOSS:
            return sWorld->getRate(RATE_CREATURE_ELITE_WORLDBOSS_SPELLDAMAGE);
        case CREATURE_ELITE_RARE:
            return sWorld->getRate(RATE_CREATURE_ELITE_RARE_SPELLDAMAGE);
        default:
            return sWorld->getRate(RATE_CREATURE_ELITE_ELITE_SPELLDAMAGE);
    }
}

bool Creature::CreateFromProto(uint32 guidlow, uint32 Entry, uint32 vehId, uint32 team, const CreatureData* data)
{
    SetZoneScript();
    if (m_zoneScript && data)
    {
        Entry = m_zoneScript->GetCreatureEntry(guidlow, data);
        if (!Entry)
            return false;
    }

    CreatureTemplate const* cinfo = sObjectMgr->GetCreatureTemplate(Entry);
    if (!cinfo)
    {
        TC_LOG_ERROR("sql.sql", "Creature::CreateFromProto(): creature template (guidlow: %u, entry: %u) does not exist.", guidlow, Entry);
        return false;
    }

    SetOriginalEntry(Entry);

    if (!vehId)
        vehId = cinfo->VehicleId;

    Object::_Create(guidlow, Entry, vehId ? HIGHGUID_VEHICLE : HIGHGUID_UNIT);

    if (!UpdateEntry(Entry, team, data))
        return false;

    if (vehId)
        CreateVehicleKit(vehId, Entry);

    return true;
}

bool Creature::LoadCreatureFromDB(uint32 guid, Map* map, bool addToMap)
{
    CreatureData const* data = sObjectMgr->GetCreatureData(guid);

    if (!data)
    {
        TC_LOG_ERROR("sql.sql", "Creature (GUID: %u) not found in table `creature`, can't load. ", guid);
        return false;
    }

    m_DBTableGuid = guid;
    if (map->GetInstanceId() == 0)
    {
        if (map->GetCreature(MAKE_NEW_GUID(guid, data->id, HIGHGUID_UNIT)))
            return false;
    }
    else
        guid = sObjectMgr->GenerateLowGuid(HIGHGUID_UNIT);

    uint16 team = 0;
    if (!Create(guid, map, data->phaseMask, data->id, 0, team, data->posX, data->posY, data->posZ, data->orientation, data))
        return false;

    //We should set first home position, because then AI calls home movement
    SetHomePosition(data->posX, data->posY, data->posZ, data->orientation);

    m_respawnradius = data->spawndist;

    m_saiscriptflag = data->saiscriptflag;
    m_respawnDelay = data->spawntimesecs;
    m_walkmode = data->walkmode;
    m_deathState = ALIVE;

    m_respawnTime  = GetMap()->GetCreatureRespawnTime(m_DBTableGuid);
    if (m_respawnTime)                          // respawn on Update
    {
        m_deathState = DEAD;
        if (CanFly())
        {
            float tz = std::max<float>(map->GetHeight(GetPhaseMask(), data->posX, data->posY, data->posZ, false), map->GetHeight(GetPhaseMask(), data->posX, data->posY, data->posZ, false, 5.0f));
            if (data->posZ - tz > 0.1f)
                Relocate(data->posX, data->posY, tz);
        }
    }

    uint32 curhealth;

    if (!m_regenHealth)
    {
        curhealth = data->curhealth;
        if (curhealth)
        {
            curhealth = uint32(curhealth*_GetHealthMod(GetCreatureTemplate()->rank));
            if (curhealth < 1)
                curhealth = 1;
        }
        SetPower(POWER_MANA, data->curmana);
    }
    else
    {
        curhealth = GetMaxHealth();
        SetPower(POWER_MANA, GetMaxPower(POWER_MANA));
    }

    SetHealth(m_deathState == ALIVE ? curhealth : 0);

    // checked at creature_template loading
    m_defaultMovementType = MovementGeneratorType(data->movementType);

    m_creatureData = data;

    if (addToMap && !GetMap()->AddToMap(this))
        return false;
    return true;
}

void Creature::SetCanDualWield(bool value)
{
    Unit::SetCanDualWield(value);
    UpdateDamagePhysical(OFF_ATTACK);
}

void Creature::LoadEquipment(uint32 equip_entry, bool force)
{
    //npcbot: prevent loading equipment for bots
    if (GetEntry() >= BOT_ENTRY_BEGIN && GetEntry() <= BOT_ENTRY_END)
        return;
    //end npcbot

    if (equip_entry == 0)
    {
        if (force)
        {
            for (uint8 i = 0; i < 3; ++i)
                SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + i, 0);
            m_equipmentId = 0;
        }
        return;
    }

    EquipmentInfo const* einfo = sObjectMgr->GetEquipmentInfo(equip_entry);
    if (!einfo)
        return;

    m_equipmentId = equip_entry;
    for (uint8 i = 0; i < 3; ++i)
        SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + i, einfo->ItemEntry[i]);
}

bool Creature::hasQuest(uint32 quest_id) const
{
    QuestRelationBounds qr = sObjectMgr->GetCreatureQuestRelationBounds(GetEntry());
    for (QuestRelations::const_iterator itr = qr.first; itr != qr.second; ++itr)
    {
        if (itr->second == quest_id)
            return true;
    }
    return false;
}

bool Creature::hasInvolvedQuest(uint32 quest_id) const
{
    QuestRelationBounds qir = sObjectMgr->GetCreatureQuestInvolvedRelationBounds(GetEntry());
    for (QuestRelations::const_iterator itr = qir.first; itr != qir.second; ++itr)
    {
        if (itr->second == quest_id)
            return true;
    }
    return false;
}

void Creature::DeleteFromDB()
{
    if (!m_DBTableGuid)
    {
        TC_LOG_ERROR("entities.unit", "Trying to delete not saved creature! LowGUID: %u, Entry: %u", GetGUIDLow(), GetEntry());
        return;
    }

    GetMap()->RemoveCreatureRespawnTime(m_DBTableGuid);
    sObjectMgr->DeleteCreatureData(m_DBTableGuid);

    SQLTransaction trans = WorldDatabase.BeginTransaction();

    PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_DEL_CREATURE);
    stmt->setUInt32(0, m_DBTableGuid);
    trans->Append(stmt);

    stmt = WorldDatabase.GetPreparedStatement(WORLD_DEL_CREATURE_ADDON);
    stmt->setUInt32(0, m_DBTableGuid);
    trans->Append(stmt);

    stmt = WorldDatabase.GetPreparedStatement(WORLD_DEL_GAME_EVENT_CREATURE);
    stmt->setUInt32(0, m_DBTableGuid);
    trans->Append(stmt);

    stmt = WorldDatabase.GetPreparedStatement(WORLD_DEL_GAME_EVENT_MODEL_EQUIP);
    stmt->setUInt32(0, m_DBTableGuid);
    trans->Append(stmt);

    WorldDatabase.CommitTransaction(trans);
}

bool Creature::IsInvisibleDueToDespawn() const
{
    if (Unit::IsInvisibleDueToDespawn())
        return true;

    if (isAlive() || m_corpseRemoveTime > time(NULL))
        return false;

    return true;
}

bool Creature::CanAlwaysSee(WorldObject const* obj) const
{
    if (IsAIEnabled && AI()->CanSeeAlways(obj))
        return true;

    return false;
}

bool Creature::canStartAttack(Unit const* who, bool force) const
{
    if (Creature const* l_Creature = who->ToCreature())
        if (GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_NO_PARRY && l_Creature->GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_NO_PARRY)
            return true;

    if (isCivilian())
        return false;

    if ((HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC) && who->GetTypeId() != TYPEID_PLAYER) || (HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC) && who->GetTypeId() == TYPEID_PLAYER))
        return false;

    if ((HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE) && GetMapId() == 940))
        return false;

    // Do not attack non-combat pets
    if (who->GetTypeId() == TYPEID_UNIT && who->GetCreatureType() == CREATURE_TYPE_NON_COMBAT_PET)
        return false;

    if (!CanFly() && (GetDistanceZ(who) > CREATURE_Z_ATTACK_RANGE + m_CombatDistance))
        //|| who->IsControlledByPlayer() && who->IsFlying()))
        // we cannot check flying for other creatures, too much map/vmap calculation
        // TODO: should switch to range attack
        return false;

    if (!force)
    {
        if (!_IsTargetAcceptable(who))
            return false;

        if (who->isInCombat() && IsWithinDist(who, ATTACK_DISTANCE))
            if (Unit* victim = who->getAttackerForHelper())
                if (IsWithinDistInMap(victim, sWorld->getFloatConfig(CONFIG_CREATURE_FAMILY_ASSISTANCE_RADIUS)))
                    force = true;

        if (!force && (IsNeutralToAll() || !IsWithinDistInMap(who, GetAttackDistance(who) + m_CombatDistance)))
            return false;
    }

    if (!canCreatureAttack(who, force))
        return false;

    return IsWithinLOSInMap(who);
}

float Creature::GetAttackDistance(Unit const* player) const
{
   // WoW Wiki: the minimum radius seems to be 5 yards, while the maximum range is 45 yards
   float maxRadius = (45.0f * sWorld->getRate(RATE_CREATURE_AGGRO));
   float minRadius = (5.0f * sWorld->getRate(RATE_CREATURE_AGGRO));
   float aggroRate = sWorld->getRate(RATE_CREATURE_AGGRO);
   uint8 expansionMaxLevel = GetMaxLevelForExpansion(GetCreatureTemplate()->expansion);
   int32 levelDifference = getLevel() - player->getLevel();

   if (aggroRate == 0.0f)
       return 0.0;

   // The aggro radius for creatures with equal level as the player is 20 yards.
   // The combatreach should not get taken into account for the distance so we drop it from the range (see Supremus as expample)
   float baseAggroDistance = 20.0f - GetFloatValue(UNIT_FIELD_COMBATREACH);

   // + - 1 yard for each level difference between player and creature
   float aggroRadius = baseAggroDistance + float(levelDifference);

   // detect range auras
   if (float(getLevel() + 5) <= sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
   {
       aggroRadius += GetTotalAuraModifier(SPELL_AURA_MOD_DETECT_RANGE);

       aggroRadius += GetTotalAuraModifier(SPELL_AURA_MOD_DETECTED_RANGE);
   }

   // The aggro range of creatures with higher levels than the total player level for the expansion should get the maxlevel treatment
   // This makes sure that creatures such as bosses wont have a bigger aggro range than the rest of the npc's
   // The following code is used for blizzlike behaivior such as skipable bosses (e.g. Commander Springvale at level 85)
   if (getLevel() > expansionMaxLevel)
       aggroRadius = baseAggroDistance + float(expansionMaxLevel - player->getLevel());

   // Make sure that we wont go over the total range limits
   if (aggroRadius > maxRadius)
       aggroRadius = maxRadius;
   else if (aggroRadius < minRadius)
       aggroRadius = minRadius;

   return (aggroRadius * aggroRate);
}

void Creature::setDeathState(DeathState s)
{
    Unit::setDeathState(s);

    if (s == JUST_DIED)
    {
		/* Dynamic Respawn Rates */
		int32 currentPlayers = -1;
		uint32 respawnDelay = m_respawnDelay;
		if (GetMap())
		{
			auto& players = GetMap()->GetPlayers();
			for (auto& playerItr : players)
			{
				if (playerItr.getSource()->GetAreaId() == GetAreaId())
					++currentPlayers;
			}
		}

		if (currentPlayers != -1)
		{
			//formula for reduction here
			const uint32 minPlayersForReduction = 5; // minimum players until reduction starts taking place
			const uint32 maxReductionPercentage = 80; // can't get reduced below 20% of the actual spawntime
			const float  percentagePerPlayer = 33.333f; // percentage of spawntime that's reduced per extra player after minPlayersForReduction is reached

			float percentageReduction = 0.0f;
			if (currentPlayers > minPlayersForReduction)
				percentageReduction = (currentPlayers - minPlayersForReduction) * percentagePerPlayer;

			if (percentageReduction > maxReductionPercentage)
				percentageReduction = maxReductionPercentage;

			respawnDelay *= (1 - (percentageReduction / 100));
		}

		m_corpseRemoveTime = time(NULL) + m_corpseDelay;
		m_respawnTime = time(NULL) + respawnDelay + m_corpseDelay;

        // always save boss respawn time at death to prevent crash cheating
        if (sWorld->getBoolConfig(CONFIG_SAVE_RESPAWN_TIME_IMMEDIATELY) || isWorldBoss())
            SaveRespawnTime();

        SetTarget(0);                // remove target selection in any cases (can be set at aura remove in Unit::setDeathState)
        SetUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_NONE);

        setActive(false);

        if (!isPet() && GetCreatureTemplate()->SkinLootId)
            if (LootTemplates_Skinning.HaveLootFor(GetCreatureTemplate()->SkinLootId))
                if (hasLootRecipient())
                    SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);

        if (HasSearchedAssistance())
        {
            SetNoSearchAssistance(false);
            UpdateSpeed(MOVE_RUN, false);
        }

        //Dismiss group if is leader
        if (m_formation && m_formation->getLeader() == this)
            m_formation->FormationReset(true);

        if ((CanFly() || IsFlying()))
            i_motionMaster.MoveFall();

        Unit::setDeathState(CORPSE);
    }
    else if (s == JUST_RESPAWNED)
    {
        //if (isPet())
        //    setActive(true);
        SetFullHealth();
        SetLootRecipient(NULL);
        ResetPlayerDamageReq();
        CreatureTemplate const* cinfo = GetCreatureTemplate();
        SetWalk(true);

        UpdateMovementFlag();

        if (cinfo->InhabitType & INHABIT_WATER && IsInWater())
            AddUnitMovementFlag(MOVEMENTFLAG_SWIMMING);
        else
            RemoveUnitMovementFlag(MOVEMENTFLAG_SWIMMING);

        SetUInt32Value(UNIT_NPC_FLAGS, cinfo->npcflag);
        ClearUnitState(uint32(UNIT_STATE_ALL_STATE));
        SetMeleeDamageSchool(SpellSchools(cinfo->dmgschool));
        //! Need to be called after LoadCreaturesAddon - MOVEMENTFLAG_HOVER is set there
        if (HasUnitMovementFlag(MOVEMENTFLAG_HOVER))
            Relocate(GetPositionX(), GetPositionY(), GetPositionZ() + GetFloatValue(UNIT_FIELD_HOVERHEIGHT), GetOrientation());

        Motion_Initialize();
        if (GetCreatureData() && GetPhaseMask() != GetCreatureData()->phaseMask)
            SetPhaseMask(GetCreatureData()->phaseMask, false);
        Unit::setDeathState(ALIVE);
        LoadCreaturesAddon(true);
    }
}

void Creature::Respawn(bool force)
{
    DestroyForNearbyPlayers();

    if (force)
    {
        if (isAlive())
            setDeathState(JUST_DIED);
        else if (getDeathState() != CORPSE)
            setDeathState(CORPSE);
    }

    RemoveCorpse(false);

    if (getDeathState() == DEAD)
    {
        if (m_DBTableGuid)
            GetMap()->RemoveCreatureRespawnTime(m_DBTableGuid);

        TC_LOG_DEBUG("entities.unit", "Respawning creature %s (GuidLow: %u, Full GUID: " UI64FMTD " Entry: %u)",
            GetName().c_str(), GetGUIDLow(), GetGUID(), GetEntry());
        m_respawnTime = 0;
        lootForPickPocketed = false;
        lootForBody         = false;

        if (m_originalEntry != GetEntry())
            UpdateEntry(m_originalEntry);

        CreatureTemplate const* cinfo = GetCreatureTemplate();
        SelectLevel(cinfo);

        m_SparringIsCanceled = !sObjectMgr->DoesCreatureHaveSparringInfo(cinfo->Entry);

        if (m_DBTableGuid)
            SetVisible(true);

        setDeathState(JUST_RESPAWNED);

        uint32 displayID = GetNativeDisplayId();
        CreatureModelInfo const* minfo = sObjectMgr->GetCreatureModelRandomGender(&displayID);
        if (minfo)                                             // Cancel load if no model defined
        {
            SetDisplayId(displayID);
            SetNativeDisplayId(displayID);
            SetByteValue(UNIT_FIELD_BYTES_0, 2, minfo->gender);
        }

        GetMotionMaster()->InitDefault();

        //Call AI respawn virtual function
        if (IsAIEnabled)
            TriggerJustRespawned = true;//delay event to next tick so all creatures are created on the map before processing

        uint32 poolid = GetDBTableGUIDLow() ? sPoolMgr->IsPartOfAPool<Creature>(GetDBTableGUIDLow()) : 0;
        if (poolid)
            sPoolMgr->UpdatePool<Creature>(poolid, GetDBTableGUIDLow());

        //Re-initialize reactstate that could be altered by movementgenerators
        InitializeReactState();
    }

    UpdateObjectVisibility();
}

void Creature::ForcedDespawn(uint32 timeMSToDespawn)
{
    if (timeMSToDespawn)
    {
        ForcedDespawnDelayEvent* pEvent = new ForcedDespawnDelayEvent(*this);

        m_Events.AddEvent(pEvent, m_Events.CalculateTime(timeMSToDespawn));
        return;
    }


    // Prevents permanent creature spawns from showing the "dead" sound despawning. 
    if (m_DBTableGuid)
        SetVisible(false);

    if (isAlive())
        setDeathState(JUST_DIED);

    RemoveCorpse(false);
}

void Creature::DespawnOrUnsummon(uint32 msTimeToDespawn /*= 0*/)
{
    if (TempSummon* summon = this->ToTempSummon())
        summon->UnSummon(msTimeToDespawn);
    else
        ForcedDespawn(msTimeToDespawn);
}

void Creature::DespawnCreaturesInArea(uint32 entry, float range)
{
    std::list<Creature*> creatures;
    GetCreatureListWithEntryInGrid(creatures, entry, range);

    if (creatures.empty())
        return;

    for (std::list<Creature*>::iterator iter = creatures.begin(); iter != creatures.end(); ++iter)
        ( *iter )->DespawnOrUnsummon();
}

bool Creature::IsImmunedToSpell(SpellInfo const* spellInfo, Unit* caster, const uint8 effectMask /*= MAX_EFFECT_MASK*/)
{
    if (!spellInfo)
        return false;

    switch (spellInfo->Id)
    {
        case 105256:   // Hagara - Frozen Tempest
        case 109552:
        case 109553:
        case 109554:
            return false;
        default:
            break;
    }

    // Creature is immune to main mechanic of the spell
    if (GetCreatureTemplate()->MechanicImmuneMask & (1 << (spellInfo->Mechanic - 1)))
        return true;

    return Unit::IsImmunedToSpell(spellInfo, caster, effectMask);
}

bool Creature::IsImmunedToSpellEffect(SpellInfo const* spellInfo, uint32 index) const
{
    if (GetCreatureTemplate()->MechanicImmuneMask & (1 << (spellInfo->Effects[index].Mechanic - 1)))
        return true;

    if (GetCreatureTemplate()->type == CREATURE_TYPE_MECHANICAL && spellInfo->Effects[index].Effect == SPELL_EFFECT_HEAL)
        return true;

    return Unit::IsImmunedToSpellEffect(spellInfo, index);
}

SpellInfo const* Creature::reachWithSpellAttack(Unit* victim)
{
    if (!victim)
        return NULL;

    for (uint32 i=0; i < CREATURE_MAX_SPELLS; ++i)
    {
        if (!m_spells[i])
            continue;
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(m_spells[i]);
        if (!spellInfo)
        {
            TC_LOG_ERROR("entities.unit", "WORLD: unknown spell id %i", m_spells[i]);
            continue;
        }

        bool bcontinue = true;
        for (uint32 j = 0; j < MAX_SPELL_EFFECTS; j++)
        {
            if ((spellInfo->Effects[j].Effect == SPELL_EFFECT_SCHOOL_DAMAGE)       ||
                (spellInfo->Effects[j].Effect == SPELL_EFFECT_INSTAKILL)            ||
                (spellInfo->Effects[j].Effect == SPELL_EFFECT_ENVIRONMENTAL_DAMAGE) ||
                (spellInfo->Effects[j].Effect == SPELL_EFFECT_HEALTH_LEECH)
                )
            {
                bcontinue = false;
                break;
            }
        }
        if (bcontinue)
            continue;

        if (spellInfo->ManaCost > (uint32)GetPower(POWER_MANA))
            continue;
        float range = spellInfo->GetMaxRange(false);
        float minrange = spellInfo->GetMinRange(false);
        float dist = GetDistance(victim);
        if (dist > range || dist < minrange)
            continue;
        if (spellInfo->PreventionType == SPELL_PREVENTION_TYPE_SILENCE && HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED))
            continue;
        if (spellInfo->PreventionType == SPELL_PREVENTION_TYPE_PACIFY && HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED))
            continue;
        return spellInfo;
    }
    return NULL;
}

SpellInfo const* Creature::reachWithSpellCure(Unit* victim)
{
    if (!victim)
        return NULL;

    for (uint32 i=0; i < CREATURE_MAX_SPELLS; ++i)
    {
        if (!m_spells[i])
            continue;
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(m_spells[i]);
        if (!spellInfo)
        {
            TC_LOG_ERROR("entities.unit", "WORLD: unknown spell id %i", m_spells[i]);
            continue;
        }

        bool bcontinue = true;
        for (uint32 j = 0; j < MAX_SPELL_EFFECTS; j++)
        {
            if ((spellInfo->Effects[j].Effect == SPELL_EFFECT_HEAL))
            {
                bcontinue = false;
                break;
            }
        }
        if (bcontinue)
            continue;

        if (spellInfo->ManaCost > (uint32)GetPower(POWER_MANA))
            continue;

        float range = spellInfo->GetMaxRange(true);
        float minrange = spellInfo->GetMinRange(true);
        float dist = GetDistance(victim);
        //if (!isInFront(victim, range) && spellInfo->AttributesEx)
        //    continue;
        if (dist > range || dist < minrange)
            continue;
        if (spellInfo->PreventionType == SPELL_PREVENTION_TYPE_SILENCE && HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED))
            continue;
        if (spellInfo->PreventionType == SPELL_PREVENTION_TYPE_PACIFY && HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED))
            continue;
        return spellInfo;
    }
    return NULL;
}

void Creature::SendAIReaction(AiReaction reactionType)
{
    WorldPacket data(SMSG_AI_REACTION, 12);

    data << uint64(GetGUID());
    data << uint32(reactionType);

    ((WorldObject*)this)->SendMessageToSet(&data, true);

    TC_LOG_DEBUG("network.opcode", "WORLD: Sent SMSG_AI_REACTION, type %u.", reactionType);
}

void Creature::CallAssistance()
{
    if (!m_AlreadyCallAssistance && getVictim() && !isPet() && !isCharmed())
    {
        SetNoCallAssistance(true);

        float radius = sWorld->getFloatConfig(CONFIG_CREATURE_FAMILY_ASSISTANCE_RADIUS);

        if (radius > 0)
        {
            std::list<Creature*> assistList;

            {
                CellCoord p(monster::ComputeCellCoord(GetPositionX(), GetPositionY()));
                Cell cell(p);
                cell.SetNoCreate();

                monster::AnyAssistCreatureInRangeCheck u_check(this, getVictim(), radius);
                monster::CreatureListSearcher<monster::AnyAssistCreatureInRangeCheck> searcher(this, assistList, u_check);

                TypeContainerVisitor<monster::CreatureListSearcher<monster::AnyAssistCreatureInRangeCheck>, GridTypeMapContainer >  grid_creature_searcher(searcher);

                cell.Visit(p, grid_creature_searcher, *GetMap(), *this, radius);
            }

            if (!assistList.empty())
            {
                AssistDelayEvent* e = new AssistDelayEvent(getVictim()->GetGUID(), *this);
                while (!assistList.empty())
                {
                    // Pushing guids because in delay can happen some creature gets despawned => invalid pointer
                    e->AddAssistant((*assistList.begin())->GetGUID());
                    assistList.pop_front();
                }
                m_Events.AddEvent(e, m_Events.CalculateTime(sWorld->getIntConfig(CONFIG_CREATURE_FAMILY_ASSISTANCE_DELAY)));
            }
        }
    }
}

void Creature::CallForHelp(float radius)
{
    if (radius <= 0.0f || !getVictim() || isPet() || isCharmed())
        return;

    CellCoord p(monster::ComputeCellCoord(GetPositionX(), GetPositionY()));
    Cell cell(p);
    cell.SetNoCreate();

    monster::CallOfHelpCreatureInRangeDo u_do(this, getVictim(), radius);
    monster::CreatureWorker<monster::CallOfHelpCreatureInRangeDo> worker(this, u_do);

    TypeContainerVisitor<monster::CreatureWorker<monster::CallOfHelpCreatureInRangeDo>, GridTypeMapContainer >  grid_creature_searcher(worker);

    cell.Visit(p, grid_creature_searcher, *GetMap(), *this, radius);
}

bool Creature::CanAssistTo(const Unit* u, const Unit* enemy, bool checkfaction /*= true*/) const
{
    // Check if enemy exists
    if (!enemy)
        return false;
    
    if (IsInEvadeMode())
        return false;

    // or if enemy is in evade mode
    if (enemy->GetTypeId() == TYPEID_UNIT && enemy->ToCreature()->IsInEvadeMode())
        return false;

    // is it true?
    if (!HasReactState(REACT_AGGRESSIVE))
        return false;

    // we don't need help from zombies :)
    if (!isAlive())
        return false;

    // we don't need help from non-combatant ;)
    if (isCivilian())
        return false;

    if (HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE))
        if ((enemy->GetTypeId() == TYPEID_PLAYER && HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC)) || (enemy->GetTypeId() != TYPEID_PLAYER && HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC)))
            return false;

    // skip fighting creature
    if (isInCombat())
        return false;

    // only free creature
    if (GetCharmerOrOwnerGUID())
        return false;

    // only from same creature faction
    if (checkfaction)
    {
        if (getFaction() != u->getFaction())
            return false;
    }
    else
    {
        if (!IsFriendlyTo(u))
            return false;
    }

    // skip non hostile to caster enemy creatures
    if (!IsHostileTo(enemy))
        return false;

    return true;
}

// use this function to avoid having hostile creatures attack
// friendlies and other mobs they shouldn't attack
bool Creature::_IsTargetAcceptable(const Unit* target, const bool aggro /*= false*/) const
{
    ASSERT(target);

    if (Creature const* l_Creature = target->ToCreature())
        if (GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_NO_PARRY && l_Creature->GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_NO_PARRY)
            return true;
    // if the target cannot be attacked, the target is not acceptable
    if (IsFriendlyTo(target)
        || !target->isTargetableForAttack(false)
        || (m_vehicle && (IsOnVehicle(target) || m_vehicle->GetBase()->IsOnVehicle(target))))
        return false;

    if (target->HasUnitState(UNIT_STATE_DIED))
    {
        // guards can detect fake death
        if (isGuard() && target->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH))
            return true;
        else
            return false;
    }

    const Unit* myVictim = getAttackerForHelper();
    const Unit* targetVictim = target->getAttackerForHelper();

    // if I'm already fighting target, or I'm hostile towards the target, the target is acceptable
    if (myVictim == target || targetVictim == this || (aggro ? !IsFriendlyTo(target) : IsHostileTo(target)))
        return true;

    // if the target's victim is friendly, and the target is neutral, the target is acceptable
    if (targetVictim && IsFriendlyTo(targetVictim))
        return true;

    // if the target's victim is not friendly, or the target is friendly, the target is not acceptable
    return false;
}

void Creature::SaveRespawnTime()
{
    if (isSummon() || !m_DBTableGuid || (m_creatureData && !m_creatureData->dbData))
        return;

    GetMap()->SaveCreatureRespawnTime(m_DBTableGuid, m_respawnTime);
}

// this should not be called by petAI or
bool Creature::canCreatureAttack(Unit const* victim, bool /*force*/) const
{
    if (Creature const* l_Creature = victim->ToCreature())
        if (GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_NO_PARRY && l_Creature->GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_NO_PARRY)
            return true;

    if (!victim->IsInMap(this))
        return false;

    if (!IsValidAttackTarget(victim))
        return false;

    // we cannot attack in evade mode
    if (IsInEvadeMode())
        return false;

    if (Player const* playerVictim = victim->ToPlayer())
        if (playerVictim->isGameMaster())
            return false;

    if (victim->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH))
        return false;

    if (!victim->isInAccessiblePlaceFor(this))
        return false;

    if (IsAIEnabled && !AI()->CanAIAttack(victim))
        return false;

    if (sMapStore.LookupEntry(GetMapId())->IsDungeon())
        return true;

    // Map visibility range, but no more than 2*cell size
    float dist = std::min<float>(GetMap()->GetVisibilityRange(), SIZE_OF_GRID_CELL * 2);

    if (Unit* unit = GetCharmerOrOwner())
        return victim->IsWithinDist(unit, dist);
    else
        return victim->IsInDist(&m_homePosition, dist);
}

CreatureAddon const* Creature::GetCreatureAddon() const
{
    if (m_DBTableGuid)
    {
        if (CreatureAddon const* addon = sObjectMgr->GetCreatureAddon(m_DBTableGuid))
            return addon;
    }

    // dependent from difficulty mode entry
    return sObjectMgr->GetCreatureTemplateAddon(GetCreatureTemplate()->Entry);
}

//creature_addon table
bool Creature::LoadCreaturesAddon(bool reload)
{
    CreatureAddon const* cainfo = GetCreatureAddon();
    if (!cainfo)
        return false;

    if (cainfo->mount != 0)
        Mount(cainfo->mount);

    if (cainfo->bytes1 != 0)
    {
        // 0 StandState
        // 1 FreeTalentPoints   Pet only, so always 0 for default creature
        // 2 StandFlags
        // 3 StandMiscFlags

        SetByteValue(UNIT_FIELD_ANIM_TIER, 0, uint8(cainfo->bytes1 & 0xFF));
        //SetByteValue(UNIT_FIELD_ANIM_TIER, 1, uint8((cainfo->bytes1 >> 8) & 0xFF));
        SetByteValue(UNIT_FIELD_ANIM_TIER, 1, 0);
        SetByteValue(UNIT_FIELD_ANIM_TIER, 2, uint8((cainfo->bytes1 >> 16) & 0xFF));
        SetByteValue(UNIT_FIELD_ANIM_TIER, 3, uint8((cainfo->bytes1 >> 24) & 0xFF));

        //! Suspected correlation between UNIT_FIELD_ANIM_TIER, offset 3, value 0x2:
        //! If no inhabittype_fly (if no MovementFlag_DisableGravity flag found in sniffs)
        //! Set MovementFlag_Hover. Otherwise do nothing.
        if (GetByteValue(UNIT_FIELD_ANIM_TIER, 3) & UNIT_BYTE1_FLAG_HOVER && !IsLevitating())
            AddUnitMovementFlag(MOVEMENTFLAG_HOVER);
    }

    if (cainfo->bytes2 != 0)
    {
        // 0 SheathState
        // 1 Bytes2Flags
        // 2 UnitRename         Pet only, so always 0 for default creature
        // 3 ShapeshiftForm     Must be determined/set by shapeshift spell/aura

        SetByteValue(UNIT_FIELD_BYTES_2, 0, uint8(cainfo->bytes2 & 0xFF));
        //SetByteValue(UNIT_FIELD_BYTES_2, 1, uint8((cainfo->bytes2 >> 8) & 0xFF));
        //SetByteValue(UNIT_FIELD_BYTES_2, 2, uint8((cainfo->bytes2 >> 16) & 0xFF));
        SetByteValue(UNIT_FIELD_BYTES_2, 2, 0);
        //SetByteValue(UNIT_FIELD_BYTES_2, 3, uint8((cainfo->bytes2 >> 24) & 0xFF));
        SetByteValue(UNIT_FIELD_BYTES_2, 3, 0);
    }

    if (cainfo->emote != 0)
        SetUInt32Value(UNIT_NPC_EMOTESTATE, cainfo->emote);

    //Load Path
    if (cainfo->path_id != 0)
        m_path_id = cainfo->path_id;

    if (!cainfo->auras.empty())
    {
        for (std::vector<uint32>::const_iterator itr = cainfo->auras.begin(); itr != cainfo->auras.end(); ++itr)
        {
            SpellInfo const* AdditionalSpellInfo = sSpellMgr->GetSpellInfo(*itr);
            if (!AdditionalSpellInfo)
            {
                TC_LOG_ERROR("sql.sql", "Creature (GUID: %u Entry: %u) has wrong spell %u defined in `auras` field.", GetGUIDLow(), GetEntry(), *itr);
                continue;
            }

            // skip already applied aura
            if (HasAura(*itr))
            {
                if (!reload)
                    TC_LOG_ERROR("sql.sql", "Creature (GUID: %u Entry: %u) has duplicate aura (spell %u) in `auras` field.", GetDBTableGUIDLow(), GetEntry(), *itr);

                continue;
            }

            AddAura(*itr, this);
            TC_LOG_DEBUG("entities.unit", "Spell: %u added to creature (GUID: %u Entry: %u)", *itr, GetGUIDLow(), GetEntry());
        }
    }

    return true;
}

/// Send a message to LocalDefense channel for players opposition team in the zone
void Creature::SendZoneUnderAttackMessage(Player* attacker)
{
    uint32 enemy_team = attacker->GetTeam();

    WorldPacket data(SMSG_ZONE_UNDER_ATTACK, 4);
    data << (uint32)GetAreaId();
    sWorld->SendGlobalMessage(&data, NULL, (enemy_team == ALLIANCE ? HORDE : ALLIANCE));
}

void Creature::SetInCombatWithZone()
{
    if (!CanHaveThreatList())
    {
        TC_LOG_ERROR("entities.unit", "Creature entry %u call SetInCombatWithZone but creature cannot have threat list.", GetEntry());
        return;
    }

    Map* map = GetMap();

    if (!map->IsDungeon())
    {
        TC_LOG_ERROR("entities.unit", "Creature entry %u call SetInCombatWithZone for map (id: %u) that isn't an instance.", GetEntry(), map->GetId());
        return;
    }

    Map::PlayerList const &PlList = map->GetPlayers();

    if (PlList.isEmpty())
        return;

    for (Map::PlayerList::const_iterator i = PlList.begin(); i != PlList.end(); ++i)
    {
        if (Player* player = i->getSource())
        {
            if (player->isGameMaster())
                continue;

            if (player->isAlive())
            {
                this->SetInCombatWith(player);
                player->SetInCombatWith(this);
                AddThreat(player, 0.0f);
            }
        }
    }
}

void Creature::ProhibitSpellSchool(SpellSchoolMask idSchoolMask, uint32 unTimeMs)
{
    time_t curTime = time(NULL);
    for (uint8 i = 0; i < CREATURE_MAX_SPELLS; ++i)
    {
        if (m_spells[i] == 0)
            continue;

        uint32 unSpellId = m_spells[i];
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(unSpellId);
        if (!spellInfo)
        {
            ASSERT(spellInfo);
            continue;
        }

        // Not send cooldown for this spells
        if (spellInfo->Attributes & SPELL_ATTR0_DISABLED_WHILE_ACTIVE)
            continue;

        if (spellInfo->PreventionType != SPELL_PREVENTION_TYPE_SILENCE)
            continue;

        if ((idSchoolMask & spellInfo->GetSchoolMask()) && GetSpellCooldownDelay(unSpellId) < unTimeMs)
        {
            AddSpellCooldown(unSpellId, 0, curTime + unTimeMs/IN_MILLISECONDS);
            if (UnitAI* ai = GetAI())
                ai->SpellInterrupted(unSpellId, unTimeMs);
        }
    }
}

bool Creature::HasSpell(uint32 spellID) const
{
    uint8 i;
    for (i = 0; i < CREATURE_MAX_SPELLS; ++i)
        if (spellID == m_spells[i])
            break;
    return i < CREATURE_MAX_SPELLS;                         //broke before end of iteration of known spells
}

time_t Creature::GetRespawnTimeEx() const
{
    time_t now = time(NULL);
    if (m_respawnTime > now)
        return m_respawnTime;
    else
        return now;
}

void Creature::GetRespawnPosition(float &x, float &y, float &z, float* ori, float* dist) const
{
    if (m_DBTableGuid)
    {
        if (CreatureData const* data = sObjectMgr->GetCreatureData(GetDBTableGUIDLow()))
        {
            x = data->posX;
            y = data->posY;
            z = data->posZ;
            if (ori)
                *ori = data->orientation;
            if (dist)
                *dist = data->spawndist;

            return;
        }
    }

    x = GetPositionX();
    y = GetPositionY();
    z = GetPositionZ();
    if (ori)
        *ori = GetOrientation();
    if (dist)
        *dist = 0;
}

void Creature::AllLootRemovedFromCorpse()
{
    if (!HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE))
    {
        time_t now = time(NULL);
        if (m_corpseRemoveTime <= now)
            return;

        float decayRate;
        CreatureTemplate const* cinfo = GetCreatureTemplate();

        decayRate = sWorld->getRate(RATE_CORPSE_DECAY_LOOTED);
        uint32 diff = uint32((m_corpseRemoveTime - now) * decayRate);

        m_respawnTime -= diff;

        // corpse skinnable, but without skinning flag, and then skinned, corpse will despawn next update
        if (cinfo && cinfo->SkinLootId)
            m_corpseRemoveTime = time(NULL);
        else
            m_corpseRemoveTime -= diff;
    }
}

uint8 Creature::getLevelForTarget(WorldObject const* target) const
{
    if (!isWorldBoss() || !target->ToUnit())
        return Unit::getLevelForTarget(target);

    uint16 level = target->ToUnit()->getLevel() + sWorld->getIntConfig(CONFIG_WORLD_BOSS_LEVEL_DIFF);
    if (level < 1)
        return 1;
    if (level > 255)
        return 255;
    return uint8(level);
}

std::string Creature::GetAIName() const
{
    return sObjectMgr->GetCreatureTemplate(GetEntry())->AIName;
}

std::string Creature::GetScriptName() const
{
    return sObjectMgr->GetScriptName(GetScriptId());
}

uint32 Creature::GetScriptId() const
{
    return sObjectMgr->GetCreatureTemplate(GetEntry())->ScriptID;
}

VendorItemData const* Creature::GetVendorItems() const
{
    return sObjectMgr->GetNpcVendorItemList(GetEntry());
}

uint32 Creature::GetVendorItemCurrentCount(VendorItem const* vItem)
{
    if (!vItem->maxcount)
        return vItem->maxcount;

    VendorItemCounts::iterator itr = m_vendorItemCounts.begin();
    for (; itr != m_vendorItemCounts.end(); ++itr)
        if (itr->itemId == vItem->item)
            break;

    if (itr == m_vendorItemCounts.end())
        return vItem->maxcount;

    VendorItemCount* vCount = &*itr;

    time_t ptime = time(NULL);

    if (time_t(vCount->lastIncrementTime + vItem->incrtime) <= ptime)
    {
        ItemTemplate const* pProto = sObjectMgr->GetItemTemplate(vItem->item);

        uint32 diff = uint32((ptime - vCount->lastIncrementTime)/vItem->incrtime);
        if ((vCount->count + diff * pProto->BuyCount) >= vItem->maxcount)
        {
            m_vendorItemCounts.erase(itr);
            return vItem->maxcount;
        }

        vCount->count += diff * pProto->BuyCount;
        vCount->lastIncrementTime = ptime;
    }

    return vCount->count;
}

uint32 Creature::UpdateVendorItemCurrentCount(VendorItem const* vItem, uint32 used_count)
{
    if (!vItem->maxcount)
        return 0;

    VendorItemCounts::iterator itr = m_vendorItemCounts.begin();
    for (; itr != m_vendorItemCounts.end(); ++itr)
        if (itr->itemId == vItem->item)
            break;

    if (itr == m_vendorItemCounts.end())
    {
        uint32 new_count = vItem->maxcount > used_count ? vItem->maxcount-used_count : 0;
        m_vendorItemCounts.push_back(VendorItemCount(vItem->item, new_count));
        return new_count;
    }

    VendorItemCount* vCount = &*itr;

    time_t ptime = time(NULL);

    if (time_t(vCount->lastIncrementTime + vItem->incrtime) <= ptime)
    {
        ItemTemplate const* pProto = sObjectMgr->GetItemTemplate(vItem->item);

        uint32 diff = uint32((ptime - vCount->lastIncrementTime)/vItem->incrtime);
        if ((vCount->count + diff * pProto->BuyCount) < vItem->maxcount)
            vCount->count += diff * pProto->BuyCount;
        else
            vCount->count = vItem->maxcount;
    }

    vCount->count = vCount->count > used_count ? vCount->count-used_count : 0;
    vCount->lastIncrementTime = ptime;
    return vCount->count;
}

TrainerSpellData const* Creature::GetTrainerSpells() const
{
    return sObjectMgr->GetNpcTrainerSpells(GetEntry());
}

// overwrite WorldObject function for proper name localization
std::string const & Creature::GetNameForLocaleIdx(LocaleConstant loc_idx) const
{
    if (loc_idx != DEFAULT_LOCALE)
    {
        uint8 uloc_idx = uint8(loc_idx);
        CreatureLocale const* cl = sObjectMgr->GetCreatureLocale(GetEntry());
        if (cl)
        {
            if (cl->Name.size() > uloc_idx && !cl->Name[uloc_idx].empty())
                return cl->Name[uloc_idx];
        }
    }

    return GetName();
}

//Do not if this works or not, moving creature to another map is very dangerous
void Creature::FarTeleportTo(Map* map, float X, float Y, float Z, float O)
{
    CleanupBeforeRemoveFromMap(false);
    GetMap()->RemoveFromMap(this, false);
    Relocate(X, Y, Z, O);
    SetMap(map);
    GetMap()->AddToMap(this);
}

void Creature::SetPosition(float x, float y, float z, float o)
{
    // prevent crash when a bad coord is sent by the client
    if (!monster::IsValidMapCoord(x, y, z, o))
    {
        TC_LOG_DEBUG("entities.unit", "Creature::SetPosition(%f, %f, %f) .. bad coordinates!", x, y, z);
        return;
    }

    GetMap()->CreatureRelocation(ToCreature(), x, y, z, o);
    if (IsVehicle())
        GetVehicleKit()->RelocatePassengers();
}

bool Creature::IsDungeonBoss() const
{
    CreatureTemplate const* cinfo = sObjectMgr->GetCreatureTemplate(GetEntry());
    return cinfo && (cinfo->flags_extra & CREATURE_FLAG_EXTRA_DUNGEON_BOSS);
}

bool Creature::SetWalk(bool enable)
{
    if (!Unit::SetWalk(enable))
        return false;

    if (!movespline->Initialized())
        return true;

    return true;
}

bool Creature::SetDisableGravity(bool disable, bool packetOnly/*=false*/)
{
    //! It's possible only a packet is sent but moveflags are not updated
    //! Need more research on this
    if (!packetOnly && !Unit::SetDisableGravity(disable))
        return false;

    if (!movespline->Initialized())
        return true;

    SendMovementDisableGravity();
    return true;
}

bool Creature::SetHover(bool enable)
{
    if (!Unit::SetHover(enable))
        return false;

    //! Unconfirmed for players:
    if (enable)
        SetByteFlag(UNIT_FIELD_ANIM_TIER, 3, UNIT_BYTE1_FLAG_HOVER);
    else
        RemoveByteFlag(UNIT_FIELD_ANIM_TIER, 3, UNIT_BYTE1_FLAG_HOVER);

    if (!movespline->Initialized())
        return true;

    return true;
}

float Creature::GetAggroRange(Unit const* target) const
{
    // Determines the aggro range for creatures (usually pets), used mainly for aggressive pet target selection.
    // Based on data from wowwiki due to lack of 3.3.5a data

    if (target && this->isPet())
    {
        uint32 targetLevel = 0;

        if (target->GetTypeId() == TYPEID_PLAYER)
            targetLevel = target->getLevelForTarget(this);
        else if (target->GetTypeId() == TYPEID_UNIT)
            targetLevel = target->ToCreature()->getLevelForTarget(this);

        uint32 myLevel = getLevelForTarget(target);
        int32 levelDiff = int32(targetLevel) - int32(myLevel);

        // The maximum Aggro Radius is capped at 45 yards (25 level difference)
        if (levelDiff < -25)
            levelDiff = -25;

        // The base aggro radius for mob of same level
        float aggroRadius = 20;

        // Aggro Radius varies with level difference at a rate of roughly 1 yard/level
        aggroRadius -= (float)levelDiff;

        // detect range auras
        aggroRadius += GetTotalAuraModifier(SPELL_AURA_MOD_DETECT_RANGE);

        // detected range auras
        aggroRadius += target->GetTotalAuraModifier(SPELL_AURA_MOD_DETECTED_RANGE);

        // Just in case, we don't want pets running all over the map
        if (aggroRadius > MAX_AGGRO_RADIUS)
            aggroRadius = MAX_AGGRO_RADIUS;

        // Minimum Aggro Radius for a mob seems to be combat range (5 yards)
        //  hunter pets seem to ignore minimum aggro radius so we'll default it a little higher
        if (aggroRadius < 10)
            aggroRadius = 10;

        return (aggroRadius);
    }

    // Default
    return 0.0f;
}

void Creature::UpdateMovementFlags(bool force)
{
    if (HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED) || isPet() || GetTransport())
        return;

    // It should be rather rare for creatures to transition between ground, water and air with random movement,
    // and since most of the creatures in the world will be using it - we can reduce strain on performance by skipping it.
    if (GetMotionMaster()->GetCurrentMovementGeneratorType() == RANDOM_MOTION_TYPE && !force)
        return;

    // Don't change flags if the creature is affected by effect motion: it is probably not in control of its movement.
    if (GetMotionMaster()->GetCurrentMovementGeneratorType() == EFFECT_MOTION_TYPE && !force)
        return;

    /*// Don't update flags if position hasn't changed
    if (m_lastMovementFlagsUpdatePosition.m_positionX == m_positionX &&
    m_lastMovementFlagsUpdatePosition.m_positionY == m_positionY &&
    m_lastMovementFlagsUpdatePosition.m_positionZ == m_positionZ && !force)
    return;

    m_lastMovementFlagsUpdatePosition.Relocate(this);*/

    // It should be rather rare for creatures to transition between ground, water and air with random movement,
    // and since most of the creatures in the world will be using it - we can reduce strain on performance by skipping it.
    if (GetMotionMaster()->GetCurrentMovementGeneratorType() == RANDOM_MOTION_TYPE)
        return;

    // Don't change flags if the creature is affected by effect motion: it is probably not in control of its movement.
    if (GetMotionMaster()->GetCurrentMovementGeneratorType() == EFFECT_MOTION_TYPE)
        return;

    if (m_IgnoreUpdateMovementFlag)
        return;

    // Set the movement flags if the creature is in that mode. (Only fly if actually in air, only swim if in water, etc)
    float ground = GetMap()->GetHeight(GetPositionX(), GetPositionY(), GetPositionZMinusOffset());

    bool isInAir = (G3D::fuzzyGt(GetPositionZMinusOffset(), ground + 0.05f) || G3D::fuzzyLt(GetPositionZMinusOffset(), ground - 0.05f)); // Can be underground too, prevent the falling

    if (CanFly() && (!IsFalling() || force))
    {
        SetDisableGravity(isInAir || !CanWalk());
        SetCanFly(true);
    }
    else
    {
        SetDisableGravity(false);
        SetCanFly(false);
    }

    if (!isInAir)
        SetFall(false);

    SetSwim(CanSwim() && IsInWater());
}

CreatureSparring const* Creature::GetSparringData(uint32 attackerEntry, uint32 victimEntry) const
{
    return sObjectMgr->GetCreatureSparringInfo(attackerEntry, victimEntry);
}

bool Creature::CanSparWith(Creature* victim) const
{
    /// Passed as nullptr sometimes intentionally
    if (victim == nullptr)
        return false;

    /// Artamedes: Sparring - If we are engaged with a non sparring npc like player or pet, don't spar anymore :)
    if (m_SparringIsCanceled)
        return false;

    if (GetSparringData(GetEntry(), victim->GetEntry()))
        return true;

    return false;
}

float Creature::GetSparringHealthLimitPctFor(Creature* victim) const
{
    if (CreatureSparring const* sparrData = GetSparringData(GetEntry(), victim->GetEntry()))
        return sparrData->GetHealthLimitPct();

    return 0.0f;
}


void Creature::UpdateMovementFlag()
{
// Do not update movement flags if creature is controlled by a player (charm/vehicle)
    if (m_movedPlayer)
        return;

    // Set the movement flags if the creature is in that mode. (Only fly if actually in air, only swim if in water, etc)
    float ground = GetMap()->GetHeight(GetPhaseMask(), GetPositionX(), GetPositionY(), GetPositionZMinusOffset());

    bool isZPosIgnoredMap = GetMapId() == 669; // More maps to come?
    bool isInAir = (G3D::fuzzyGt(GetPositionZMinusOffset(), ground + 0.05f) || G3D::fuzzyLt(GetPositionZMinusOffset(), ground - 0.05f) || isZPosIgnoredMap); // Can be underground too, prevent the falling
    bool allowedToFly = (GetUnitMovementFlags() & MOVEMENTFLAG_CAN_FLY || GetByteValue(UNIT_FIELD_ANIM_TIER, 3) & UNIT_BYTE1_FLAG_HOVER || IsInEvadeMode() || GetMotionMaster()->GetCurrentMovementGeneratorType() == HOME_MOTION_TYPE  || GetMapId() == 669 || getVictim() && getVictim()->IsFlying()) || isInAir;
    bool allowedToWalk = GetUnitMovementFlags() & MOVEMENTFLAG_WALKING && !(GetByteValue(UNIT_FIELD_ANIM_TIER, 3) & UNIT_BYTE1_FLAG_HOVER) && !isInAir;

    if (GetInhabitType() & INHABIT_AIR && isInAir && !IsFalling() && allowedToFly)
    {
        if (GetCreatureTemplate()->InhabitType & INHABIT_GROUND && !allowedToWalk)
        {
            SetCanFly(true);
            SetDisableGravity(true);
        }
        else
            SetDisableGravity(true);
    }
    else
    {
        SetCanFly(false);
        SetDisableGravity(false);
    }

    if (!isInAir)
        SetFall(false);

    SetSwim(GetInhabitType() & INHABIT_WATER && IsInWater());
}

bool Creature::IsInWater() const
{
    if (GetBaseSwapMap() != NULL)
        return GetBaseSwapMap()->IsInWater(GetPositionX(), GetPositionY(), GetPositionZ());
    return GetBaseMap()->IsInWater(GetPositionX(), GetPositionY(), GetPositionZ());
}

bool Creature::IsUnderWater() const
{
    if (GetBaseSwapMap() != NULL)
        return GetBaseSwapMap()->IsInWater(GetPositionX(), GetPositionY(), GetPositionZ());
    return GetBaseMap()->IsUnderWater(GetPositionX(), GetPositionY(), GetPositionZ());
}

void Creature::SetObjectScale(float scale)
{
    Unit::SetObjectScale(scale);

    if (CreatureModelInfo const* minfo = sObjectMgr->GetCreatureModelInfo(GetDisplayId()))
    {
        SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, minfo->bounding_radius * scale);
        SetFloatValue(UNIT_FIELD_COMBATREACH, minfo->combat_reach * scale);
    }
}

void Creature::SetDisplayId(uint32 modelId)
{
    Unit::SetDisplayId(modelId);

    if (CreatureModelInfo const* minfo = sObjectMgr->GetCreatureModelInfo(modelId))
    {
        SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, minfo->bounding_radius * GetFloatValue(OBJECT_FIELD_SCALE_X));
        SetFloatValue(UNIT_FIELD_COMBATREACH, minfo->combat_reach * GetFloatValue(OBJECT_FIELD_SCALE_X));
    }
}

void Creature::TalkWithDelay(uint32 Delay, int32 Id, uint64 WhisperGuid, ChatMsg MsgType)
{
    class TalkDelayEvent : public BasicEvent
    {
    public:
        TalkDelayEvent(Creature* _crea, int32 _id, uint64 _whisperGUID, uint32 const& _delay, ChatMsg _msgType) :
            crea(_crea), whisperGuid(_whisperGUID), id(_id), delay(_delay), msgType(_msgType)
        {
        }

        bool Execute(uint64 /*execTime*/, uint32 /*diff*/)
        {
            if (id < 0)
            {
                sCreatureTextMgr->SendBroadcast(crea, id *= -1, whisperGuid, msgType, TEXT_RANGE_NORMAL);
            }
            else
                sCreatureTextMgr->SendChat(crea, id, whisperGuid);
            return true;
        }

    private:
        Creature* crea;
        uint64 whisperGuid;
        int32 id;
        uint32 delay;
        ChatMsg msgType;
    };

    m_Events.AddEvent(new TalkDelayEvent(this, Id, WhisperGuid, Delay, MsgType), m_Events.CalculateTime(Delay));
}

void Creature::SetTarget(uint32 guid)
{
	if (IsFocusing(NULL, true))
		m_suppressedTarget = guid;
	else
		SetGuidValue(UNIT_FIELD_TARGET, guid);
}

void Creature::FocusTarget(Spell const* focusSpell, WorldObject* target)
{
	// already focused
	if (m_focusSpell)
		return;

	// don't use spell focus for vehicle spells
	if (focusSpell->GetSpellInfo()->HasAura(SPELL_AURA_CONTROL_VEHICLE))
		return;

	if ((!target || target == this) && !focusSpell->GetCastTime()) // instant cast, untargeted (or self-targeted) spell doesn't need any facing updates
		return;

	// store pre-cast values for target and orientation (used to later restore)
	if (!IsFocusing(NULL, true))
	{ // only overwrite these fields if we aren't transitioning from one spell focus to another
		m_suppressedTarget = GetGuidValue(UNIT_FIELD_TARGET);
		m_suppressedOrientation = GetOrientation();
	}

	m_focusSpell = focusSpell;

	// set target, then force send update packet to players if it changed to provide appropriate facing
	uint32 newTarget = target ? target->GetGUID() : 0;
	if (GetGuidValue(UNIT_FIELD_TARGET) != newTarget)
	{
		SetGuidValue(UNIT_FIELD_TARGET, newTarget);

		if ( // here we determine if the (relatively expensive) forced update is worth it, or whether we can afford to wait until the scheduled update tick
			( // only require instant update for spells that actually have a visual
				focusSpell->GetSpellInfo()->SpellVisual[0] ||
				focusSpell->GetSpellInfo()->SpellVisual[1]
				) && (
					!focusSpell->GetCastTime() || // if the spell is instant cast
					focusSpell->GetSpellInfo()->HasAttribute(SPELL_ATTR5_DONT_TURN_DURING_CAST) // client gets confused if we attempt to turn at the regularly scheduled update packet
					)
			)
		{
			std::list<Player*> playersNearby;
			GetPlayerListInGrid(playersNearby, GetVisibilityRange());
			for (Player* player : playersNearby)
			{
				// only update players that are known to the client (have already been created)
				if (player->HaveAtClient(this))
					SendUpdateToPlayer(player);
			}
		}
	}

	bool canTurnDuringCast = !focusSpell->GetSpellInfo()->HasAttribute(SPELL_ATTR5_DONT_TURN_DURING_CAST);
	// Face the target - we need to do this before the unit state is modified for no-turn spells
	if (target)
		SetFacingToObject(target);
	else if (!canTurnDuringCast)
		if (Unit* victim = getVictim())
			SetFacingToObject(victim); // ensure orientation is correct at beginning of cast

	if (!canTurnDuringCast)
		AddUnitState(UNIT_STATE_CANNOT_TURN);
}

bool Creature::IsFocusing(Spell const* focusSpell, bool withDelay)
{
	if (!isAlive()) // dead creatures cannot focus
	{
		ReleaseFocus(NULL, false);
		return false;
	}

	if (focusSpell && (focusSpell != m_focusSpell))
		return false;

	if (!m_focusSpell)
	{
		if (!withDelay || !m_focusDelay)
			return false;
		if (GetMSTimeDiffToNow(m_focusDelay) > 1000) // @todo figure out if we can get rid of this magic number somehow
		{
			m_focusDelay = 0; // save checks in the future
			return false;
		}
	}

	return true;
}

void Creature::ReleaseFocus(Spell const* focusSpell, bool withDelay)
{
	if (!m_focusSpell)
		return;

	// focused to something else
	if (focusSpell && focusSpell != m_focusSpell)
		return;

	if (isPet()) // player pets do not use delay system
	{
		SetGuidValue(UNIT_FIELD_TARGET, m_suppressedTarget);
		if (m_suppressedTarget)
		{
			if (WorldObject* objTarget = ObjectAccessor::GetWorldObject(*this, m_suppressedTarget))
				SetFacingToObject(objTarget);
		}
		else
			SetFacingTo(m_suppressedOrientation);
	}
	else
		// tell the creature that it should reacquire its actual target after the delay expires (this is handled in ::Update)
		// player pets don't need to do this, as they automatically reacquire their target on focus release
		MustReacquireTarget();

	if (m_focusSpell->GetSpellInfo()->HasAttribute(SPELL_ATTR5_DONT_TURN_DURING_CAST))
		ClearUnitState(UNIT_STATE_CANNOT_TURN);

	m_focusSpell = NULL;
	m_focusDelay = (!isPet() && withDelay) ? getMSTime() : 0; // don't allow re-target right away to prevent visual bugs
}

void Creature::Regenerate(Powers power)
{
    uint32 curValue = GetPower(power);
    uint32 maxValue = GetMaxPower(power);

    if (curValue >= maxValue)
        return;

    float addvalue = 0.0f;

    switch (power)
    {
        case POWER_FOCUS:
        {
            // For hunter pets.
            addvalue = 24 * sWorld->getRate(RATE_POWER_FOCUS);
            break;
        }
        case POWER_ENERGY:
        {
            if (GetEntry() == 26125) // Dk Ghouls
            {
                if (Unit* owner = GetOwner())
                {
                    if (owner->GetTypeId() == TYPEID_PLAYER)
                    {
                        float haste = (1.0f / owner->ToPlayer()->GetFloatValue(PLAYER_FIELD_MOD_HASTE_REGEN) - 1.0f) * 100.0f;
                        addvalue += 20.0f + CalculatePct(20.0f, haste);
                    }
                    else
                        addvalue += 20;
                }
            }
            else
                addvalue += 20;
            break;
        }
       default:
           return;
    }

    // Apply modifiers (if any).
    float modifier = 0.0f;
    AuraEffectList const& ModPowerRegenPCTAuras = GetAuraEffectsByType(SPELL_AURA_MOD_POWER_REGEN_PERCENT);
    for (AuraEffectList::const_iterator i = ModPowerRegenPCTAuras.begin(); i != ModPowerRegenPCTAuras.end(); ++i)
        if (Powers((*i)->GetMiscValue()) == power)
            modifier += (*i)->GetAmount();

    if (modifier)
        AddPct(addvalue, modifier);

    if (power != POWER_ENERGY)
        addvalue += GetTotalAuraModifierByMiscValue(SPELL_AURA_MOD_POWER_REGEN, power) * (isHunterPet() ? PET_FOCUS_REGEN_INTERVAL : CREATURE_REGEN_INTERVAL) / (5 * IN_MILLISECONDS);
    else
        addvalue += (float)GetTotalAuraModifierByMiscValue(SPELL_AURA_MOD_POWER_REGEN, power) * ((float)CREATURE_REGEN_INTERVAL / 5140.0f);

    if (power == POWER_ENERGY)
    {
        tmpEnergyReg += std::min((float)CREATURE_REGEN_ENERGY_INTERVAL * addvalue / CREATURE_REGEN_INTERVAL, 10.0f);
        int32 enerReg = int32(tmpEnergyReg);
        if (enerReg)
        {
            ModifyPower(power, enerReg);
            tmpEnergyReg = fmod(tmpEnergyReg, 1);
        }
    }
    else
        ModifyPower(power, int32(addvalue));
}

//misc
uint32 Creature::GetBlockPercent()
{
    if (bot_AI)
        return std::max<uint32>(bot_AI->GetShieldBlockValue() / 1000, 30);

    return 30;
}
//end misc
uint8 Creature::GetBotClass() const
{
    switch (m_bot_class)
    {
        case DRUID_BEAR_FORM:
        case DRUID_CAT_FORM:
        //case TRAVEL:
        //case FLY:
            return CLASS_DRUID;
        default:
            return m_bot_class;
    }
}

void Creature::SetIAmABot(bool bot)
{
    if (!bot)
    {
        bot_AI->UnsummonAll();
        IsAIEnabled = false;
        bot_AI = NULL;
        SetUInt64Value(UNIT_FIELD_CREATEDBY, 0);
    }
}

void Creature::SetBotsPetDied()
{
    if (!m_bots_pet)
        return;

    m_bots_pet->SetCharmerGUID(0);
    m_bots_pet->SetCreatureOwner(NULL);
    //m_bots_pet->GetBotPetAI()->SetCreatureOwner(NULL);
    m_bots_pet->SetIAmABot(false);
	m_bot_owner->SetMinion((Minion*)m_bots_pet, false, PET_SLOT_NOT_IN_SLOT);
    m_bots_pet->CleanupsBeforeDelete();
    m_bots_pet->AddObjectToRemoveList();
    m_bots_pet = NULL;
}

void Creature::SetBotTank(Unit* newtank)
{
    if (!bot_AI || !IsAIEnabled)
        return;

    uint64 tankGuid = bot_AI->GetBotTankGuid();
    if (newtank && newtank->GetGUID() == tankGuid)
        return;

    Creature* oldtank = tankGuid && IS_CREATURE_GUID(tankGuid) ? sObjectAccessor->GetObjectInWorld(tankGuid, (Creature*)NULL) : NULL;
    if (oldtank && oldtank->IsInWorld() && (oldtank->GetIAmABot() || oldtank->GetIAmABotsPet()))
    {
        oldtank->RemoveAurasDueToSpell(DEFENSIVE_STANCE_PASSIVE);
        uint8 ClassOrPetType = oldtank->GetIAmABotsPet() ? bot_pet_ai::GetPetType(oldtank) : oldtank->GetBotClass();
        oldtank->GetBotAI()->ApplyPassives(ClassOrPetType);
    }

    if (newtank == this)
    {
        for (uint8 i = 0; i < 2 + GetMap()->IsRaid(); ++i)
            AddAura(DEFENSIVE_STANCE_PASSIVE, this);

        if (m_bot_owner)
        {
            switch (urand(1,5))
            {
                case 1: MonsterWhisper("I am tank here!", m_bot_owner->GetGUID()); break;
                case 2: MonsterWhisper("I will tank now.", m_bot_owner->GetGUID()); break;
                case 3: MonsterWhisper("I gonna tank", m_bot_owner->GetGUID()); break;
                case 4: MonsterWhisper("I think I will be best tank here...", m_bot_owner->GetGUID()); break;
                case 5: MonsterWhisper("I AM the tank!", m_bot_owner->GetGUID()); break;
            }
        }

        bot_AI->UpdateHealth();
        if (!isInCombat())
            SetBotCommandState(COMMAND_FOLLOW, true);
    }

    bot_AI->SetBotTank(newtank);
}

void Creature::SetBotCommandState(CommandStates st, bool force)
{
    if (bot_AI && IsAIEnabled)
        bot_AI->SetBotCommandState(st, force);
}
CommandStates Creature::GetBotCommandState() const
{
    return bot_AI ? bot_AI->GetBotCommandState() : COMMAND_ABANDON;
}
//Bot damage mods
void Creature::ApplyBotDamageMultiplierMelee(uint32& damage, CalcDamageInfo& damageinfo) const
{
    if (bot_AI)
        bot_AI->ApplyBotDamageMultiplierMelee(damage, damageinfo);
}
void Creature::ApplyBotDamageMultiplierMelee(int32& damage, SpellNonMeleeDamage& damageinfo, SpellInfo const* spellInfo, WeaponAttackType attackType, bool& crit) const
{
    if (bot_AI)
        bot_AI->ApplyBotDamageMultiplierMelee(damage, damageinfo, spellInfo, attackType, crit);
}
void Creature::ApplyBotDamageMultiplierSpell(int32& damage, SpellNonMeleeDamage& damageinfo, SpellInfo const* spellInfo, WeaponAttackType attackType, bool& crit) const
{
    if (bot_AI)
        bot_AI->ApplyBotDamageMultiplierSpell(damage, damageinfo, spellInfo, attackType, crit);
}

void Creature::ApplyBotDamageMultiplierEffect(SpellInfo const* spellInfo, uint8 effect_index, float &value) const
{
    if (bot_AI)
        bot_AI->ApplyBotDamageMultiplierEffect(spellInfo, effect_index, value);
}

bool Creature::GetIAmABot() const
{
    return bot_AI && bot_AI->IsMinionAI();
}

bool Creature::GetIAmABotsPet() const
{
    return bot_AI && bot_AI->IsPetAI();
}

bot_minion_ai* Creature::GetBotMinionAI() const
{
    return IsAIEnabled && bot_AI && bot_AI->IsMinionAI() ? const_cast<bot_minion_ai*>(bot_AI->GetMinionAI()) : NULL;
}

bot_pet_ai* Creature::GetBotPetAI() const
{
    return IsAIEnabled && bot_AI && bot_AI->IsPetAI() ? const_cast<bot_pet_ai*>(bot_AI->GetPetAI()) : NULL;
}

void Creature::InitBotAI(bool asPet)
{
    ASSERT(!bot_AI);

    if (asPet)
        bot_AI = (bot_pet_ai*)AI();
    else
        bot_AI = (bot_minion_ai*)AI();
}

void Creature::SetBotShouldUpdateStats()
{
    if (bot_AI)
        bot_AI->SetShouldUpdateStats();
}

void Creature::OnBotSummon(Creature* summon)
{
    if (bot_AI)
        bot_AI->OnBotSummon(summon);
}

void Creature::OnBotDespawn(Creature* summon)
{
    if (bot_AI)
        bot_AI->OnBotDespawn(summon);
}

void Creature::RemoveBotItemBonuses(uint8 slot)
{
    if (bot_AI)
        bot_AI->RemoveItemBonuses(slot);
}
void Creature::ApplyBotItemBonuses(uint8 slot)
{
    if (bot_AI)
        bot_AI->ApplyItemBonuses(slot);
}

bool Creature::CanUseOffHand() const
{
    return bot_AI && bot_AI->CanUseOffHand();
}
bool Creature::CanUseRanged() const
{
    return bot_AI && bot_AI->CanUseRanged();
}
bool Creature::CanEquip(ItemTemplate const* item, uint8 slot) const
{
    return bot_AI && bot_AI->CanEquip(item, slot);
}
bool Creature::Unequip(uint8 slot) const
{
    return bot_AI && bot_AI->Unequip(slot);
}
bool Creature::Equip(uint32 itemId, uint8 slot) const
{
    return bot_AI && bot_AI->Equip(itemId, slot);
}
bool Creature::ResetEquipment(uint8 slot) const
{
    return bot_AI && bot_AI->ResetEquipment(slot);
}

void Creature::PrepareChanneledCast(float facing, uint32 spell_id, bool triggered)
{
    GetMotionMaster()->Clear();
    AttackStop();
    SetReactState(REACT_PASSIVE);
    SetFacingTo(facing);

    if (spell_id)
        CastSpell(this, spell_id, triggered);
}

void Creature::RemoveChanneledCast(uint64 target)
{
    SetReactState(REACT_AGGRESSIVE);

    if (Unit* itr = ObjectAccessor::GetUnit(*this, target))
    {
        GetMotionMaster()->MoveChase(itr);
        Attack(itr, true);
    }
    else if (Player* itr = FindNearestPlayer(100.0f))
    {
        GetMotionMaster()->MoveChase(itr);
        Attack(itr, true);
    }
}

float Creature::GetVisibilityDistance() const
{
    if (m_visibleDistance > -1)
    {
        switch (m_visibleDistance)
        {
        case VD_TINY:
            return VISIBILITY_DISTANCE_TINY;
        case VD_SMALL:
            return VISIBILITY_DISTANCE_SMALL;
        case VD_NORMAL:
            return VISIBILITY_DISTANCE_NORMAL;
        case VD_LARGE:
            return VISIBILITY_DISTANCE_LARGE;
        case VD_GIGANTIC:
            return VISIBILITY_DISTANCE_GIGANTIC;
        case VD_INFINITE:
            return VISIBILITY_DISTANCE_INFINITE;
        case VD_ENTIRE_MAP:
            return VISIBILITY_DISTANCE_ENTIRE_MAP;
        }
    }
    return GetMap()->GetVisibilityRange();
}

void Creature::OnEvade(uint32 /*reason*/)
{
    if (IsAIEnabled)
        if (!AI()->IsIgnoringEvade())
            AI()->EnterEvadeMode();
}