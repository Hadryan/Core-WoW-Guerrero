DELETE FROM `creature` WHERE `id`=50062;
INSERT INTO `creature` (`guid`, `id`, `map`, `zone`, `area`, `spawnMask`, `phaseMask`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `walkmode`) VALUES 
(817686, 50062, 646, 5042, 5291, 1, 1, 0, 0, 691.109, 441.321, 159.918, 0, 259200, 0, 0, 774900, 0, 2, 0, 0, 0, 0);

DELETE FROM `creature_template` WHERE `entry`=50062;
INSERT INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `modelid1`, `modelid2`, `modelid3`, `modelid4`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `exp_unk`, `faction_a`, `faction_h`, `npcflag`, `speed_walk`, `speed_run`, `speed_swim`, `speed_fly`, `scale`, `rank`, `mindmg`, `maxdmg`, `dmgschool`, `attackpower`, `dmg_multiplier`, `baseattacktime`, `rangeattacktime`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `trainer_type`, `trainer_spell`, `trainer_class`, `trainer_race`, `minrangedmg`, `maxrangedmg`, `rangedattackpower`, `type`, `type_flags`, `type_flags2`, `lootid`, `pickpocketloot`, `skinloot`, `resistance1`, `resistance2`, `resistance3`, `resistance4`, `resistance5`, `resistance6`, `spell1`, `spell2`, `spell3`, `spell4`, `spell5`, `spell6`, `spell7`, `spell8`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `InhabitType`, `HoverHeight`, `Health_mod`, `Mana_mod`, `Mana_mod_extra`, `Armor_mod`, `RacialLeader`, `questItem1`, `questItem2`, `questItem3`, `questItem4`, `questItem5`, `questItem6`, `movementId`, `RegenHealth`, `equipment_id`, `mechanic_immune_mask`, `flags_extra`, `ScriptName`, `WDBVerified`, `ReactState`) VALUES('50062','0','0','0','0','0','37149','0','0','0','Aeonaxx','Mate of Aeosera','vehichleCursor','0','85','85','0','0','17','17','0','1','1.14286','1','0.857143','1','0','1000','2000','0','24','7.5','2000','2000','1','0','2048','2048','0','0','0','0','0','4','6','0','2','0','0','50062','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','1800','5850','','1','5','1','120','1','1','1','0','0','0','0','0','0','0','100','1','0','0','2','','12340','2');

DELETE FROM `creature_addon` WHERE `guid`=817686;
INSERT INTO `creature_addon` (`guid`, `path_id`, `mount`, `bytes1`, `bytes2`, `emote`, `auras`) VALUES 
(817686, 8176860, 0, 0, 0, 0, '');

DELETE FROM `waypoint_data` WHERE `id`=8176860;
INSERT INTO `waypoint_data` (`id`, `point`, `position_x`, `position_y`, `position_z`, `orientation`, `delay`, `move_type`, `action`, `action_chance`, `wpguid`) VALUES
(8176860, 1, 691.109253, 441.321045, 159.917511, 0, 0, 0, 0, 100, 0),
(8176860, 2, 734.466858, 335.121704, 151.883545, 0, 0, 0, 0, 100, 0),
(8176860, 3, 785.374207, 249.132980, 142.047119, 0, 0, 0, 0, 100, 0),
(8176860, 4, 917.375122, 206.658371, 98.268250, 0, 0, 0, 0, 100, 0),
(8176860, 5, 1032.703003, 204.170242, 65.425850, 0, 0, 0, 0, 100, 0),
(8176860, 6, 1125.020020, 239.496735, 53.314648, 0, 0, 0, 0, 100, 0),
(8176860, 7, 1228.294800, 289.709564, 49.963840, 0, 0, 0, 0, 100, 0),
(8176860, 8, 1261.522461, 323.307587, 52.345081, 0, 0, 0, 0, 100, 0),
(8176860, 9, 1277.443115, 399.509430, 79.511749, 0, 0, 0, 0, 100, 0),
(8176860, 10, 1284.742798, 447.079651, 99.989288, 0, 0, 0, 0, 100, 0),
(8176860, 11, 1286.424927, 560.721863, 91.720894, 0, 0, 0, 0, 100, 0),
(8176860, 12, 1281.497192, 641.266663, 70.514145, 0, 0, 0, 0, 100, 0),
(8176860, 13, 1273.012207, 708.533997, 56.190769, 0, 0, 0, 0, 100, 0),
(8176860, 14, 1221.520996, 756.301453, 58.341640, 0, 0, 0, 0, 100, 0),
(8176860, 15, 1147.755005, 807.564392, 79.985260, 0, 0, 0, 0, 100, 0),
(8176860, 16, 1080.575317, 851.238586, 79.550537, 0, 0, 0, 0, 100, 0),
(8176860, 17, 991.131470, 835.090698, 69.918159, 0, 0, 0, 0, 100, 0),
(8176860, 18, 918.481445, 780.010864, 65.417328, 0, 0, 0, 0, 100, 0),
(8176860, 19, 892.277954, 758.644409, 62.941914, 0, 0, 0, 0, 100, 0),
(8176860, 20, 817.113770, 705.887756, 52.722054, 0, 0, 0, 0, 100, 0),
(8176860, 21, 781.149597, 659.282410, 62.813545, 0, 0, 0, 0, 100, 0),
(8176860, 22, 770.083435, 607.540649, 94.355408, 0, 0, 0, 0, 100, 0),
(8176860, 23, 753.484253, 529.927979, 141.668213, 0, 0, 0, 0, 100, 0);
