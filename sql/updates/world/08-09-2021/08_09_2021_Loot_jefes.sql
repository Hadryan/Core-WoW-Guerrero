UPDATE `creature_template` SET  `lootid`='39605' WHERE entry IN (39605);
UPDATE `creature_template` SET  `lootid`='29611' WHERE entry IN (29611);
UPDATE `creature_template` SET  `lootid`='36648' WHERE entry IN (36648);
UPDATE `creature_template` SET  `lootid`='42928' WHERE entry IN (42928);
UPDATE `creature_template` SET  `lootid`='10181' WHERE entry IN (10181);
UPDATE `creature_template` SET  `lootid`='7999' WHERE entry IN (7999);
UPDATE `creature_template` SET  `lootid`='16802' WHERE entry IN (16802);
UPDATE `creature_template` SET  `lootid`='17468' WHERE entry IN (17468);

DELETE FROM `creature_loot_template` WHERE entry IN (39605,29611,36648,42928,10181,7999,16802,17468);
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `lootmode`, `groupid`, `mincountOrRef`, `maxcount`) VALUES 
(36648, 30609, 100, 1, 0, 1, 1),
(42928, 30609, 100, 1, 0, 1, 1),
(10181, 34092, 100, 1, 0, 1, 1),
(7999, 34092, 100, 1, 0, 1, 1),
(29611, 37676, 100, 1, 0, 1, 1),
(39605, 37676, 100, 1, 0, 1, 1),
(16802, 43516, 100, 1, 0, 1, 1),
(17468, 43516, 100, 1, 0, 1, 1);