delete from `gossip_menu_option` where menu_id = 50006;

insert into `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `OptionBroadcastTextID`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `box_coded`, `box_money`, `box_text`, `BoxBroadcastTextID`) values('50006','1','2','|TInterface/ICONS/Achievment_boss_madnessofdeathwing:35:35|t Dragon Soul','0','1','1','0','0','0','0','Are you sure, that you want to go to Dragon Soul?','0');
insert into `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `OptionBroadcastTextID`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `box_coded`, `box_money`, `box_text`, `BoxBroadcastTextID`) values('50006','2','2','|TInterface/ICONS/Achievement_boss_lordanthricyst:35:35|t Firelands','0','1','1','0','0','0','0','Are you sure, that you want to go to Fireland?','0');
insert into `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `OptionBroadcastTextID`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `box_coded`, `box_money`, `box_text`, `BoxBroadcastTextID`) values('50006','3','2','|TInterface/ICONS/achievement_zone_tolbarad:35:35|t Bastion Hold','0','1','1','0','0','0','0','Are you sure, that you want to go to Baradin Hold?','0');
insert into `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `OptionBroadcastTextID`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `box_coded`, `box_money`, `box_text`, `BoxBroadcastTextID`) values('50006','4','2','|TInterface/ICONS/achievement_reputation_ogre:35:35|t The Bastion of Twilight','0','1','1','0','0','0','0','Are you sure, that you want to go to The Bastion of Twilight?','0');
insert into `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `OptionBroadcastTextID`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `box_coded`, `box_money`, `box_text`, `BoxBroadcastTextID`) values('50006','5','2','|TInterface/ICONS/achievement_boss_nefarion:35:35|t Blackwing Descent','0','1','1','0','0','0','0','Blackwing Descent','0');
insert into `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `OptionBroadcastTextID`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `box_coded`, `box_money`, `box_text`, `BoxBroadcastTextID`) values('50006','6','0','|TInterface/ICONS/Mail_GMIcon:35:35|t<<Back>>','0','1','1','58000','0','0','0',NULL,'0');

delete from `smart_scripts` where entryorguid = 85002;

insert into `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) values('85002','0','8','0','62','0','100','0','50006','1','0','0','62','1','0','0','0','0','0','7','1','440','2300','-8289.11','-4525.5','-219.763','0.358896','teleport to Dragon Soul');
insert into `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) values('85002','0','9','0','62','0','100','0','50006','2','0','0','62','1','0','0','0','0','0','7','1','616','5039','3992.71','-2960.73','1002.55','2.11884','teleport to Fireland');
insert into `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) values('85002','0','10','0','62','0','100','0','50006','3','0','0','62','732','0','0','0','0','0','7','732','5095','5399','-1262.09','1049.36','106.996','3.1918','teleport to Baradin');
insert into `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) values('85002','0','11','0','62','0','100','0','50006','4','0','0','62','0','0','0','0','0','0','7','0','4922','5473','-4887.24','-4251.23','827.763','2.1767','teleport to Crepusculo');
insert into `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) values('85002','0','12','0','62','0','100','0','50006','5','0','0','62','0','0','0','0','0','0','7','0','25','5729','-7534','-1210.67','477.729','2.00286','teleport to descenso de alanegra');