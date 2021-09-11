#include "ScriptMgr.h"
#include "Group.h"

const uint32 ONE_CHARACTER_VIP = 18158;

enum ForbiddenAreas
{
	WEB_SHOP = 268,  // prevent enter normal players, no more
	AREA_GM_ISLAND = 876,   // cant enter players only gm
};

class map_security : public PlayerScript
{
public:
	map_security() : PlayerScript("map_security") {}

	void OnUpdateZone(Player* player, uint32 newZone, uint32 newArea)
	{
		uint32 spellId = 54844;
		player->AddAura(spellId, player);
		player->RemoveAura(spellId);

		switch (player->GetAreaId())
		{
		case WEB_SHOP:
		{
			if (player->GetSession()->GetSecurity() == 1 
			|| player->GetSession()->GetSecurity() == 2 
			|| player->GetSession()->GetSecurity() == 3 
			|| player->GetSession()->GetSecurity() == 4 
			|| player->GetSession()->GetSecurity() == 5 
			|| player->GetSession()->GetSecurity() == 6 
			|| player->GetSession()->GetSecurity() == 7 
			|| player->GetSession()->GetSecurity() == 8 
			|| player->GetSession()->GetSecurity() == 9 
			|| player->GetSession()->GetSecurity() == 10 
			|| player->GetSession()->GetSecurity() == 11 
			|| player->GetSession()->GetSecurity() == 12 
			|| player->GetSession()->GetSecurity() == 13 
			|| player->GetSession()->GetSecurity() == 14 
			|| player->GetSession()->GetSecurity() == 255
			|| player->HasItemCount(ONE_CHARACTER_VIP, 1))
			return;

			player->TeleportTo(571, 5813.780273f, 650.739685f, 647.398682f, 4.028399f); // Prison
			player->CastSpell(player, 39258);
			player->CastSpell(player, 35182);
			player->CastSpell(player, 38505);
			player->Say("|cFFFF0000SOY UN MURLOC SIMPLE SIN DIRECCION", LANG_UNIVERSAL);
			player->SetDisplayId(506);
		}
		case AREA_GM_ISLAND:
		{
			if (player->GetSession()->GetSecurity() == 1 
			|| player->GetSession()->GetSecurity() == 2 
			|| player->GetSession()->GetSecurity() == 3 
			|| player->GetSession()->GetSecurity() == 4 
			|| player->GetSession()->GetSecurity() == 5 
			|| player->GetSession()->GetSecurity() == 6 
			|| player->GetSession()->GetSecurity() == 7 
			|| player->GetSession()->GetSecurity() == 8 
			|| player->GetSession()->GetSecurity() == 9 
			|| player->GetSession()->GetSecurity() == 10 
			|| player->GetSession()->GetSecurity() == 11 
			|| player->GetSession()->GetSecurity() == 12 
			|| player->GetSession()->GetSecurity() == 13 
			|| player->GetSession()->GetSecurity() == 14 
			|| player->GetSession()->GetSecurity() == 255
			|| player->HasItemCount(ONE_CHARACTER_VIP, 1))
			return;

			player->TeleportTo(571, 5813.780273f, 650.739685f, 647.398682f, 4.028399f); // Prison
			player->CastSpell(player, 39258);
			player->CastSpell(player, 35182);
			player->CastSpell(player, 38505);
			player->Say("|cFFFF0000SOY UN MURLOC SIMPLE SIN DIRECCION", LANG_UNIVERSAL);
			player->SetDisplayId(506);
		}

		break;
		}
	}
};

class gamemasters_security : public PlayerScript
{
public:
	gamemasters_security() : PlayerScript("gamemasters_security") {}

	void OnLogin(Player* player)
	{
			if (player->GetSession()->GetSecurity() == 1 
			|| player->GetSession()->GetSecurity() == 2 
			|| player->GetSession()->GetSecurity() == 3 
			|| player->GetSession()->GetSecurity() == 4 
			|| player->GetSession()->GetSecurity() == 5 
			|| player->GetSession()->GetSecurity() == 6 
			|| player->GetSession()->GetSecurity() == 7 
			|| player->GetSession()->GetSecurity() == 8 
			|| player->GetSession()->GetSecurity() == 9 
			|| player->GetSession()->GetSecurity() == 10 
			|| player->GetSession()->GetSecurity() == 11 
			|| player->GetSession()->GetSecurity() == 12 
			|| player->GetSession()->GetSecurity() == 13 
			|| player->GetSession()->GetSecurity() == 14
			|| player->GetSession()->GetSecurity() == 255)
		{
			for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; slot++)
				player->DestroyItem(INVENTORY_SLOT_BAG_0, slot, true);
			
			ChatHandler(player->GetSession()).PSendSysMessage("|cff00ccffRecuerda que todo lo que haces queda registrado en nuestros LOGS.|r");
			player->EquipNewItem(EQUIPMENT_SLOT_CHEST, 2586, true); // pechera gm
			player->EquipNewItem(EQUIPMENT_SLOT_HEAD, 12064, true); // capucha gm
			player->EquipNewItem(EQUIPMENT_SLOT_MAINHAND, 192, true); // arma gm
			player->EquipNewItem(EQUIPMENT_SLOT_FEET, 11508, true); // pies gm
			//player->AddAura(52561, player);
			//player->AddAura(60044, player); 
		}

		SessionMap sessions = sWorld->GetAllSessions();
		for (SessionMap::iterator itr = sessions.begin(); itr != sessions.end(); ++itr)
		{
			if (Player* plr = itr->second->GetPlayer())
			{
				if (player->GetSession()->GetSecurity() >= 0)
					return;

				if (player != plr)
				{
					if (player->GetSession()->GetRemoteAddress() == plr->GetSession()->GetRemoteAddress())
						;//player->GetSession()->KickPlayer();
				}
			}
		}
	}
};

void AddSC_GMItems()
{
	new gamemasters_security();
	new map_security();
}