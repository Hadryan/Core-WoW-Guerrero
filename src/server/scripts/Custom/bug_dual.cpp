#include "ScriptPCH.h"
#define GOSSIP_TEXT_TEST        500000
#define GOSSIP_XP_OFF_1           "Deseas reparar tu personaje bugeado?"
#define MSG_PLACE "ARREGLAR DUAL SOLO SIRVE PARA NIVELES 85"
#define GOSSIP_XP_OFF_2           "SUBA A NIVEL 10"
#define GOSSIP_XP_OFF_3           "SUBA A NIVEL 85"

class bug_dual : public CreatureScript
{
public:
	bug_dual() : CreatureScript("bug_dual") { }

	bool OnGossipHello(Player* player, Creature* creature)
	{


		//Check if player is new player
		if (player->getLevel() == 85)
		{
			player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_XP_OFF_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
			player->PlayerTalkClass->SendGossipMenu(GOSSIP_TEXT_TEST, creature->GetGUID());
		}
		//if (player->getLevel() == 9)
		//{
		//player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_XP_OFF_2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
		//player->PlayerTalkClass->SendGossipMenu(GOSSIP_TEXT_TEST, creature->GetGUID());
		//}
		//if (player->getLevel() == 10)
		//{
		//player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_XP_OFF_3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
		//player->PlayerTalkClass->SendGossipMenu(GOSSIP_TEXT_TEST, creature->GetGUID());
		//}
		return true;
	}

	bool OnGossipSelect(Player* player, Creature* /*creature*/, uint32 /*sender*/, uint32 action)
	{
		player->PlayerTalkClass->ClearMenus();
		switch (action)
		{
			// CASO I
		case GOSSIP_ACTION_INFO_DEF + 1:
		{

										   player->GiveLevel(9);
										   player->PlayerTalkClass->SendCloseGossip();
										   player->SaveToDB();
										   player->GiveLevel(10);
										   player->PlayerTalkClass->SendCloseGossip();
										   player->SaveToDB();
										   player->GiveLevel(85);
										   player->PlayerTalkClass->SendCloseGossip();
										   player->SaveToDB();
		};
			// CASO II
			//case GOSSIP_ACTION_INFO_DEF + 2:
			//{

			//player->GiveLevel(10);
			//player->PlayerTalkClass->SendCloseGossip();
			//player->SaveToDB();
			//};
			// CASO III
			//case GOSSIP_ACTION_INFO_DEF + 3:
			//{

			//player->GiveLevel(85);
			//player->PlayerTalkClass->SendCloseGossip();
			//player->SaveToDB();
			//};
		}
		return true;
	}
};

#include "ScriptPCH.h"

#define GOSSIP_TEXT_TEST        500000
#define GOSSIP_XP_OFF_1           "Esto solo funciona si eres nivel 20 al 79"
#define MSG_PLACE "TIENES QUE SER NIVEL 20 PARA PEDIR LA PROMOCIÓN 85"
//#define GOSSIP_XP_OFF_2           "Promoción para Dks"

class Get_NPC : public CreatureScript
{
public:
	Get_NPC() : CreatureScript("Get_NPC") { }

	bool OnGossipHello(Player* player, Creature* creature)
	{
		player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_XP_OFF_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
		player->PlayerTalkClass->SendGossipMenu(GOSSIP_TEXT_TEST, creature->GetGUID());
		return true;
	}

	bool OnGossipSelect(Player* player, Creature* /*creature*/, uint32 /*sender*/, uint32 action)
	{
		player->PlayerTalkClass->ClearMenus();


		if (player->getLevel() >= 80 || player->getLevel() <= 19)
		{
			ChatHandler(player->GetSession()).PSendSysMessage("|cffff0000Ya no puedes hacer esto");
			player->PlayerTalkClass->SendCloseGossip();
			return true;
		}

		switch (action)
		{
			// Levelup 80
		case GOSSIP_ACTION_INFO_DEF + 1:
		{
										   player->SetMoney(150000000);
										   player->GiveLevel(80);
										   player->PlayerTalkClass->SendCloseGossip();
										   //player->learnSpell(33388, true);
										   //player->learnSpell(33391, true);
										   //player->learnSpell(34090, true);
										   //player->learnSpell(34091, true);
										   //player->learnSpell(54197, true);
										   //player->learnSpell(90267, true);
										   //player->learnSpell(90265, true);
										   player->AddItem(51809, 4);

										   if (player->GetTeam() == ALLIANCE)
										   { //ID DEL SPELL QUE LLEVE A SHOP ALIANZA
											   player->TeleportTo(0, -8826.86141f, 622.722778f, 94.881386f, 0.310105f);
											   //pPlayer->CastSpell(pPlayer,61420,true,NULL,NULL,pPlayer->GetGUID());
										   }
										   else
										   { //ID DEL SPELL QUE LLEVE A SHOP HORDA
											   player->TeleportTo(1, 1570.515625f, -4396.883789f, 16.021320f, 0.592816f);
											   //pPlayer->CastSpell(pPlayer,34673,true,NULL,NULL,pPlayer->GetGUID());
										   }
										   player->PlayerTalkClass->SendCloseGossip();
										   player->SaveToDB();
		}
			break;
			// caso 2  para dks
			/*case GOSSIP_ACTION_INFO_DEF + 2:
			{
			player->SetMoney(5000000);
			player->GiveLevel(85);
			player->PlayerTalkClass->SendCloseGossip();
			player->learnSpell(33388, true);
			player->learnSpell(33391, true);
			player->learnSpell(34090, true);
			player->learnSpell(34091, true);
			player->learnSpell(54197, true);
			player->learnSpell(90267, true);
			player->learnSpell(90265, true);
			player->AddItem(51809, 4);
			if (player->GetTeam() == ALLIANCE){ //ID DEL SPELL QUE LLEVE A SHOP ALIANZA
			player->TeleportTo(0, -8826.86141f, 622.722778f, 94.881386f, 0.310105f);
			//pPlayer->CastSpell(pPlayer,61420,true,NULL,NULL,pPlayer->GetGUID());
			}
			else { //ID DEL SPELL QUE LLEVE A SHOP HORDA
			player->TeleportTo(1, 1570.515625f, -4396.883789f, 16.021320f, 0.592816f);
			//pPlayer->CastSpell(pPlayer,34673,true,NULL,NULL,pPlayer->GetGUID());
			}
			player->PlayerTalkClass->SendCloseGossip();
			player->SaveToDB();
			}
			break;*/
			player->PlayerTalkClass->SendCloseGossip();
		}
		return true;
	}

	struct gambler_passivesAI : public ScriptedAI
	{
		gambler_passivesAI(Creature * c) : ScriptedAI(c){ }

		uint32 uiAdATimer;
		uint32 uiAdBTimer;
		uint32 uiAdCTimer;

		void Reset()
		{
			uiAdATimer = 1000;
			uiAdBTimer = 23000;
			uiAdCTimer = 11000;
		}


		void UpdateAI(const uint32 diff)
		{

			if (uiAdATimer <= diff)
			{
				me->MonsterSay("Bienvenidos a MonsterWoW", LANG_UNIVERSAL, NULL);
				me->HandleEmoteCommand(EMOTE_ONESHOT_EXCLAMATION);
				me->CastSpell(me, 44940);
				uiAdATimer = 61000;
			}
			else
				uiAdATimer -= diff;

			if (uiAdBTimer <= diff)
			{
				me->MonsterSay("Si tienes entre nivel 20 a 79 acercate a mi", LANG_UNIVERSAL, NULL);
				me->HandleEmoteCommand(EMOTE_ONESHOT_EXCLAMATION);
				uiAdBTimer = 61000;
			}
			else
				uiAdBTimer -= diff;
		}
	};

	CreatureAI * GetAI(Creature * pCreature) const
	{
		return new gambler_passivesAI(pCreature);
	}

};

class RateExp_On_Kill : public PlayerScript
{
public:
	RateExp_On_Kill() : PlayerScript("RateExp_On_Kill") { }

	void OnLogin(Player * Killer)
	{
		if (Killer->getLevel() >= 80)
		{
			Killer->CastSpell(Killer, 57353);
			Killer->CastSpell(Killer, 57353);
			Killer->CastSpell(Killer, 57353);
		}
	}
};

void AddSC_bug_dual()
{
	new bug_dual;
	new Get_NPC;
	new RateExp_On_Kill;
}
