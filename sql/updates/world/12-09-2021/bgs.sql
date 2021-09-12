/*Table structure for table `battleground_template` */

DROP TABLE IF EXISTS `battleground_template`;

CREATE TABLE `battleground_template` (
  `id` mediumint(8) unsigned NOT NULL,
  `MinPlayersPerTeam` smallint(5) unsigned NOT NULL DEFAULT '0',
  `MaxPlayersPerTeam` smallint(5) unsigned NOT NULL DEFAULT '0',
  `MinLvl` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `MaxLvl` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `AllianceStartLoc` mediumint(8) unsigned NOT NULL,
  `AllianceStartO` float NOT NULL,
  `HordeStartLoc` mediumint(8) unsigned NOT NULL,
  `HordeStartO` float NOT NULL,
  `StartMaxDist` float NOT NULL DEFAULT '0',
  `Weight` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `ScriptName` char(64) NOT NULL DEFAULT '',
  `Comment` char(32) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

/*Data for the table `battleground_template` */

insert  into `battleground_template`(`id`,`MinPlayersPerTeam`,`MaxPlayersPerTeam`,`MinLvl`,`MaxLvl`,`AllianceStartLoc`,`AllianceStartO`,`HordeStartLoc`,`HordeStartO`,`StartMaxDist`,`Weight`,`ScriptName`,`Comment`) values (108,3,10,85,85,1726,2.53684,1727,6.27533,55,6,'','Twin Peaks'),(100,10,10,85,85,0,0,0,0,75,1,'','Rated Battleground 10 vs 10'),(32,3,40,45,85,0,0,0,0,0,6,'','Random battleground'),(9,3,15,65,85,1367,0,1368,0,0,6,'','Strand of the Ancients'),(30,3,40,71,85,1485,0,1486,3.16124,200,0,'','Isle of Conquest'),(11,0,5,10,85,1364,0,1365,0,0,3,'','The Ring of Valor'),(8,0,5,10,85,1258,0,1259,3.14159,0,3,'','Ruins of Lordaeron'),(7,3,15,35,85,1103,3.03123,1104,0.055761,75,6,'','Eye of The Storm'),(6,0,5,10,85,0,0,0,0,0,3,'','All Arena'),(5,0,5,10,85,939,0,940,3.14159,0,3,'','Blades\'s Edge Arena'),(4,0,5,10,85,929,0,936,3.14159,0,3,'','Nagrand Arena'),(3,3,15,10,85,890,3.91571,889,0.813671,75,6,'','Arathi Basin'),(10,0,5,10,85,1362,0,1363,3.14159,0,3,'','Dalaran Sewers'),(2,3,10,10,85,769,3.14159,770,0.151581,75,6,'','Warsong Gulch'),(1,3,40,85,85,611,3.16312,610,0.715504,100,1,'','Alterac Valley'),(120,3,10,81,85,1740,0,1739,0,75,6,'','Battle for Gilneas');
