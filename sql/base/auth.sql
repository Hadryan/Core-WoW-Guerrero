/*
SQLyog Ultimate v11.11 (64 bit)
MySQL - 5.5.9-log : Database - emucoach_v14_vip_auth
*********************************************************************
*/

/*!40101 SET NAMES utf8 */;

/*!40101 SET SQL_MODE=''*/;

/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
/*Table structure for table `account` */

DROP TABLE IF EXISTS `account`;

CREATE TABLE `account` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT 'Identifier',
  `username` varchar(32) NOT NULL DEFAULT '',
  `sha_pass_hash` varchar(40) NOT NULL DEFAULT '',
  `sessionkey` varchar(80) NOT NULL DEFAULT '',
  `v` varchar(64) NOT NULL DEFAULT '',
  `s` varchar(64) NOT NULL DEFAULT '',
  `token_key` varchar(100) NOT NULL DEFAULT '',
  `email` varchar(254) NOT NULL DEFAULT '',
  `joindate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `last_ip` varchar(15) NOT NULL DEFAULT '127.0.0.1',
  `failed_logins` int(10) unsigned NOT NULL DEFAULT '0',
  `locked` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `last_login` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
  `online` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `expansion` tinyint(3) unsigned NOT NULL DEFAULT '3',
  `mutetime` bigint(20) NOT NULL DEFAULT '0',
  `mutereason` varchar(255) NOT NULL,
  `muteby` varchar(50) DEFAULT NULL,
  `locale` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `os` varchar(3) NOT NULL DEFAULT '',
  `recruiter` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `idx_username` (`username`)
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=utf8 COMMENT='Account System';

/*Data for the table `account` */

insert  into `account`(`id`,`username`,`sha_pass_hash`,`sessionkey`,`v`,`s`,`token_key`,`email`,`joindate`,`last_ip`,`failed_logins`,`locked`,`last_login`,`online`,`expansion`,`mutetime`,`mutereason`,`muteby`,`locale`,`os`,`recruiter`) values (1,'AHBOT','10D5841C9147072BB966BF4E3F237C9A9D3AD2D1','3A836A086440EFE5ADFB6063D43FC92B01DF0536D66B7F9B3B5635BE73A777B802AB94DCEA2E0979','3B19322E76D8D2CF79D23543B02F8B34FC98FD4A6C7DBE96B20AD67D93947A8C','BE42373AE2D372C7533E0EA1FE4F007348B8E17B2A6ED57CE7B2D2FAB4AF7613','','','2016-07-11 02:28:36','0',0,0,'2016-07-11 02:33:11',0,3,0,'',NULL,0,'Win',0),(2,'ADMIN','8301316D0D8448A34FA6D0C6BF1CBFA2B4A1A93A','D33B474204AB714F7C2DFB136D273A772597160121491D3E28BE1955F7F2B7EE2B77771EF195D0C9','5FBA6F854BC5CD8A62C36FDD977628EE3A368E08E19FAABAB274EF64575AF323','E0E30595B061BBC9BCAF79510A02299AFE3CDD7BD1ED3C7ECC7A131F8B5CBE3B','','','2018-05-19 12:42:24','127.0.0.1',0,0,'2021-09-06 10:16:48',1,3,0,'',NULL,6,'Win',0),(3,'1','3E774731D33D9224AC36AF3D85BA1F81B31BC84D','630AAFB807971E0054B347132FE0E4719F794E62AED4B8F953C0130A8F9216058E59DA0AAE74D9E2','55D25C053A27E0AFA9752C3B3A0954446766A55F3006BB8CEA78A21BC66C9EAA','D9DA7EAB5223FCE56F14F7E344B0EB2C664DFAD231FA21E7D30A3DA96FDB3821','','','2019-06-30 04:48:48','127.0.0.1',0,0,'2021-04-02 16:42:40',0,3,0,'',NULL,0,'Win',0);

/*Table structure for table `account_access` */

DROP TABLE IF EXISTS `account_access`;

CREATE TABLE `account_access` (
  `id` int(10) unsigned NOT NULL,
  `gmlevel` tinyint(3) unsigned NOT NULL,
  `RealmID` int(11) NOT NULL DEFAULT '-1',
  `Comment` text,
  PRIMARY KEY (`id`,`RealmID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/*Data for the table `account_access` */

insert  into `account_access`(`id`,`gmlevel`,`RealmID`,`Comment`) values (2,4,-1,NULL);

/*Table structure for table `account_banned` */

DROP TABLE IF EXISTS `account_banned`;

CREATE TABLE `account_banned` (
  `id` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Account id',
  `bandate` int(10) unsigned NOT NULL DEFAULT '0',
  `unbandate` int(10) unsigned NOT NULL DEFAULT '0',
  `bannedby` varchar(50) DEFAULT NULL,
  `banreason` varchar(255) NOT NULL,
  `active` tinyint(3) unsigned NOT NULL DEFAULT '1',
  PRIMARY KEY (`id`,`bandate`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Ban List';

/*Data for the table `account_banned` */

/*Table structure for table `account_ip_history` */

DROP TABLE IF EXISTS `account_ip_history`;

CREATE TABLE `account_ip_history` (
  `id` int(10) DEFAULT NULL,
  `RealmID` int(10) DEFAULT NULL,
  `IP` varchar(15) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*Data for the table `account_ip_history` */

/*Table structure for table `account_premium` */

DROP TABLE IF EXISTS `account_premium`;

CREATE TABLE `account_premium` (
  `id` int(11) NOT NULL DEFAULT '0' COMMENT 'Account id',
  `setdate` bigint(40) NOT NULL DEFAULT '0',
  `unsetdate` bigint(40) NOT NULL DEFAULT '0',
  `premium_type` tinyint(4) unsigned NOT NULL DEFAULT '1',
  `active` tinyint(4) NOT NULL DEFAULT '1',
  PRIMARY KEY (`id`,`setdate`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Premium Accounts';

/*Data for the table `account_premium` */

/*Table structure for table `account_punishment` */

DROP TABLE IF EXISTS `account_punishment`;

CREATE TABLE `account_punishment` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `account` int(11) unsigned NOT NULL,
  `by` int(11) NOT NULL,
  `penalty_points` int(11) unsigned NOT NULL,
  `comment` varchar(255) NOT NULL,
  `time` bigint(20) unsigned NOT NULL,
  `active` tinyint(3) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=99 DEFAULT CHARSET=utf8;

/*Data for the table `account_punishment` */

/*Table structure for table `account_tempban` */

DROP TABLE IF EXISTS `account_tempban`;

CREATE TABLE `account_tempban` (
  `accountId` int(11) NOT NULL,
  `reason` varchar(255) NOT NULL,
  PRIMARY KEY (`accountId`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

/*Data for the table `account_tempban` */

/*Table structure for table `account_warning` */

DROP TABLE IF EXISTS `account_warning`;

CREATE TABLE `account_warning` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `accountId` int(11) unsigned NOT NULL,
  `by_account` int(11) unsigned NOT NULL,
  `comment` varchar(255) NOT NULL,
  `time` bigint(20) unsigned NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=169 DEFAULT CHARSET=utf8;

/*Data for the table `account_warning` */

/*Table structure for table `accounts_to_delete` */

DROP TABLE IF EXISTS `accounts_to_delete`;

CREATE TABLE `accounts_to_delete` (
  `id` int(11) unsigned NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Account System';

/*Data for the table `accounts_to_delete` */

/*Table structure for table `autobroadcast` */

DROP TABLE IF EXISTS `autobroadcast`;

CREATE TABLE `autobroadcast` (
  `id` int(10) DEFAULT NULL,
  `weight` int(10) DEFAULT NULL,
  `text` longtext,
  `text_1` longtext,
  `text_2` longtext,
  `text_3` longtext,
  `text_4` longtext,
  `text_5` longtext,
  `text_6` longtext,
  `text_7` longtext,
  `RealmID` int(11) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*Data for the table `autobroadcast` */

insert  into `autobroadcast`(`id`,`weight`,`text`,`text_1`,`text_2`,`text_3`,`text_4`,`text_5`,`text_6`,`text_7`,`RealmID`) values (1,1,'Welcome to the EmuCoach Official Repack - v14.0 - VIP Blizzlike! Make sure to visit our forum - |cffDBFBFFhttp://www.emucoach.com',NULL,NULL,NULL,NULL,NULL,NULL,NULL,1),(2,1,'The EmuCoach team is working daily on improving the repack, check www.emucoach.com for further updates.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,1),(3,2,'This is the newest VIP Version that is available on EmuCoach. Thank you for supporting us.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,1),(4,3,'You can edit these messages in your auth database -> Autobroadcast. Best Regards - EmuCoach.com',NULL,NULL,NULL,NULL,NULL,NULL,NULL,1);

/*Table structure for table `bug_tickets` */

DROP TABLE IF EXISTS `bug_tickets`;

CREATE TABLE `bug_tickets` (
  `ticketId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `realm` int(10) unsigned NOT NULL DEFAULT '0',
  `guid` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Global Unique Identifier of ticket creator',
  `message` text NOT NULL,
  `createTime` int(10) unsigned NOT NULL DEFAULT '0',
  `pool` varchar(32) NOT NULL,
  `mapId` smallint(5) unsigned NOT NULL DEFAULT '0',
  `posX` float NOT NULL DEFAULT '0',
  `posY` float NOT NULL DEFAULT '0',
  `posZ` float NOT NULL DEFAULT '0',
  `closedBy` int(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`ticketId`)
) ENGINE=MyISAM AUTO_INCREMENT=313 DEFAULT CHARSET=utf8 COMMENT='Support System';

/*Data for the table `bug_tickets` */

/*Table structure for table `ip2nation` */

DROP TABLE IF EXISTS `ip2nation`;

CREATE TABLE `ip2nation` (
  `ip` int(11) unsigned NOT NULL DEFAULT '0',
  `country` varchar(2) NOT NULL DEFAULT '',
  KEY `ip` (`ip`) USING BTREE
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

/*Data for the table `ip2nation` */

/*Table structure for table `ip2nationcountries` */

DROP TABLE IF EXISTS `ip2nationcountries`;

CREATE TABLE `ip2nationcountries` (
  `code` varchar(2) NOT NULL DEFAULT '',
  `iso_code_2` varchar(2) NOT NULL DEFAULT '',
  `iso_code_3` varchar(3) DEFAULT '',
  `iso_country` varchar(255) NOT NULL DEFAULT '',
  `country` varchar(255) NOT NULL DEFAULT '',
  `lat` float NOT NULL DEFAULT '0',
  `lon` float NOT NULL DEFAULT '0',
  PRIMARY KEY (`code`),
  KEY `code` (`code`) USING BTREE
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

/*Data for the table `ip2nationcountries` */

/*Table structure for table `ip_banned` */

DROP TABLE IF EXISTS `ip_banned`;

CREATE TABLE `ip_banned` (
  `ip` varchar(15) NOT NULL DEFAULT '127.0.0.1',
  `bandate` int(10) unsigned NOT NULL,
  `unbandate` int(10) unsigned NOT NULL,
  `bannedby` varchar(50) NOT NULL DEFAULT '[Console]',
  `banreason` varchar(255) NOT NULL DEFAULT 'no reason',
  PRIMARY KEY (`ip`,`bandate`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Banned IPs';

/*Data for the table `ip_banned` */

/*Table structure for table `logs` */

DROP TABLE IF EXISTS `logs`;

CREATE TABLE `logs` (
  `time` int(10) unsigned NOT NULL,
  `realm` int(10) unsigned NOT NULL,
  `type` tinyint(3) unsigned NOT NULL,
  `level` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `string` text CHARACTER SET latin1
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/*Data for the table `logs` */

/*Table structure for table `rbac_account_groups` */

DROP TABLE IF EXISTS `rbac_account_groups`;

CREATE TABLE `rbac_account_groups` (
  `accountId` int(10) unsigned NOT NULL COMMENT 'Account id',
  `groupId` int(10) unsigned NOT NULL COMMENT 'Group id',
  `realmId` int(11) NOT NULL DEFAULT '-1' COMMENT 'Realm Id, -1 means all',
  PRIMARY KEY (`accountId`,`groupId`,`realmId`),
  KEY `fk__rbac_account_groups__rbac_groups` (`groupId`) USING BTREE
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Account-Group relation';

/*Data for the table `rbac_account_groups` */

/*Table structure for table `rbac_account_permissions` */

DROP TABLE IF EXISTS `rbac_account_permissions`;

CREATE TABLE `rbac_account_permissions` (
  `accountId` int(10) unsigned NOT NULL COMMENT 'Account id',
  `permissionId` int(10) unsigned NOT NULL COMMENT 'Permission id',
  `granted` tinyint(1) NOT NULL DEFAULT '1' COMMENT 'Granted = 1, Denied = 0',
  `realmId` int(11) NOT NULL DEFAULT '-1' COMMENT 'Realm Id, -1 means all',
  PRIMARY KEY (`accountId`,`permissionId`,`realmId`),
  KEY `fk__rbac_account_roles__rbac_permissions` (`permissionId`) USING BTREE
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Account-Permission relation';

/*Data for the table `rbac_account_permissions` */

/*Table structure for table `rbac_account_roles` */

DROP TABLE IF EXISTS `rbac_account_roles`;

CREATE TABLE `rbac_account_roles` (
  `accountId` int(10) unsigned NOT NULL COMMENT 'Account id',
  `roleId` int(10) unsigned NOT NULL COMMENT 'Role id',
  `granted` tinyint(1) NOT NULL DEFAULT '1' COMMENT 'Granted = 1, Denied = 0',
  `realmId` int(11) NOT NULL DEFAULT '-1' COMMENT 'Realm Id, -1 means all',
  PRIMARY KEY (`accountId`,`roleId`,`realmId`),
  KEY `fk__rbac_account_roles__rbac_roles` (`roleId`) USING BTREE
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Account-Role relation';

/*Data for the table `rbac_account_roles` */

/*Table structure for table `rbac_group_roles` */

DROP TABLE IF EXISTS `rbac_group_roles`;

CREATE TABLE `rbac_group_roles` (
  `groupId` int(10) unsigned NOT NULL COMMENT 'group id',
  `roleId` int(10) unsigned NOT NULL COMMENT 'Role id',
  PRIMARY KEY (`groupId`,`roleId`),
  KEY `fk__rbac_group_roles__rbac_roles` (`roleId`) USING BTREE
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Group Role relation';

/*Data for the table `rbac_group_roles` */

/*Table structure for table `rbac_groups` */

DROP TABLE IF EXISTS `rbac_groups`;

CREATE TABLE `rbac_groups` (
  `id` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Group id',
  `name` varchar(100) NOT NULL COMMENT 'Group name',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Group List';

/*Data for the table `rbac_groups` */

/*Table structure for table `rbac_permissions` */

DROP TABLE IF EXISTS `rbac_permissions`;

CREATE TABLE `rbac_permissions` (
  `id` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Permission id',
  `name` varchar(100) NOT NULL COMMENT 'Permission name',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Permission List';

/*Data for the table `rbac_permissions` */

/*Table structure for table `rbac_role_permissions` */

DROP TABLE IF EXISTS `rbac_role_permissions`;

CREATE TABLE `rbac_role_permissions` (
  `roleId` int(10) unsigned NOT NULL COMMENT 'Role id',
  `permissionId` int(10) unsigned NOT NULL COMMENT 'Permission id',
  PRIMARY KEY (`roleId`,`permissionId`),
  KEY `fk__role_permissions__rbac_permissions` (`permissionId`) USING BTREE
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Role Permission relation';

/*Data for the table `rbac_role_permissions` */

/*Table structure for table `rbac_roles` */

DROP TABLE IF EXISTS `rbac_roles`;

CREATE TABLE `rbac_roles` (
  `id` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Role id',
  `name` varchar(100) NOT NULL COMMENT 'Role name',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Roles List';

/*Data for the table `rbac_roles` */

/*Table structure for table `rbac_security_level_groups` */

DROP TABLE IF EXISTS `rbac_security_level_groups`;

CREATE TABLE `rbac_security_level_groups` (
  `secId` tinyint(3) unsigned NOT NULL COMMENT 'Security Level id',
  `groupId` int(10) unsigned NOT NULL COMMENT 'group id',
  PRIMARY KEY (`secId`,`groupId`),
  KEY `fk__rbac_security_level_groups__rbac_groups` (`groupId`) USING BTREE
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Default groups to assign when an account is set gm level';

/*Data for the table `rbac_security_level_groups` */

/*Table structure for table `realmcharacters` */

DROP TABLE IF EXISTS `realmcharacters`;

CREATE TABLE `realmcharacters` (
  `realmid` int(10) unsigned NOT NULL DEFAULT '0',
  `acctid` int(10) unsigned NOT NULL,
  `numchars` tinyint(3) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`realmid`,`acctid`),
  KEY `acctid` (`acctid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Realm Character Tracker';

/*Data for the table `realmcharacters` */

insert  into `realmcharacters`(`realmid`,`acctid`,`numchars`) values (1,1,1),(1,2,1),(1,3,1);

/*Table structure for table `realmlist` */

DROP TABLE IF EXISTS `realmlist`;

CREATE TABLE `realmlist` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(32) NOT NULL DEFAULT '',
  `address` varchar(255) NOT NULL DEFAULT '127.0.0.1',
  `localAddress` varchar(255) NOT NULL DEFAULT '127.0.0.1',
  `localSubnetMask` varchar(255) NOT NULL DEFAULT '255.255.255.0',
  `port` smallint(5) unsigned NOT NULL DEFAULT '8085',
  `icon` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `flag` tinyint(3) unsigned NOT NULL DEFAULT '2',
  `timezone` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `allowedSecurityLevel` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `population` float unsigned NOT NULL DEFAULT '0',
  `gamebuild` int(10) unsigned NOT NULL DEFAULT '15595',
  PRIMARY KEY (`id`),
  UNIQUE KEY `idx_name` (`name`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8 COMMENT='Realm System';

/*Data for the table `realmlist` */

insert  into `realmlist`(`id`,`name`,`address`,`localAddress`,`localSubnetMask`,`port`,`icon`,`flag`,`timezone`,`allowedSecurityLevel`,`population`,`gamebuild`) values (1,'Emucoach - V14 VIP','127.0.0.1','127.0.0.1','255.255.255.0',8085,1,0,1,0,0,15595);

/*Table structure for table `uptime` */

DROP TABLE IF EXISTS `uptime`;

CREATE TABLE `uptime` (
  `realmid` int(10) unsigned NOT NULL,
  `starttime` int(10) unsigned NOT NULL DEFAULT '0',
  `uptime` int(10) unsigned NOT NULL DEFAULT '0',
  `maxplayers` smallint(5) unsigned NOT NULL DEFAULT '0',
  `revision` varchar(255) NOT NULL DEFAULT 'monstercore',
  PRIMARY KEY (`realmid`,`starttime`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Uptime system';

/*Data for the table `uptime` */

insert  into `uptime`(`realmid`,`starttime`,`uptime`,`maxplayers`,`revision`) values (1,1617399729,1201,1,'monster rev. 23bf871b7274+ 2021-04-01 21:33:20 +0200 (master branch) (Win64, Release)'),(1,1630852559,0,0,'monster rev. 31-08-2021 (Emucore-DL) (Win64, Release)'),(1,1630853047,600,1,'monster rev. 31-08-2021 (Emucore-DL) (Win64, Release)'),(1,1630938840,1347,1,'monster rev. 0000-00-00 00:00:00 +0000 (Archived) (Win64, Release)'),(1,1630940754,0,0,'monster rev. 0000-00-00 00:00:00 +0000 (Archived) (Win64, Release)'),(1,1630940924,0,0,'monster rev. 0000-00-00 00:00:00 +0000 (Archived) (Win64, Release)'),(1,1630941401,0,0,'monster rev. 0000-00-00 00:00:00 +0000 (Archived) (Win64, Release)');

/*Table structure for table `vip_accounts` */

DROP TABLE IF EXISTS `vip_accounts`;

CREATE TABLE `vip_accounts` (
  `id` int(11) NOT NULL DEFAULT '0' COMMENT 'Account id',
  `type` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `setdate` bigint(40) NOT NULL DEFAULT '0',
  `unsetdate` bigint(40) NOT NULL DEFAULT '0',
  `active` tinyint(4) NOT NULL DEFAULT '1',
  PRIMARY KEY (`id`,`setdate`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='VIP Accounts';

/*Data for the table `vip_accounts` */

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;
