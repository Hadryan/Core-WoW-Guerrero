/*
SQLyog Ultimate v11.11 (64 bit)
MySQL - 5.5.9-log : Database - world_v15
*********************************************************************
*/

/*!40101 SET NAMES utf8 */;

/*!40101 SET SQL_MODE=''*/;

/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
CREATE DATABASE /*!32312 IF NOT EXISTS*/`world_v15` /*!40100 DEFAULT CHARACTER SET latin1 */;

USE `world_v15`;

/*Table structure for table `spellregulator` */

DROP TABLE IF EXISTS `spellregulator`;

CREATE TABLE `spellregulator` (
  `spellId` int(11) unsigned NOT NULL,
  `percentage` float NOT NULL DEFAULT '100',
  `comment` varchar(255) DEFAULT NULL,
  PRIMARY KEY (`spellId`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

/*Data for the table `spellregulator` */

insert  into `spellregulator`(`spellId`,`percentage`,`comment`) values (49020,60,'DK - Asolar'),(84839,20,'vengance - paly');

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;
