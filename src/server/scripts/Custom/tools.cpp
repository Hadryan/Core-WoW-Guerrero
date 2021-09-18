/* Made by: aaddss A.K.A Deathsoul
untested (in game)
*/


#include "ScriptPCH.h"

class npc_tool : public CreatureScript
{
public:
	npc_tool() : CreatureScript("npc_tool"){}

	bool OnGossipHello(Player * pPlayer, Creature * pCreature)
	{

		pPlayer->ADD_GOSSIP_ITEM(4, " Cambio de Raza 100 Tokens", GOSSIP_SENDER_MAIN, 0);
		pPlayer->ADD_GOSSIP_ITEM(4, " Cambio de Faccion 200  Tokens", GOSSIP_SENDER_MAIN, 1);
		pPlayer->ADD_GOSSIP_ITEM(4, " Personalizar el PJ 100 Tokens", GOSSIP_SENDER_MAIN, 2);
		pPlayer->ADD_GOSSIP_ITEM(4, " Cambio de nombre 100 Tokens", GOSSIP_SENDER_MAIN, 3);
		pPlayer->PlayerTalkClass->SendGossipMenu(9425, pCreature->GetGUID());
		return true;
	}

	bool OnGossipSelect(Player * Player, Creature * Creature, uint32 /*uiSender*/, uint32 uiAction)
	{
		if (!Player)
			return true;

		switch (uiAction)
		{
		case 0:
			if (Player->HasItemCount(29434, 50)) // race change token (12345) you can change it
			{
				Player->DestroyItemCount(29434, 50, true, true); // race change token (12345) you can change it
				Player->DestroyItemCount(29434, 50, true, false); // race change token (12345) you can change it
				Player->SetAtLoginFlag(AT_LOGIN_CHANGE_RACE);
				CharacterDatabase.PExecute("UPDATe characters Set at_login = at_login | '128' Where guid = %u", Player->GetGUID());
				Player->GetSession()->SendNotification("Relogea porfavor!");

			}
			else
			{
				Player->GetSession()->SendNotification("Necesitas 100 Distinvidos de Justicia!");

			}
			break;
		case 1:
			if (Player->HasItemCount(29434, 100)) // race change token (123) you can change it
			{
				Player->DestroyItemCount(29434, 100, true, true); // race change token (123) you can change it
				Player->DestroyItemCount(29434, 100, true, false); // race change token (123) you can change it
				Player->SetAtLoginFlag(AT_LOGIN_CHANGE_FACTION);
				CharacterDatabase.PExecute("UPDATE characters SET at_login = at_login | '64' WHERE guid = %u", Player->GetGUID());
				Player->GetSession()->SendNotification("Relogea porfavor!");


			}

			else
			{
				Player->GetSession()->SendNotification("Necesitas 100 Distintivos de Justicia!");

			}
			break;
		case 2:
			if (Player->HasItemCount(29434, 50)) // customize change token (152) you can change it
			{
				Player->DestroyItemCount(29434, 50, true, true); // customize change token (152) you can change it
				Player->DestroyItemCount(29434, 50, true, false); // customize change token (152) you can change it
				Player->SetAtLoginFlag(AT_LOGIN_CUSTOMIZE);
				CharacterDatabase.PExecute("UPDATE characters SET at_login = at_login | '8' WHERE guid = %u", Player->GetGUID());
				Player->GetSession()->SendNotification("Necesitas relogear!");


			}

			else
			{
				Player->GetSession()->SendNotification("Necesitas 100 Distintivo de Justicia!");

			}
			break;
		case 3:
			if (Player->HasItemCount(29434, 50)) // name change token (1552) you can change it
			{
				Player->DestroyItemCount(29434, 50, true, true); // name change token (1552) you can change it
				Player->DestroyItemCount(29434, 50, true, false); // name change token (1552) you can change it
				Player->SetAtLoginFlag(AT_LOGIN_RENAME);
				CharacterDatabase.PExecute("UPDATE characters SET at_login = at_login | '8' WHERE guid = %u", Player->GetGUID());
				Player->GetSession()->SendNotification("Necesitas Relogear!");


			}

			else
			{
				Player->GetSession()->SendNotification("Necesitas 100 Distintivo de Justicia!");

			}
			break;
		}
		return true;
	}
};

void AddSc_npc_tool()
{
	new npc_tool();
}
