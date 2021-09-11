#include "ScriptMgr.h"
#include "Cell.h"
#include "CellImpl.h"
#include "GameEventMgr.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Unit.h"
#include "GameObject.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "InstanceScript.h"
#include "CombatAI.h"
#include "PassiveAI.h"
#include "Chat.h"
#include "DBCStructure.h"
#include "DBCStores.h"
#include "ObjectMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "Channel.h"
#include "Language.h"
#include "DBCStructure.h"
#include "InstanceScript.h"
#include "Map.h"
#include "MapManager.h"
#include "Group.h"
#include "GroupMgr.h"

#include "Config.h"
#include "WorldSession.h"
#include "Player.h"
#include "World.h"
#include "DatabaseEnv.h"
#define Playedtimetochat 1500
#define mutetimecencure 300
#define mutetimeantispam 10
#define FACTION_SPECIFIC 0

const uint32 ONE_CHARACTER_VIP = 18156;
const uint32 ONE_CHARACTER_VIP_2 = 18157;

using namespace std;
const char* CLASS_ICON;
const char* RACE_ICON;

std::string GetNameLink(Player* player)
{
	std::string name = player->GetName();
	std::string color;
	std::string icon;
	switch (player->getRace())
	{
		// Done - Bloodelf
	case RACE_BLOODELF:
		if (player->getGender() == GENDER_MALE)
			RACE_ICON = "|TInterface/ICONS/Achievement_Character_Bloodelf_Male:15|t";
		else
			RACE_ICON = "|TInterface/ICONS/Achievement_Character_Bloodelf_Female:15|t";
		break;
		// Done - Dranei
	case RACE_DRAENEI:
		if (player->getGender() == GENDER_FEMALE)
			RACE_ICON = "|TInterface/ICONS/Achievement_Character_Draenei_Female:15|t";
		else
			RACE_ICON = "|TInterface/ICONS/Achievement_Character_Draenei_Male:15|t";
		break;
	case RACE_DWARF:
		if (player->getGender() == GENDER_FEMALE)
			RACE_ICON = "|TInterface/ICONS/Achievement_Character_Dwarf_Female:15|t";
		else
			RACE_ICON = "|TInterface/ICONS/Achievement_Character_Dwarf_Male:15|t";
		break;
		// Done - Gnome
	case RACE_GNOME:
		if (player->getGender() == GENDER_FEMALE)
			RACE_ICON = "|TInterface/ICONS/Achievement_Character_Gnome_Female:15|t";
		else
			RACE_ICON = "|TInterface/ICONS/Achievement_Character_Gnome_Male:15|t";
		break;
		// Done - Human
	case RACE_HUMAN:
		if (player->getGender() == GENDER_FEMALE)
			RACE_ICON = "|TInterface/ICONS/Achievement_Character_Human_Female:15|t";
		else
			RACE_ICON = "|TInterface/ICONS/Achievement_Character_Human_Male:15|t";
		break;
	case RACE_NIGHTELF:
		if (player->getGender() == GENDER_FEMALE)
			RACE_ICON = "|TInterface/ICONS/Achievement_Character_Nightelf_Female:15|t";
		else
			RACE_ICON = "|TInterface/ICONS/Achievement_Character_Nightelf_Male:15|t";
		break;
	case RACE_ORC:
		if (player->getGender() == GENDER_FEMALE)
			RACE_ICON = "|TInterface/ICONS/Achievement_Character_Orc_Female:15|t";
		else
			RACE_ICON = "|TInterface/ICONS/Achievement_Character_Orc_Male:15|t";
		break;
		// Done - Tauren
	case RACE_TAUREN:
		if (player->getGender() == GENDER_FEMALE)
			RACE_ICON = "|TInterface/ICONS/Achievement_Character_Tauren_Female:15|t";
		else
			RACE_ICON = "|TInterface/ICONS/Achievement_Character_Tauren_Male:15|t";
		break;
	case RACE_TROLL:
		if (player->getGender() == GENDER_FEMALE)
			RACE_ICON = "|TInterface/ICONS/Achievement_Character_Troll_Female:15|t";
		else
			RACE_ICON = "|TInterface/ICONS/Achievement_Character_Troll_Male:15|t";
		break;
	case RACE_UNDEAD_PLAYER:
		if (player->getGender() == GENDER_FEMALE)
			RACE_ICON = "|TInterface/ICONS/Achievement_Character_Undead_Female:15|t";
		else
			RACE_ICON = "|TInterface/ICONS/Achievement_Character_Undead_Male:15|t";
		break;
	}

	switch (player->getClass())
	{
	case CLASS_DEATH_KNIGHT:
		color = "|cffC41F3B";
		//CLASS_ICON = "|TInterface\\icons\\Spell_Deathknight_ClassIcon:15|t|r";
		break;
	case CLASS_DRUID:
		color = "|cffFF7D0A";
		// CLASS_ICON = "|TInterface\\icons\\Ability_Druid_Maul:15|t|r";
		break;
	case CLASS_HUNTER:
		color = "|cffABD473";
		// CLASS_ICON = "|TInterface\\icons\\INV_Weapon_Bow_07:15|t|r";
		break;
	case CLASS_MAGE:
		color = "|cff69CCF0";
		//CLASS_ICON = "|TInterface\\icons\\INV_Staff_13:15|t|r";
		break;
	case CLASS_PALADIN:
		color = "|cffF58CBA";
		// CLASS_ICON = "|TInterface\\icons\\INV_Hammer_01:15|t|r";
		break;
	case CLASS_PRIEST:
		color = "|cffFFFFFF";
		// CLASS_ICON = "|TInterface\\icons\\INV_Staff_30:15|t|r";
		break;
	case CLASS_ROGUE:
		color = "|cffFFF569";
		// CLASS_ICON = "|TInterface\\icons\\INV_ThrowingKnife_04:15|t|r";
		break;
	case CLASS_SHAMAN:
		color = "|cff0070DE";
		// CLASS_ICON = "|TInterface\\icons\\Spell_Nature_BloodLust:15|t|r";
		break;
	case CLASS_WARLOCK:
		color = "|cff9482C9";
		//  CLASS_ICON = "|TInterface\\icons\\Spell_Nature_FaerieFire:15|t|r";
		break;
	case CLASS_WARRIOR:
		color = "|cffC79C6E";
		// CLASS_ICON = "|TInterface\\icons\\INV_Sword_27.png:15|t|r";
		break;
	}
	return "|Hplayer:" + name + "|h" + RACE_ICON + "|cffFFFFFF[" + color + name + "|cffFFFFFF]|h|r";
}

void _SendWorldChat(Player* player, string message)
{
	size_t stringpos;

	if (message.find("|TInterface") != string::npos)
		return;

	if (message.find("\n") != string::npos)
		return;

	if ((stringpos = message.find("|Hquest:")) != string::npos)
		return;

	if ((stringpos = message.find("|Htrade:")) != string::npos)
		return;

	if ((stringpos = message.find("|Htalent:")) != string::npos)
		return;

	if ((stringpos = message.find("|Henchant:")) != string::npos)
		return;

	if ((stringpos = message.find("|Hachievement:")) != string::npos)
		return;

	if ((stringpos = message.find("|Hglyph:")) != string::npos)
		return;

	if ((stringpos = message.find("|Hspell:")) != string::npos)
		return;

	if ((stringpos = message.find("Hitem:")) != string::npos)
		return;

	if (message.find("|c") != string::npos && message.find("|r") != string::npos)
		return;

	if (message.find("|c") != string::npos && message.find("|h") != string::npos)
		return;

	uint8 cheksSize = 118;//Change these if you want to add more words to the array.
	std::string checks[118];//Change these if you want to add more words to the array.
	// Strony (Sites)
	// Strony (Sites)
	checks[0] = "http://";
	checks[1] = ".com";
	checks[2] = ".net";
	checks[3] = ".org";
	checks[4] = "www.";
	checks[5] = "wow-";
	checks[6] = "-wow";
	checks[7] = "no-ip";
	checks[8] = ".zapto";
	checks[9] = ".biz";
	checks[10] = ".servegame";
	checks[11] = ".ir";
	checks[12] = "com.br";

	checks[13] = "h t t p : / /";
	checks[14] = ". c o m";
	checks[15] = ". n e t";
	checks[16] = ". o r g";
	checks[17] = "w w w .";
	checks[18] = " w o w -";
	checks[19] = "- w o w";
	checks[20] = "n o - i p";
	checks[21] = ". z a p t o";
	checks[22] = ". b i z";
	checks[23] = ". s e r v e g a m e";
	checks[24] = ". b r";
	checks[25] = "c o m . b r";

	checks[26] = "h  t  t  p  :  /  /";
	checks[27] = ".  c  o  m";
	checks[28] = ".  n  e  t";
	checks[29] = ".  o  r  g";
	checks[30] = "w  w  w  .";
	checks[31] = " w  o  w  -";
	checks[32] = "-  w  o  w";
	checks[33] = "n  o  -  i  p";
	checks[34] = ".  z  a  p  t  o";
	checks[35] = ".  b  i  z";
	checks[36] = ".  s  e  r  v  e  g  a  m  e";
	checks[37] = ".  b  r";
	checks[38] = "c  o  m  .  b  r";

	checks[39] = "h   t   t   p   :   /   /";
	checks[40] = ".   c   o   m";
	checks[41] = ".   n   e   t";
	checks[42] = ".   o   r   g";
	checks[43] = "w   w   w   .";
	checks[44] = " w   o   w   -";
	checks[45] = "-   w   o   w";
	checks[46] = "n   o   -   i   p";
	checks[47] = ".   z   a   p   t   o";
	checks[48] = ".   b   i   z";
	checks[49] = ".   s   e   r   v   e   g   a   m   e";
	checks[50] = ".   b   r";
	checks[51] = "   c   o   m   .   b   r";

	checks[52] = "h    t    t    p   :   /   /";
	checks[53] = ".    c    o    m";
	checks[54] = ".    n    e   t";
	checks[55] = ".    o    r    g";
	checks[56] = "w    w    w    .";
	checks[57] = "w    o    w    -";
	checks[58] = "-    w    o    w";
	checks[59] = "n    o    -    i    p";
	checks[60] = ".    z    a    p    t    o";
	checks[61] = ".    b    i     z";
	checks[62] = ".    s    e    r    v    e    g    a    m    e";
	checks[63] = ".    b    r";
	checks[64] = "c    o    m    .    b    r";

	checks[65] = "trevon";
	checks[66] = "megawow";
	checks[67] = "fatalwow";
	checks[68] = "uniforgiven-wow";
	checks[69] = "wow-autolouco";
	checks[70] = "heaven-wow";
	checks[71] = "fireballwow";
	checks[72] = "wowbrasilpa";
	checks[73] = "fatalitywow";
	checks[74] = "demonic-wow";
	checks[75] = "revenge-wow";
	checks[76] = "heavenwow";
	checks[77] = "logon.";
	checks[78] = "linebr";
	checks[79] = "azralon";
	checks[80] = "ultra";
	checks[81] = "ultra-wow";

	checks[82] = "t r e v o n";
	checks[83] = "m e g a w o w";
	checks[84] = "f a t a l w o w";
	checks[85] = "u n i f o r g i v e n - w o w";
	checks[86] = "w o w - a u t o l o u c o";
	checks[87] = "h e a v e n - w o w";
	checks[88] = "f i r e b a l l w o w";
	checks[89] = "w o w b r a s i l  p a";
	checks[90] = "f a t a l i t y w o w";
	checks[91] = "d e m o n i c - w o w";
	checks[92] = "r e v e n g e - w o w";
	checks[93] = "h e a v e n w o w";
	checks[94] = "u n d e a d - w o w";
	checks[95] = "l i n e b r";
	checks[96] = "a z r a l o n";
	checks[97] = "b l a c k - w o w";
	checks[98] = "t r e v o n w o w";

	checks[99] = "t  r  e  v  o  n";
	checks[100] = "m  e  g  a  w  o  w";
	checks[101] = "f  a  t  a  l  w  o  w";
	checks[102] = "u  n  i  f  o  r  g  i  v  e  n  -  w  o  w";
	checks[103] = "w  o  w  -  a  u  t  o   l o  u  c  o";
	checks[104] = "h  e  a  v  e  n  -  w  o  w";
	checks[105] = "f  i  r  e  b  a  l  l  w  o  w";
	checks[106] = "w  o  w  b  r  a  s  i  l  p  a";
	checks[107] = "f  a  t  a  l  i  t  y  w  o  w";
	checks[108] = "d  e  m  o  n  i  c  -  w  o  w";
	checks[109] = "r  e  v  e  n  g  e  -  w  o  w";
	checks[110] = "h  e  a  v  e  n  w  o  w";
	checks[111] = "u  n  d  e  a  d  -  w  o  w";
	checks[112] = "l  i  n  e  b  r";
	checks[113] = "a  z  r  a  l  o  n";
	checks[114] = "b  l  a  c  k  -  w  o  w";
	checks[115] = "t  r  e  v  o  n  w  o  w";

	checks[116] = " [The Lightbringer's Redemption]"; // old source code will crashed with this macro we cencured this
	checks[117] = "[The Lightbringer's Redemption]"; // old source code will crashed with this macro we cencured this

	for (int i = 0; i < cheksSize; ++i)
	{
		if (message.find(checks[i]) != string::npos)
		{
			std::string say = "";
			std::string str = "";
			say = message;
			sWorld->SendGMText(17000, player->GetName().c_str(), say.c_str()); // sned passive report to gm
			say = "";
			ChatHandler(player->GetSession()).PSendSysMessage("Links or Bad Words are not allowed on the server.");
			PreparedStatement* mt = LoginDatabase.GetPreparedStatement(LOGIN_UPD_MUTE_TIME);
			int64 muteTime = time(NULL) + mutetimecencure; // muted player if use bad words
			player->GetSession()->m_muteTime = muteTime;
			mt->setInt64(0, muteTime);
			return;
		}
	}

	string msg;
	ostringstream chat_string;

	if (player->GetTotalPlayedTime() <= Playedtimetochat) // New If - Played Time Need For Use This Cmd
	{
		std::string adStr = secsToTimeString(Playedtimetochat - player->GetTotalPlayedTime());
		player->GetSession()->SendNotification("You Must %s seconds played To use world chat!", adStr.c_str());
		return;
	}

	// if you have vip script can enable this option and will work eazy like amdwow
	/*switch (player->GetSession()->GetVipLevel()) // vip cases aded
	{
	case 1: // Vip Rank 1
	msg += "|cffffffff[VIP Bronze]";
	break;
	case 2: // Vip Rank 1
	msg += "|cffbbbbbb[VIP Silver]";
	break;
	case 3: // Vip Rank 1
	msg += "|cffff00ff[VIP Gold]";
	break;
	case 4: // Vip Rank 1
	msg += "|cffff6060[VIP Platinum]";
	break;
	case 5: // Vip Rank 1
	msg += "|cff0000ff[VIP Diamond]";
	break;
	case 6: // Vip Rank 1
	msg += "|cffff0000[VIP Warlord]";
	break;
	}*/
	switch (player->GetSession()->GetSecurity())
	{
		// Player
	case SEC_PLAYER:
		if (player->HasItemCount(ONE_CHARACTER_VIP, 1))
		{
			msg += "[|cFF9370DBEpic] ";
			msg += "|TInterface\\icons\\spell_fire_rune:13|t|r] ";
			msg += GetNameLink(player);
			msg += " |cffffff00";
		}
		else if (player->HasItemCount(ONE_CHARACTER_VIP_2, 1))
		{
			msg += "|cFFFFA500Legend ";
			msg += "|TInterface\\icons\\spell_fire_rune:13|t|r] ";
			msg += GetNameLink(player);
			msg += " |cffffff00";
		}
		else if (player->GetTeam() == ALLIANCE)
		{
			msg += "|cff00ff00[Monster ";
			msg += "|cff0000ff|TInterface\\pvpframe\\pvp-currency-alliance:13|t|r] ";
			msg += GetNameLink(player);
			msg += ":|cff00ff00";
		}
		else
		{
			msg += "|cff00ff00[Monster ";
			msg += "|cffff0000|TInterface\\pvpframe\\pvp-currency-horde:13|t|r] ";
			msg += GetNameLink(player);
			msg += ": |cff00ff00";
		}

		break;
		// Moderador - 2
	case SEC_MODERATOR:
		msg += "|cff4fdeff[Staff ";
		msg += "|cff4fdeffMonster]";
		msg += GetNameLink(player);
		msg += ": |cff4fdeff";
		break;
		// GM - 3
	case SEC_GAMEMASTER:
		msg += "|cff4fdeff[Staff ";
		msg += "|cff4fdeffMonster]";
		msg += GetNameLink(player);
		msg += ": |cff4fdeff";
		break;
	case SEC_GAMEMASTER_II:
		msg += "|cff4fdeff[Staff ";
		msg += "|cff4fdeffMonster]";
		msg += GetNameLink(player);
		msg += ": |cff4fdeff";
		break;
	case SEC_GAMEMASTER_III:
		msg += "|cff4fdeff[Staff ";
		msg += "|cff4fdeffMonster]";
		msg += GetNameLink(player);
		msg += ": |cff4fdeff";
		break;
	case SEC_GAMEMASTER_IV:
		msg += "|cff4fdeff[Staff ";
		msg += "|cff4fdeffMonster]";
		msg += GetNameLink(player);
		msg += ": |cff4fdeff";
		break;
	case SEC_GAMEMASTER_V:
		msg += "|cff4fdeff[Staff ";
		msg += "|cff4fdeffMonster]";
		msg += GetNameLink(player);
		msg += ": |cff4fdeff";
		break;
	case SEC_GAMEMASTER_VI:
		msg += "|cff4fdeff[Staff ";
		msg += "|cff4fdeffMonster]";
		msg += GetNameLink(player);
		msg += ": |cff4fdeff";
		break;
	case SEC_GAMEMASTER_VII:
		msg += "|cff4fdeff[Staff ";
		msg += "|cff4fdeffMonster]";
		msg += GetNameLink(player);
		msg += ": |cff4fdeff";
		break;
	case SEC_MODERATOR_II:
		msg += "|cff4fdeff[Staff ";
		msg += "|cff4fdeffMonster]";
		msg += GetNameLink(player);
		msg += ": |cff4fdeff";
		break;
		// Admin - 6
	case SEC_ADMINISTRATOR:
		msg += "|cff4fdeff[Staff ";
		msg += "|cff4fdeffMonster]";
		msg += GetNameLink(player);
		msg += ": |cff4fdeff";
		break;
		// Consola - 8
	case SEC_CONSOLE:
		msg += "|cff4fdeff[Staff ";
		msg += "|cff4fdeffMonster]";
		msg += GetNameLink(player);
		msg += ": |cff4fdeff";
		break;
				// Devoloper - 255
	case SEC_OWNER:
		msg += "|cFF006400[Monster ";
		msg += "|cFF8B0000Owner]";
		msg += GetNameLink(player);
		msg += ": |cff4fdeff";
		break;
	}

	chat_string << msg << " " << message;

	char c_msg[1024];

	snprintf(c_msg, 1024, chat_string.str().c_str());

	if (FACTION_SPECIFIC)
	{
		SessionMap sessions = sWorld->GetAllSessions();
		for (SessionMap::iterator itr = sessions.begin(); itr != sessions.end(); ++itr)
		if (Player* plr = itr->second->GetPlayer())
		if (plr->GetTeam() == player->GetTeam())
			sWorld->SendServerMessage(SERVER_MSG_STRING, msg.c_str(), plr);
	}
	else
		sWorld->SendGlobalText(c_msg, NULL);
}

class cs_world_chat : public CommandScript
{
public:
	cs_world_chat() : CommandScript("cs_world_chat") {}

	static bool HandleWorldChatCommand(ChatHandler* handler, const char* args)
	{
		if (!*args)
			return false;

		Player* player = handler->GetSession()->GetPlayer();

		_SendWorldChat(handler->GetSession()->GetPlayer(), args);
		// add mute time for stop spam
		PreparedStatement* mt = LoginDatabase.GetPreparedStatement(LOGIN_UPD_MUTE_TIME);
		int64 muteTime = time(NULL) + mutetimeantispam;
		player->GetSession()->m_muteTime = muteTime;
		mt->setInt64(0, muteTime);
		return true;
	}
	std::vector<ChatCommand> GetCommands() const override
	{
		static std::vector<ChatCommand> cs_world_chat =
		{
			{ "world", SEC_PLAYER, true, &HandleWorldChatCommand, "" },
		};

		return cs_world_chat;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class System_Censure : public PlayerScript
{
public:
	System_Censure() : PlayerScript("System_Censure") {}

	void OnChat(Player* player, uint32 /*type*/, uint32 lang, std::string& msg)
	{
		CheckMessage(player, msg, lang, NULL, NULL, NULL, NULL);
	}

	void OnChat(Player* player, uint32 /*type*/, uint32 lang, std::string& msg, Player* receiver)
	{

		CheckMessage(player, msg, lang, receiver, NULL, NULL, NULL);
	}

	void OnChat(Player* player, uint32 /*type*/, uint32 lang, std::string& msg, Group* group)
	{
		CheckMessage(player, msg, lang, NULL, group, NULL, NULL);
	}

	void OnChat(Player* player, uint32 /*type*/, uint32 lang, std::string& msg, Guild* guild)
	{
		CheckMessage(player, msg, lang, NULL, NULL, guild, NULL);
	}



	void CheckMessage(Player* player, std::string& msg, uint32 lang, Player* /*receiver*/, Group* /*group*/, Guild* /*guild*/, Channel* channel)
	{
		std::string lower = msg;
		std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

		QueryResult Result = LoginDatabase.Query("SELECT text FROM chat_filter");

		if (!Result)
			return;

		do
		{
			Field* Fields = Result->Fetch();
			std::string moton = Fields[0].GetString();

			if (lower.find(moton) != std::string::npos)
			{
				msg = "|cFFFF0000[!#$!#!$]";

				std::string str = "";
				SessionMap ss = sWorld->GetAllSessions();
				for (SessionMap::const_iterator itr = ss.begin(); itr != ss.end(); ++itr)
				if (itr->second->GetSecurity() > 0)
					str = "|cFFFF0000[ Anti Spam ]|cFF00FFFF [ |cFF60FF00" + std::string(player->GetName()) + "|cFF00FFFF ] [ |cFF60FF00" + lower + "|cFF00FFFF ] Check it!";
				WorldPacket data(SMSG_NOTIFICATION, (str.size() + 1));
				data << str;
				sWorld->SendGlobalGMMessage(&data);

				ChatHandler(player->GetSession()).PSendSysMessage("|cFFFF0000[ Anti Spam ] : The message published by you is contrary to the rules of the Server");
				return;
			}
		} while (Result->NextRow());
	}

	// added by lizard.tiny Anti Farm - Start
	void OnPVPKill(Player * killer, Player * killed)
	{
		if (killer->GetGUID() == killed->GetGUID())
		{
			return;
		}

		if (killer->GetSession()->GetRemoteAddress() == killed->GetSession()->GetRemoteAddress())
		{
			std::string str = "";
			SessionMap ss = sWorld->GetAllSessions();
			for (SessionMap::const_iterator itr = ss.begin(); itr != ss.end(); ++itr)
			if (itr->second->GetSecurity() > 0)
				str = "|cFFFFFF00[ Anti Farming ]|cFF00FFFF[ |cFF60FF00" + std::string(killer->GetName()) + "|cFF00FFFF ] Check it!";
			WorldPacket data(SMSG_NOTIFICATION, (str.size() + 1));
			data << str;
			sWorld->SendGlobalGMMessage(&data);
		}
	}
	// added by lizard.tiny Anti Farm - End

};

class TeleportPlayerOnLogOut : public PlayerScript
{
public:
	TeleportPlayerOnLogOut() : PlayerScript("TeleportPlayerOnLogOut") {}

	void OnLogout(Player* player)
	{
		Map const* map = player->IsInWorld() ? player->GetMap() : sMapMgr->FindMap(player->GetMapId(), player->GetInstanceId());
		if (map->IsRaid())
			player->GetSession()->KickPlayer("Posible debug raid");
	}
};

void AddSC_System_Censure()
{
	new System_Censure();
	new TeleportPlayerOnLogOut();
	new cs_world_chat;
}
