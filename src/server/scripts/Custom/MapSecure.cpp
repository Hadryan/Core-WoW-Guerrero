#include "ScriptMgr.h"
#include "Group.h"

enum ForbiddenMaps
{
	MAP_INFRALAR = 646, //.tele Deepholm
};

class map_security_level : public PlayerScript
{
public:
	map_security_level() : PlayerScript("map_security_level") {}

	void OnUpdateZone(Player* player, uint32 newZone, uint32 newArea)
	{
		switch (player->GetMapId())
		{
			case MAP_INFRALAR:
			{
			if (player->getLevel() > 82)
			{
				player->GetSession()->SendNotification("Tienes el lvl requerido");
				return;
			}
				
			if (player->GetTeam() == HORDE)
			{	
				player->TeleportTo(1, 1361.152344f, -4374.675293f, 26.095335f, 0.196188f);
			}
			
			if (player->GetTeam() == ALLIANCE)
			{	
				player->TeleportTo(0, -9091.350586f, 415.365051f, 92.122787f, 0.614164f);
			}				
		}
		break;
		}
	}
};

void AddSC_MapSecureLvl()
{
	new map_security_level();
}