/*
* Copyright (C) 2008-2017 monsterCore <http://www.monstercore.org/>
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

// This is where scripts' loading functions should be declared:
// The name of this function should match:
// void Add${NameOfDirectory}Scripts()

/* This is where custom scripts' loading functions should be declared. */
//void AddSC_Skirmish_npc();
void AddSC_solo_queue();
void AddSC_SpellRegulator();
void AddSC_StabilityTest();
void AddSC_npc_level_booster();
void AddSC_Transmogrifier_NPC();
void AddSC_GMItems();
void AddSC_bug_dual();
void AddSC_System_Censure();
void AddSC_CdReset();
void AddSc_npc_tool();
void AddCustomScripts()
{
    /* This is where custom scripts should be added. */
    //AddSC_Skirmish_npc();
    AddSC_solo_queue();
    AddSC_SpellRegulator();
    AddSC_StabilityTest();
    AddSC_npc_level_booster();
	AddSC_Transmogrifier_NPC();
	AddSC_GMItems();
	AddSC_bug_dual();
	AddSC_System_Censure();
	AddSC_CdReset();
	AddSc_npc_tool();
}