/*
 * Copyright (C) 2008-2013 monsterCore <http://www.monstercore.org/>
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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

/* ScriptData
SDName: Mulgore
SD%Complete: 100
SDComment: Support for quest: 11129, 861
SDCategory: Mulgore
EndScriptData */

/* ContentData
npc_skorn_whitecloud
npc_kyle_frenzied
EndContentData */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "Player.h"
#include "SpellInfo.h"

/*######
# npc_skorn_whitecloud
######*/

#define GOSSIP_SW "Tell me a story, Skorn."

class npc_skorn_whitecloud : public CreatureScript
{
public:
    npc_skorn_whitecloud() : CreatureScript("npc_skorn_whitecloud") { }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        if (action == GOSSIP_ACTION_INFO_DEF)
            player->SEND_GOSSIP_MENU(523, creature->GetGUID());
		if (action == GOSSIP_ACTION_TRADE)
			player->GetSession()->SendListInventory(creature->GetGUID());
        return true;
    }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (creature->isQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

		if (!player->GetQuestRewardStatus(770))
		{
			player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SW, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
			player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, GOSSIP_TEXT_BROWSE_GOODS, GOSSIP_SENDER_INFO, GOSSIP_ACTION_TRADE); // Npc is a vendor as well
		}

		else
			player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, GOSSIP_TEXT_BROWSE_GOODS, GOSSIP_SENDER_INFO, GOSSIP_ACTION_TRADE); // Npc is a vendor as well
		player->SEND_GOSSIP_MENU(7640, creature->GetGUID());

        return true;
    }

};

/*#####
# npc_kyle_frenzied
######*/

enum KyleFrenzied
{
    EMOTE_SEE_LUNCH         = 0,
    EMOTE_EAT_LUNCH         = 1,
    EMOTE_DANCE             = 2,

    SPELL_LUNCH             = 42222,
    NPC_KYLE_FRENZIED       = 23616,
    NPC_KYLE_FRIENDLY       = 23622,
    POINT_ID                = 1
};

class npc_kyle_frenzied : public CreatureScript
{
public:
    npc_kyle_frenzied() : CreatureScript("npc_kyle_frenzied") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_kyle_frenziedAI (creature);
    }

    struct npc_kyle_frenziedAI : public ScriptedAI
    {
        npc_kyle_frenziedAI(Creature* creature) : ScriptedAI(creature) {}

        bool EventActive;
        bool IsMovingToLunch;
        uint64 PlayerGUID;
        uint32 EventTimer;
        uint8 EventPhase;

        void Reset()
        {
            EventActive = false;
            IsMovingToLunch = false;
            PlayerGUID = 0;
            EventTimer = 5000;
            EventPhase = 0;

            if (me->GetEntry() == NPC_KYLE_FRIENDLY)
                me->UpdateEntry(NPC_KYLE_FRENZIED);
        }

        void SpellHit(Unit* Caster, SpellInfo const* Spell)
        {
            if (!me->getVictim() && !EventActive && Spell->Id == SPELL_LUNCH)
            {
                if (Caster->GetTypeId() == TYPEID_PLAYER)
                    PlayerGUID = Caster->GetGUID();

                if (me->GetMotionMaster()->GetCurrentMovementGeneratorType() == WAYPOINT_MOTION_TYPE)
                {
                    me->GetMotionMaster()->MovementExpired();
                    me->GetMotionMaster()->MoveIdle();
                    me->StopMoving();
                }

                EventActive = true;
                Talk(EMOTE_SEE_LUNCH);
                me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_ONESHOT_CREATURE_SPECIAL);
            }
        }

        void MovementInform(uint32 Type, uint32 PointId)
        {
            if (Type != POINT_MOTION_TYPE || !EventActive)
                return;

            if (PointId == POINT_ID)
                IsMovingToLunch = false;
        }

        void UpdateAI(const uint32 diff)
        {
            if (EventActive)
            {
                if (IsMovingToLunch)
                    return;

                if (EventTimer <= diff)
                {
                    EventTimer = 5000;
                    ++EventPhase;

                    switch (EventPhase)
                    {
                        case 1:
                            if (Unit* unit = Unit::GetUnit(*me, PlayerGUID))
                            {
                                if (GameObject* go = unit->GetGameObject(SPELL_LUNCH))
                                {
                                    IsMovingToLunch = true;
                                    me->GetMotionMaster()->MovePoint(POINT_ID, go->GetPositionX(), go->GetPositionY(), go->GetPositionZ());
                                }
                            }
                            break;
                        case 2:
                            Talk(EMOTE_EAT_LUNCH);
                            me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_USE_STANDING);
                            break;
                        case 3:
                            if (Player* unit = Unit::GetPlayer(*me, PlayerGUID))
                                unit->TalkedToCreature(me->GetEntry(), me->GetGUID());

                            me->UpdateEntry(NPC_KYLE_FRIENDLY);
                            break;
                        case 4:
                            EventTimer = 30000;
                            Talk(EMOTE_DANCE);
                            me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_DANCESPECIAL);
                            break;
                        case 5:
                            me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_NONE);
                            Reset();
                            me->GetMotionMaster()->Clear();
                            break;
                    }
                }
                else
                    EventTimer -= diff;
            }
        }
    };

};
/*
class npc_stonebull_crayfish : public CreatureScript
{
public:
    npc_stonebull_crayfish() : CreatureScript("npc_stonebull_crayfish") {}

    struct npc_stonebull_crayfishAI : public PassiveAI
    {
        npc_stonebull_crayfishAI(Creature* creature) : PassiveAI(creature) {}

        void OnSpellClick(Unit* player, bool& result)
        {
            if (!result)
                return;

            me->DespawnOrUnsummon(550);
        }
    };

    CreatureAI* GetAI(Creature * creature) const
    {
        return new npc_stonebull_crayfishAI(creature);
    }
};*/

/*#####
# npc_plains_vision
######*/

float wp_plain_vision[50][3] =
{
	{ -2226.32f, -408.095f, -9.36235f },
	{ -2203.04f, -437.212f, -5.72498f },
	{ -2163.91f, -457.851f, -7.09049f },
	{ -2123.87f, -448.137f, -9.29591f },
	{ -2104.66f, -427.166f, -6.49513f },
	{ -2101.48f, -422.826f, -5.3567f },
	{ -2097.56f, -417.083f, -7.16716f },
	{ -2084.87f, -398.626f, -9.88973f },
	{ -2072.71f, -382.324f, -10.2488f },
	{ -2054.05f, -356.728f, -6.22468f },
	{ -2051.8f, -353.645f, -5.35791f },
	{ -2049.08f, -349.912f, -6.15723f },
	{ -2030.6f, -310.724f, -9.59302f },
	{ -2002.15f, -249.308f, -10.8124f },
	{ -1972.85f, -195.811f, -10.6316f },
	{ -1940.93f, -147.652f, -11.7055f },
	{ -1888.06f, -81.943f, -11.4404f },
	{ -1837.05f, -34.0109f, -12.258f },
	{ -1796.12f, -14.6462f, -10.3581f },
	{ -1732.61f, -4.27746f, -10.0213f },
	{ -1688.94f, -0.829945f, -11.7103f },
	{ -1681.32f, 13.0313f, -9.48056f },
	{ -1677.04f, 36.8349f, -7.10318f },
	{ -1675.2f, 68.559f, -8.95384f },
	{ -1676.57f, 89.023f, -9.65104f },
	{ -1678.16f, 110.939f, -10.1782f },
	{ -1677.86f, 128.681f, -5.73869f },
	{ -1675.27f, 144.324f, -3.47916f },
	{ -1671.7f, 163.169f, -1.23098f },
	{ -1666.61f, 181.584f, 5.26145f },
	{ -1661.51f, 196.154f, 8.95252f },
	{ -1655.47f, 210.811f, 8.38727f },
	{ -1647.07f, 226.947f, 5.27755f },
	{ -1621.65f, 232.91f, 2.69579f },
	{ -1600.23f, 237.641f, 2.98539f },
	{ -1576.07f, 242.546f, 4.66541f },
	{ -1554.57f, 248.494f, 6.60377f },
	{ -1547.53f, 259.302f, 10.6741f },
	{ -1541.7f, 269.847f, 16.4418f },
	{ -1539.83f, 278.989f, 21.0597f },
	{ -1540.16f, 290.219f, 27.8247f },
	{ -1538.99f, 298.983f, 34.0032f },
	{ -1540.38f, 307.337f, 41.3557f },
	{ -1536.61f, 314.884f, 48.0179f },
	{ -1532.42f, 323.277f, 55.6667f },
	{ -1528.77f, 329.774f, 61.1525f },
	{ -1525.65f, 333.18f, 63.2161f },
	{ -1517.01f, 350.713f, 62.4286f },
	{ -1511.39f, 362.537f, 62.4539f },
	{ -1508.68f, 366.822f, 62.733f }
};

class npc_plains_vision : public CreatureScript
{
public:
	npc_plains_vision() : CreatureScript("npc_plains_vision") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_plains_visionAI(creature);
	}

	struct npc_plains_visionAI : public ScriptedAI
	{
		npc_plains_visionAI(Creature* creature) : ScriptedAI(creature) {}

		bool newWaypoint;
		uint8 WayPointId;
		uint8 amountWP;

		void Reset()
		{
			WayPointId = 0;
			newWaypoint = true;
			amountWP = 49;
		}

		void EnterCombat(Unit* /*who*/) {}

		void MovementInform(uint32 type, uint32 id)
		{
			if (type != POINT_MOTION_TYPE)
				return;

			if (id < amountWP)
			{
				++WayPointId;
				newWaypoint = true;
			}
			else
			{
				me->setDeathState(JUST_DIED);
				me->RemoveCorpse();
			}
		}

		void UpdateAI(const uint32 /*diff*/)
		{
			if (newWaypoint)
			{
				me->GetMotionMaster()->MovePoint(WayPointId, wp_plain_vision[WayPointId][0], wp_plain_vision[WayPointId][1], wp_plain_vision[WayPointId][2]);
				newWaypoint = false;
			}
		}
	};
};

// 36790
class npc_eagle_spirit : public CreatureScript
{
public:
	npc_eagle_spirit() : CreatureScript("npc_eagle_spirit") { }

	struct npc_eagle_spiritAI : public ScriptedAI
	{
		npc_eagle_spiritAI(Creature *c) : ScriptedAI(c) {}

		void Reset()
		{
			me->SetSpeed(MOVE_FLIGHT, 4.0f, true);
			me->GetMotionMaster()->MovePath(36790, false);
		}

		void MovementInform(uint32 type, uint32 pointId) override
		{
			if (type == WAYPOINT_MOTION_TYPE && pointId == 5)
				if (Vehicle* vehicle = me->GetVehicleKit())
					if (Unit* unit = vehicle->GetPassenger(0))
						if (Player* player = unit->ToPlayer())
							player->ExitVehicle();
		}
	};

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_eagle_spiritAI(creature);
	}
};

void AddSC_mulgore()
{
    new npc_skorn_whitecloud();
    new npc_kyle_frenzied();
    // new npc_stonebull_crayfish(); // Handled through SAI
	new npc_plains_vision();
	new npc_eagle_spirit();
}
