DELETE FROM `creature_template` WHERE entry = 120031;
INSERT INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `modelid1`, `modelid2`, `modelid3`, `modelid4`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `exp_unk`, `faction_a`, `faction_h`, `npcflag`, `speed_walk`, `speed_run`, `speed_swim`, `speed_fly`, `scale`, `rank`, `mindmg`, `maxdmg`, `dmgschool`, `attackpower`, `dmg_multiplier`, `baseattacktime`, `rangeattacktime`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `trainer_type`, `trainer_spell`, `trainer_class`, `trainer_race`, `minrangedmg`, `maxrangedmg`, `rangedattackpower`, `type`, `type_flags`, `type_flags2`, `lootid`, `pickpocketloot`, `skinloot`, `resistance1`, `resistance2`, `resistance3`, `resistance4`, `resistance5`, `resistance6`, `spell1`, `spell2`, `spell3`, `spell4`, `spell5`, `spell6`, `spell7`, `spell8`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `InhabitType`, `HoverHeight`, `Health_mod`, `Mana_mod`, `Mana_mod_extra`, `Armor_mod`, `RacialLeader`, `questItem1`, `questItem2`, `questItem3`, `questItem4`, `questItem5`, `questItem6`, `movementId`, `RegenHealth`, `equipment_id`, `mechanic_immune_mask`, `flags_extra`, `ScriptName`, `WDBVerified`, `ReactState`) VALUES('120031','0','0','0','0','0','38804','0','0','0','Transfigurador','Monster-WoW','Transmogrify','0','85','85','3','0','35','35','1','1','1.14286','1','1.14286','1.5','3','12','15.525','0','48','0.75','2000','0','1','0','2048','8','0','0','0','0','0','6.6','11.475','32','7','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','','0','3','0','0.0238095','0','1','1','0','0','0','0','0','0','0','0','1','0','0','0','Transmogrifier_NPC','1','2');

DELETE FROM `creature` WHERE id = 120031;

UPDATE `creature` SET  `id`='120031' WHERE id = 54442;
UPDATE `creature` SET  `id`='120031' WHERE id = 54473;

DELETE FROM `creature` WHERE id = 54442;
DELETE FROM `creature` WHERE id = 54473;

INSERT INTO `creature` (`guid`, `id`, `map`, `zone`, `area`, `spawnMask`, `phaseMask`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `walkmode`, `saiscriptflag`) VALUES('75257','120031','1','1637','5167','1','1','0','0','1727.33','-4519.5','32.6442','1.29154','90','0','0','1','0','0','0','0','0','0','0');
INSERT INTO `creature` (`guid`, `id`, `map`, `zone`, `area`, `spawnMask`, `phaseMask`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `walkmode`, `saiscriptflag`) VALUES('39404','120031','0','1519','5704','1','1','0','0','-8699.7','838.564','99.2015','2.1293','90','0','0','1','0','0','0','0','0','0','0');
