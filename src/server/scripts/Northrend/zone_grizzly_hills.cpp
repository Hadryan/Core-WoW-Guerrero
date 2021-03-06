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

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedEscortAI.h"
#include "Player.h"
#include "SpellInfo.h"
#include "CreatureTextMgr.h"

/*######
## Quest 12027: Mr. Floppy's Perilous Adventure
######*/

enum eFloppy
{
    NPC_MRFLOPPY                = 26589,
    NPC_HUNGRY_WORG             = 26586,
    NPC_RAVENOUS_WORG           = 26590,   //RWORG
    NPC_EMILY                   = 26588,

    QUEST_PERILOUS_ADVENTURE    = 12027,

    SPELL_MRFLOPPY              = 47184,    //vehicle aura

    SAY_WORGHAGGRO1             = 0, //Um... I think one of those wolves is back...
    SAY_WORGHAGGRO2             = 1, //He's going for Mr. Floppy!
    SAY_WORGRAGGRO3             = 2, //Oh, no! Look, it's another wolf, and it's a biiiiiig one!
    SAY_WORGRAGGRO4             = 3, //He's gonna eat Mr. Floppy! You gotta help Mr. Floppy! You just gotta!
    SAY_RANDOMAGGRO             = 4, //There's a big meanie attacking Mr. Floppy! Help!
    SAY_VICTORY1                = 5, //Let's get out of here before more wolves find us!
    SAY_VICTORY2                = 6, //Don't go toward the light, Mr. Floppy!
    SAY_VICTORY3                = 7, //Mr. Floppy, you're ok! Thank you so much for saving Mr. Floppy!
    SAY_VICTORY4                = 8, //I think I see the camp! We're almost home, Mr. Floppy! Let's go!
    TEXT_EMOTE_WP1              = 9, //Mr. Floppy revives
    TEXT_EMOTE_AGGRO            = 10, //The Ravenous Worg chomps down on Mr. Floppy
    SAY_QUEST_ACCEPT            = 11, //Are you ready, Mr. Floppy? Stay close to me and watch out for those wolves!
    SAY_QUEST_COMPLETE          = 12  //Thank you for helping me get back to the camp. Go tell Walter that I'm safe now!
};

//emily
class npc_emily : public CreatureScript
{
public:
    npc_emily() : CreatureScript("npc_emily") { }

    struct npc_emilyAI : public npc_escortAI
    {
        npc_emilyAI(Creature* creature) : npc_escortAI(creature) { }

        uint32 m_uiChatTimer;

        uint64 RWORGGUID;
        uint64 MrfloppyGUID;

        bool Completed;

        void JustSummoned(Creature* summoned)
        {
            if (Creature* Mrfloppy = GetClosestCreatureWithEntry(me, NPC_MRFLOPPY, 50.0f))
                summoned->AI()->AttackStart(Mrfloppy);
            else
                summoned->AI()->AttackStart(me->getVictim());
        }

        void WaypointReached(uint32 waypointId)
        {
            Player* player = GetPlayerForEscort();
            if (!player)
                return;

            switch (waypointId)
            {
                case 9:
                    if (Creature* Mrfloppy = GetClosestCreatureWithEntry(me, NPC_MRFLOPPY, 100.0f))
                        MrfloppyGUID = Mrfloppy->GetGUID();
                    break;
                case 10:
                    if (Unit::GetCreature(*me, MrfloppyGUID))
                    {
                        Talk(SAY_WORGHAGGRO1);
                        me->SummonCreature(NPC_HUNGRY_WORG, me->GetPositionX()+5, me->GetPositionY()+2, me->GetPositionZ()+1, 3.229f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 120000);
                    }
                    break;
                case 11:
                    if (Creature* Mrfloppy = Unit::GetCreature(*me, MrfloppyGUID))
                        Mrfloppy->GetMotionMaster()->MoveFollow(me, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);
                    break;
                case 17:
                    if (Creature* Mrfloppy = Unit::GetCreature(*me, MrfloppyGUID))
                        Mrfloppy->GetMotionMaster()->MovePoint(0, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ());
                    Talk(SAY_WORGRAGGRO3);
                    if (Creature* RWORG = me->SummonCreature(NPC_RAVENOUS_WORG, me->GetPositionX()+10, me->GetPositionY()+8, me->GetPositionZ()+2, 3.229f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 120000))
                    {
                        RWORG->setFaction(35);
                        RWORGGUID = RWORG->GetGUID();
                    }
                    break;
                case 18:
                    if (Creature* Mrfloppy = Unit::GetCreature(*me, MrfloppyGUID))
                    {
                        if (Creature* RWORG = Unit::GetCreature(*me, RWORGGUID))
                            RWORG->GetMotionMaster()->MovePoint(0, Mrfloppy->GetPositionX(), Mrfloppy->GetPositionY(), Mrfloppy->GetPositionZ());
                        DoCast(Mrfloppy, SPELL_MRFLOPPY);
                    }
                    break;
                case 19:
                    if (Creature* Mrfloppy = Unit::GetCreature(*me, MrfloppyGUID))
                    {
                        if (Mrfloppy->HasAura(SPELL_MRFLOPPY, 0))
                        {
                            if (Creature* RWORG = Unit::GetCreature(*me, RWORGGUID))
                                Mrfloppy->EnterVehicle(RWORG);
                        }
                    }
                    break;
                case 20:
                    if (Creature* RWORG = Unit::GetCreature(*me, RWORGGUID))
                        RWORG->HandleEmoteCommand(34);
                    break;
                case 21:
                    if (Creature* Mrfloppy = Unit::GetCreature(*me, MrfloppyGUID))
                    {
                        if (Creature* RWORG = Unit::GetCreature(*me, RWORGGUID))
                        {
                            RWORG->Kill(Mrfloppy);
                            Mrfloppy->ExitVehicle();
                            RWORG->setFaction(14);
                            RWORG->GetMotionMaster()->MovePoint(0, RWORG->GetPositionX()+10, RWORG->GetPositionY()+80, RWORG->GetPositionZ());
                            Talk(SAY_VICTORY2);
                        }
                    }
                    break;
                case 22:
                    if (Creature* Mrfloppy = Unit::GetCreature(*me, MrfloppyGUID))
                    {
                        if (Mrfloppy->isDead())
                        {
                            if (Creature* RWORG = Unit::GetCreature(*me, RWORGGUID))
                                RWORG->DisappearAndDie();
                            me->GetMotionMaster()->MovePoint(0, Mrfloppy->GetPositionX(), Mrfloppy->GetPositionY(), Mrfloppy->GetPositionZ());
                            Mrfloppy->setDeathState(ALIVE);
                            Mrfloppy->GetMotionMaster()->MoveFollow(me, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);
                            Talk(SAY_VICTORY3);
                        }
                    }
                    break;
                case 24:
                    if (player)
                    {
                        Completed = true;
                        player->GroupEventHappens(QUEST_PERILOUS_ADVENTURE, me);
                        Talk(SAY_QUEST_COMPLETE, player->GetGUID());
                    }
                    me->SetWalk(false);
                    break;
                case 25:
                    Talk(SAY_VICTORY4);
                    break;
                case 27:
                    me->DisappearAndDie();
                    if (Creature* Mrfloppy = Unit::GetCreature(*me, MrfloppyGUID))
                        Mrfloppy->DisappearAndDie();
                    break;
            }
        }

        void EnterCombat(Unit* /*Who*/)
        {
            Talk(SAY_RANDOMAGGRO);
        }

        void Reset()
        {
            m_uiChatTimer = 4000;
            MrfloppyGUID = 0;
            RWORGGUID = 0;
        }

        void UpdateAI(const uint32 uiDiff)
        {
            npc_escortAI::UpdateAI(uiDiff);

            if (HasEscortState(STATE_ESCORT_ESCORTING))
            {
                if (m_uiChatTimer <= uiDiff)
                    m_uiChatTimer = 12000;
                else
                    m_uiChatTimer -= uiDiff;
            }
        }
    };

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
    {
        if (quest->GetQuestId() == QUEST_PERILOUS_ADVENTURE)
        {
            creature->AI()->Talk(SAY_QUEST_ACCEPT);
            if (Creature* Mrfloppy = GetClosestCreatureWithEntry(creature, NPC_MRFLOPPY, 180.0f))
                Mrfloppy->GetMotionMaster()->MoveFollow(creature, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);

            if (npc_escortAI* pEscortAI = CAST_AI(npc_emily::npc_emilyAI, (creature->AI())))
                pEscortAI->Start(true, false, player->GetGUID());
        }
        return true;
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_emilyAI(creature);
    }
};

//mrfloppy
class npc_mrfloppy : public CreatureScript
{
public:
    npc_mrfloppy() : CreatureScript("npc_mrfloppy") { }

    struct npc_mrfloppyAI : public ScriptedAI
    {
        npc_mrfloppyAI(Creature* creature) : ScriptedAI(creature) {}

        uint64 EmilyGUID;
        uint64 RWORGGUID;
        uint64 HWORGGUID;

        void Reset() {}

        void EnterCombat(Unit* Who)
        {
            if (Creature* Emily = GetClosestCreatureWithEntry(me, NPC_EMILY, 50.0f))
            {
                switch (Who->GetEntry())
                {
                    case NPC_HUNGRY_WORG:
                        Emily->AI()->Talk(SAY_WORGHAGGRO2);
                        break;
                    case NPC_RAVENOUS_WORG:
                        Emily->AI()->Talk(SAY_WORGRAGGRO4);
                        break;
                    default:
                        Emily->AI()->Talk(SAY_RANDOMAGGRO);
                }
            }
        }

        void EnterEvadeMode() {}

        void MoveInLineOfSight(Unit* /*who*/) {}

        void UpdateAI(const uint32 /*diff*/)
        {
            if (!UpdateVictim())
                return;
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_mrfloppyAI(creature);
    }
};

// Outhouse Bunny

enum eOuthouseBunny
{
    SPELL_OUTHOUSE_GROANS           = 48382,
    SPELL_CAMERA_SHAKE              = 47533,
    SPELL_DUST_FIELD                = 48329
};

enum eSounds
{
    SOUND_FEMALE        = 12671,
    SOUND_MALE          = 12670
};

class npc_outhouse_bunny : public CreatureScript
{
public:
    npc_outhouse_bunny() : CreatureScript("npc_outhouse_bunny") { }

    struct npc_outhouse_bunnyAI : public ScriptedAI
    {
        npc_outhouse_bunnyAI(Creature* creature) : ScriptedAI(creature) {}

        uint8 m_counter;
        uint8 m_gender;

        void Reset()
        {
            m_counter = 0;
            m_gender = 0;
        }

        void SetData(uint32 uiType, uint32 uiData)
        {
            if (uiType == 1)
                m_gender = uiData;
        }

        void SpellHit(Unit* pCaster, const SpellInfo* pSpell)
        {
             if (pSpell->Id == SPELL_OUTHOUSE_GROANS)
             {
                ++m_counter;
                if (m_counter < 5)
                    DoCast(pCaster, SPELL_CAMERA_SHAKE, true);
                else
                    m_counter = 0;
                DoCast(me, SPELL_DUST_FIELD, true);
                switch (m_gender)
                {
                    case GENDER_FEMALE:
                        DoPlaySoundToSet(me, SOUND_FEMALE);
                        break;

                    case GENDER_MALE:
                        DoPlaySoundToSet(me, SOUND_MALE);
                        break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_outhouse_bunnyAI(creature);
    }
};

// Tallhorn Stage

enum etallhornstage
{
    OBJECT_HAUNCH                   = 188665
};

class npc_tallhorn_stag : public CreatureScript
{
public:
    npc_tallhorn_stag() : CreatureScript("npc_tallhorn_stag") { }

    struct npc_tallhorn_stagAI : public ScriptedAI
    {
        npc_tallhorn_stagAI(Creature* creature) : ScriptedAI(creature) {}

        uint8 m_uiPhase;

        void Reset()
        {
            m_uiPhase = 1;
        }

        void UpdateAI(const uint32 /*uiDiff*/)
        {
            if (m_uiPhase == 1)
            {
                if (me->FindNearestGameObject(OBJECT_HAUNCH, 2.0f))
                {
                    me->SetStandState(UNIT_STAND_STATE_DEAD);
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                    me->SetUInt32Value(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);
                }
                m_uiPhase = 0;
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_tallhorn_stagAI(creature);
    }
};

// Amberpine Woodsman

enum eamberpinewoodsman
{
    TALLHORN_STAG                   = 26363
};

class npc_amberpine_woodsman : public CreatureScript
{
public:
    npc_amberpine_woodsman() : CreatureScript("npc_amberpine_woodsman") { }

    struct npc_amberpine_woodsmanAI : public ScriptedAI
    {
        npc_amberpine_woodsmanAI(Creature* creature) : ScriptedAI(creature) {}

        uint8 m_uiPhase;
        uint32 m_uiTimer;

        void Reset()
        {
            m_uiTimer = 0;
            m_uiPhase = 1;
        }

        void UpdateAI(const uint32 uiDiff)
        {
            // call this each update tick?
            if (me->FindNearestCreature(TALLHORN_STAG, 0.2f))
            {
                me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_USE_STANDING);
            }
            else
                if (m_uiPhase)
                {
                    if (m_uiTimer <= uiDiff)
                    {
                        switch (m_uiPhase)
                        {
                            case 1:
                                me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_LOOT);
                                m_uiTimer = 3000;
                                m_uiPhase = 2;
                                break;
                            case 2:
                                me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_ONESHOT_ATTACK1H);
                                m_uiTimer = 4000;
                                m_uiPhase = 1;
                                break;
                        }
                    }
                    else
                    m_uiTimer -= uiDiff;
                }
                ScriptedAI::UpdateAI(uiDiff);

            UpdateVictim();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_amberpine_woodsmanAI(creature);
    }
};
/*######
## Quest 12288: Overwhelmed!
######*/

enum eSkirmisher
{
    SPELL_RENEW_SKIRMISHER      = 48812,
    CREDIT_NPC                  = 27466,

    RANDOM_SAY                  = 0,
};

class npc_wounded_skirmisher : public CreatureScript
{
public:
    npc_wounded_skirmisher() : CreatureScript("npc_wounded_skirmisher") { }

    struct npc_wounded_skirmisherAI : public ScriptedAI
    {
        npc_wounded_skirmisherAI(Creature* creature) : ScriptedAI(creature) {}

        uint64 uiPlayerGUID;

        uint32 DespawnTimer;

        void Reset()
        {
            DespawnTimer = 5000;
            uiPlayerGUID = 0;
        }

        void MovementInform(uint32, uint32 id)
        {
            if (id == 1)
                me->DespawnOrUnsummon(DespawnTimer);
        }

        void SpellHit(Unit* caster, const SpellInfo* spell)
        {
            if (spell->Id == SPELL_RENEW_SKIRMISHER && caster->GetTypeId() == TYPEID_PLAYER
                && caster->ToPlayer()->GetQuestStatus(12288) == QUEST_STATUS_INCOMPLETE)
            {
                caster->ToPlayer()->KilledMonsterCredit(CREDIT_NPC, 0);
                sCreatureTextMgr->SendChat(me, RANDOM_SAY, 0, CHAT_MSG_ADDON, LANG_ADDON, TEXT_RANGE_NORMAL, 0, TEAM_OTHER, false, caster->ToPlayer());
                if (me->IsStandState())
                    me->GetMotionMaster()->MovePoint(1, me->GetPositionX()+7, me->GetPositionY()+7, me->GetPositionZ());
                else
                {
                    me->SetStandState(UNIT_STAND_STATE_STAND);
                    me->DespawnOrUnsummon(DespawnTimer);
                }
            }
        }

        void UpdateAI(const uint32 /*diff*/)
        {
            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_wounded_skirmisherAI(creature);
    }
};

/*Lightning Sentry - if you kill it when you have your Minion with you, you will get a quest credit*/
enum eSentry
{
    QUEST_OR_MAYBE_WE_DONT_A                     = 12138,
    QUEST_OR_MAYBE_WE_DONT_H                     = 12198,

    NPC_LIGHTNING_SENTRY                         = 26407,
    NPC_WAR_GOLEM                                = 27017,

    SPELL_CHARGED_SENTRY_TOTEM                   = 52703,
};

class npc_lightning_sentry : public CreatureScript
{
public:
    npc_lightning_sentry() : CreatureScript("npc_lightning_sentry") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_lightning_sentryAI(creature);
    }

    struct npc_lightning_sentryAI : public ScriptedAI
    {
        npc_lightning_sentryAI(Creature* creature) : ScriptedAI(creature) { }

        uint32 uiChargedSentryTotem;

        void Reset()
        {
            uiChargedSentryTotem = urand(10000, 12000);
        }

        void UpdateAI(const uint32 uiDiff)
        {
            if (!UpdateVictim())
                return;

            if (uiChargedSentryTotem <= uiDiff)
            {
                DoCast(SPELL_CHARGED_SENTRY_TOTEM);
                uiChargedSentryTotem = urand(10000, 12000);
            }
            else
                uiChargedSentryTotem -= uiDiff;

            DoMeleeAttackIfReady();
        }

        void JustDied(Unit* killer)
        {
            if (killer->ToPlayer() && killer->ToPlayer()->GetTypeId() == TYPEID_PLAYER)
            {
                if (me->FindNearestCreature(NPC_WAR_GOLEM, 10.0f, true))
                {
                    if (killer->ToPlayer()->GetQuestStatus(QUEST_OR_MAYBE_WE_DONT_A) == QUEST_STATUS_INCOMPLETE ||
                        killer->ToPlayer()->GetQuestStatus(QUEST_OR_MAYBE_WE_DONT_H) == QUEST_STATUS_INCOMPLETE)
                        killer->ToPlayer()->KilledMonsterCredit(NPC_WAR_GOLEM, 0);
                }
            }
        }
    };
};

/*Venture co. Straggler - when you cast Smoke Bomb, he will yell and run away*/
enum eSmokeEmOut
{
    SAY_SEO                                     = 0,
    QUEST_SMOKE_EM_OUT_A                        = 12323,
    QUEST_SMOKE_EM_OUT_H                        = 12324,
    SPELL_SMOKE_BOMB                            = 49075,
    SPELL_CHOP                                  = 43410,
    SPELL_VENTURE_STRAGGLER_CREDIT              = 49093,
};

class npc_venture_co_straggler : public CreatureScript
{
    public:
        npc_venture_co_straggler() : CreatureScript("npc_venture_co_straggler") { }

        struct npc_venture_co_stragglerAI : public ScriptedAI
        {
            npc_venture_co_stragglerAI(Creature* creature) : ScriptedAI(creature) { }

            uint64 uiPlayerGUID;
            uint32 uiRunAwayTimer;
            uint32 uiTimer;
            uint32 uiChopTimer;

            void Reset()
            {
                uiPlayerGUID = 0;
                uiTimer = 0;
                uiRunAwayTimer = 0;
                uiChopTimer = urand(10000, 12500);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IMMUNE_TO_PC);
                me->SetReactState(REACT_AGGRESSIVE);
            }

            void UpdateAI(const uint32 uiDiff)
            {
                if (uiPlayerGUID && uiRunAwayTimer <= uiDiff)
                {
                    if (Player* player = Unit::GetPlayer(*me, uiPlayerGUID))
                    {
                        switch (uiTimer)
                        {
                            case 0:
                                DoCast(player, SPELL_VENTURE_STRAGGLER_CREDIT);
                                me->GetMotionMaster()->MovePoint(0, me->GetPositionX()-7, me->GetPositionY()+7, me->GetPositionZ());
                                uiRunAwayTimer = 2500;
                                ++uiTimer;
                                break;
                            case 1:
                                Talk(SAY_SEO);
                                me->GetMotionMaster()->MovePoint(0, me->GetPositionX()-7, me->GetPositionY()-5, me->GetPositionZ());
                                uiRunAwayTimer = 2500;
                                ++uiTimer;
                                break;
                            case 2:
                                me->GetMotionMaster()->MovePoint(0, me->GetPositionX()-5, me->GetPositionY()-5, me->GetPositionZ());
                                uiRunAwayTimer = 2500;
                                ++uiTimer;
                                break;
                            case 3:
                                me->DisappearAndDie();
                                uiTimer = 0;
                                break;
                        }
                    }
                }
                else if (uiRunAwayTimer)
                    uiRunAwayTimer -= uiDiff;

                if (!UpdateVictim())
                    return;

                if (uiChopTimer <= uiDiff)
                {
                    DoCastVictim(SPELL_CHOP);
                    uiChopTimer = urand(10000, 12000);
                }
                else
                    uiChopTimer -= uiDiff;

                DoMeleeAttackIfReady();
            }

            void SpellHit(Unit* caster, SpellInfo const* spell)
            {
                if (spell->Id == SPELL_SMOKE_BOMB && caster->GetTypeId() == TYPEID_PLAYER)
                {
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IMMUNE_TO_PC);
                    me->SetReactState(REACT_PASSIVE);
                    me->CombatStop(false);
                    uiPlayerGUID = caster->GetGUID();
                    uiRunAwayTimer = 3500;
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_venture_co_stragglerAI(creature);
        }
};

#include "ScriptPCH.h"
#include "ScriptedEscortAI.h"

#define GOSSIP_ITEM1 "You're free to go Orsonn, but first tell me what's wrong with the furbolg."
#define GOSSIP_ITEM2 "What happened then?"
#define GOSSIP_ITEM3 "Thank you, Son of Ursoc. I'll see what can be done."
#define GOSSIP_ITEM4 "Who was this stranger?"
#define GOSSIP_ITEM5 "Thank you, Kodian. I'll do what I can."

enum eEnums
{
	GOSSIP_TEXTID_ORSONN1 = 12793,
	GOSSIP_TEXTID_ORSONN2 = 12794,
	GOSSIP_TEXTID_ORSONN3 = 12796,

	GOSSIP_TEXTID_KODIAN1 = 12797,
	GOSSIP_TEXTID_KODIAN2 = 12798,

	NPC_ORSONN = 27274,
	NPC_KODIAN = 27275,

	//trigger creatures
	NPC_ORSONN_CREDIT = 27322,
	NPC_KODIAN_CREDIT = 27321,

	QUEST_CHILDREN_OF_URSOC = 12247,
	QUEST_THE_BEAR_GODS_OFFSPRING = 12231
};

class npc_orsonn_and_kodian : public CreatureScript
{
public:
	npc_orsonn_and_kodian() : CreatureScript("npc_orsonn_and_kodian") { }

	bool OnGossipHello(Player* pPlayer, Creature* pCreature)
	{

		if (pPlayer->GetQuestStatus(QUEST_CHILDREN_OF_URSOC) == QUEST_STATUS_INCOMPLETE || pPlayer->GetQuestStatus(QUEST_THE_BEAR_GODS_OFFSPRING) == QUEST_STATUS_INCOMPLETE)
		{
			switch (pCreature->GetEntry())
			{
			case NPC_ORSONN:
				if (!pPlayer->GetReqKillOrCastCurrentCount(QUEST_CHILDREN_OF_URSOC, NPC_ORSONN_CREDIT) || !pPlayer->GetReqKillOrCastCurrentCount(QUEST_THE_BEAR_GODS_OFFSPRING, NPC_ORSONN_CREDIT))
				{
					pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
					pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXTID_ORSONN1, pCreature->GetGUID());
					return true;
				}
				break;
			case NPC_KODIAN:
				if (!pPlayer->GetReqKillOrCastCurrentCount(QUEST_CHILDREN_OF_URSOC, NPC_KODIAN_CREDIT) || !pPlayer->GetReqKillOrCastCurrentCount(QUEST_THE_BEAR_GODS_OFFSPRING, NPC_KODIAN_CREDIT))
				{
					pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM4, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
					pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXTID_KODIAN1, pCreature->GetGUID());
					return true;
				}
				break;
			}
		}

		pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetGUID());
		return true;
	}

	bool OnGossipSelect(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction)
	{
		pPlayer->PlayerTalkClass->ClearMenus();
		switch (uiAction)
		{
		case GOSSIP_ACTION_INFO_DEF + 1:
			pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
			pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXTID_ORSONN2, pCreature->GetGUID());
			break;
		case GOSSIP_ACTION_INFO_DEF + 2:
			pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
			pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXTID_ORSONN3, pCreature->GetGUID());
			break;
		case GOSSIP_ACTION_INFO_DEF + 3:
			pPlayer->CLOSE_GOSSIP_MENU();
			pPlayer->TalkedToCreature(NPC_ORSONN_CREDIT, pCreature->GetGUID());
			break;

		case GOSSIP_ACTION_INFO_DEF + 4:
			pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM5, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5);
			pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXTID_KODIAN2, pCreature->GetGUID());
			break;
		case GOSSIP_ACTION_INFO_DEF + 5:
			pPlayer->CLOSE_GOSSIP_MENU();
			pPlayer->TalkedToCreature(NPC_KODIAN_CREDIT, pCreature->GetGUID());
			break;
		}

		return true;
	}
};

/*######
## Quest: A Blade Fit For A Champion
######*/

enum LakeFrog
{
	// Creature
	NPC_LAKE_FROG = 33211,
	NPC_LAKE_FROG_QUEST = 33224,
	NPC_MAIDEN_OF_ASHWOOD_LAKE = 33220,
	// Items
	ITEM_WARTS_B_GONE_LIP_BALM = 44986,
	// Spells
	SPELL_WARTSBGONE_LIP_BALM = 62574,
	SPELL_FROG_LOVE = 62537, // for 1 minute !
	SPELL_WARTS = 62581,
	SPELL_MAIDEN_OF_ASHWOOD_LAKE_TRANSFORM = 62550,
	SPELL_SUMMON_ASHWOOD_BRAND = 62554,
	SPELL_FROG_KISS = 62536,
	// Text
	SAY_MAIDEN_0 = 0,
	SAY_MAIDEN_1 = 1
};

enum LakeFrogEvents
{
	EVENT_LAKEFROG_1 = 1,
	EVENT_LAKEFROG_2 = 2,
	EVENT_LAKEFROG_3 = 3,
	EVENT_LAKEFROG_4 = 4,
	EVENT_LAKEFROG_5 = 5
};

class npc_lake_frog : public CreatureScript
{
public:
	npc_lake_frog() : CreatureScript("npc_lake_frog") { }

	struct npc_lake_frogAI : public ScriptedAI
	{
		npc_lake_frogAI(Creature* creature) : ScriptedAI(creature) { }

		void Reset()
		{
			_following = false;
			_runningScript = false;
			if (me->GetEntry() == NPC_LAKE_FROG_QUEST)
				me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
		}

		void UpdateAI(uint32 const diff)
		{
			if (_following)
				if (!me->HasAura(SPELL_FROG_LOVE))
					me->DespawnOrUnsummon(1000);

			_events.Update(diff);

			while (uint32 eventId = _events.ExecuteEvent())
			{
				switch (eventId)
				{
				case EVENT_LAKEFROG_1:
					DoCast(me, SPELL_MAIDEN_OF_ASHWOOD_LAKE_TRANSFORM);
					me->SetEntry(NPC_MAIDEN_OF_ASHWOOD_LAKE);
					_events.ScheduleEvent(EVENT_LAKEFROG_2, 2000);
					break;
				case EVENT_LAKEFROG_2:
					Talk(SAY_MAIDEN_0);
					_events.ScheduleEvent(EVENT_LAKEFROG_3, 3000);
					break;
				case EVENT_LAKEFROG_3:
					me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
					_events.ScheduleEvent(EVENT_LAKEFROG_4, 25000);
					break;
				case EVENT_LAKEFROG_4:
					me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
					_events.ScheduleEvent(EVENT_LAKEFROG_5, 2000);
					break;
				case EVENT_LAKEFROG_5:
					Talk(SAY_MAIDEN_1);
					me->DespawnOrUnsummon(4000);
					break;
				default:
					break;
				}
			}
		}

		void ReceiveEmote(Player* player, uint32 emote)
		{
			if (_following || _runningScript)
				return;

			if (emote == TEXT_EMOTE_KISS && me->IsWithinDistInMap(player, 30.0f) && player->HasItemCount(ITEM_WARTS_B_GONE_LIP_BALM, 1, false))
			{
				if (!player->HasAura(SPELL_WARTSBGONE_LIP_BALM))
					player->AddAura(SPELL_WARTS, player);
				else
				{
					DoCast(player, SPELL_FROG_KISS); // Removes SPELL_WARTSBGONE_LIP_BALM

					if (me->GetEntry() == NPC_LAKE_FROG)
					{
						me->AddAura(SPELL_FROG_LOVE, me);
						me->GetMotionMaster()->MoveFollow(player, 0.3f, frand(M_PI / 2, M_PI + (M_PI / 2)));
						_following = true;
					}
					else if (me->GetEntry() == NPC_LAKE_FROG_QUEST)
					{
						me->GetMotionMaster()->MoveIdle();
						me->SetFacingToObject(player);
						_runningScript = true;
						_events.ScheduleEvent(EVENT_LAKEFROG_1, 2000);
					}
				}
			}
		}

		void sGossipSelect(Player* player, uint32 /*sender*/, uint32 /*action*/)
		{
			DoCast(player, SPELL_SUMMON_ASHWOOD_BRAND);
		}

	private:
		EventMap _events;
		bool   _following;
		bool   _runningScript;
	};

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_lake_frogAI(creature);
	}
};

enum InfectedWorgenBite
{
    SPELL_INFECTED_WORGEN_BITE = 53094,
    SPELL_WORGENS_CALL = 53095
};

class spell_infected_worgen_bite : public SpellScriptLoader
{
public:
    spell_infected_worgen_bite() : SpellScriptLoader("spell_infected_worgen_bite") { }

    class spell_infected_worgen_bite_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_infected_worgen_bite_AuraScript);

        void HandleAfterEffectApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            Unit* target = GetTarget();
            if (target->GetTypeId() == TYPEID_PLAYER)
                if (GetStackAmount() == GetSpellInfo()->StackAmount)
                {
                    if (target->GetTypeId() == TYPEID_PLAYER)
                        target->RemoveAura(SPELL_INFECTED_WORGEN_BITE);

                    target->CastSpell(target, SPELL_WORGENS_CALL, true);
                }
        }

        void Register()
        {
            AfterEffectApply += AuraEffectApplyFn(spell_infected_worgen_bite_AuraScript::HandleAfterEffectApply, EFFECT_1, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAPPLY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_infected_worgen_bite_AuraScript();
    }
};

enum ShredderDelivery
{
    NPC_BROKEN_DOWN_SHREDDER = 27354
};

class spell_shredder_delivery : public SpellScriptLoader
{
public:
    spell_shredder_delivery() : SpellScriptLoader("spell_shredder_delivery") { }

    class spell_shredder_delivery_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_shredder_delivery_SpellScript);

        bool Load() override
        {
            return GetCaster()->GetTypeId() == TYPEID_UNIT;
        }

        void HandleScript(SpellEffIndex /*effIndex*/)
        {
            if (GetCaster()->ToCreature()->GetEntry() == NPC_BROKEN_DOWN_SHREDDER)
                GetCaster()->ToCreature()->DespawnOrUnsummon();
        }

        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(spell_shredder_delivery_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_shredder_delivery_SpellScript();
    }
};

void AddSC_grizzly_hills()
{
    new npc_emily();
    new npc_mrfloppy();
    new npc_outhouse_bunny();
    new npc_tallhorn_stag();
    new npc_amberpine_woodsman();
    new npc_wounded_skirmisher();
    new npc_lightning_sentry();
    new npc_venture_co_straggler();
	new npc_orsonn_and_kodian();
	new npc_lake_frog();
    new spell_infected_worgen_bite();
    new spell_shredder_delivery();
}
