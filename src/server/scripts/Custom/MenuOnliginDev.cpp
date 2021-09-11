#include "ScriptPCH.h"

#define MENU_ID 123 // Our menuID used to match the sent menu to select hook (playerscript)

class example_PlayerGossip : public PlayerScript
{
public: 
    example_PlayerGossip() : PlayerScript("example_PlayerGossip") {}

    void OnLogin(Player* player)        // Any hook hered
    {
        player->PlayerTalkClass->ClearMenus();                              // Clears old options 
        player->ADD_GOSSIP_ITEM(4, " |TInterface\\glues\\common\\GLUES-WOW-CCLOGO:80:150:-15|t", GOSSIP_SENDER_MAIN, 999);
        player->ADD_GOSSIP_ITEM(4, " |cFF8B0000BIENVENIDOS A MONSTER-WOW", GOSSIP_SENDER_MAIN, 999);
		player->ADD_GOSSIP_ITEM(4, " |cFF8B0000---------------------------------------", GOSSIP_SENDER_MAIN, 999);
		player->ADD_GOSSIP_ITEM(4, " |cFF9400D3Nuevo Core y Base de datos actualizada 2021", GOSSIP_SENDER_MAIN, 999);
		player->ADD_GOSSIP_ITEM(4, " |cFF8B0000---------------------------------------", GOSSIP_SENDER_MAIN, 999);
		player->ADD_GOSSIP_ITEM(4, "    |cFF006400Anticheat On |cFF8B0000| |cFF006400Secure System On", GOSSIP_SENDER_MAIN, 999);
                                                                            // SetMenuId must be after clear menu and before send menu!!
        player->PlayerTalkClass->GetGossipMenu().SetMenuId(MENU_ID);        // Sets menu ID so we can identify our menu in Select hook. Needs unique number for the menu
		player->SEND_GOSSIP_MENU(1006365, player->GetGUID());
    }
};


void AddSC_WelcomeWindowDev() // Add to scriptloader normally
{
    new example_PlayerGossip();
}
