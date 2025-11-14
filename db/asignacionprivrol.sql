/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `asignacionprivrol`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `asignacionprivrol` (
  `rol` varchar(10) NOT NULL DEFAULT '',
  `objeto` varchar(10) NOT NULL DEFAULT '',
  `privilegio` varchar(3) NOT NULL DEFAULT '',
  PRIMARY KEY (`rol`,`objeto`,`privilegio`) USING BTREE,
  KEY `objeto` (`objeto`) USING BTREE,
  KEY `FK_asignacionprivrol_2` (`privilegio`,`objeto`) USING BTREE,
  CONSTRAINT `FK_asignacionprivrol_2` FOREIGN KEY (`privilegio`, `objeto`) REFERENCES `privilegios` (`privilegio`, `objeto`) ON UPDATE CASCADE,
  CONSTRAINT `asignacionprivrol_ibfk_1` FOREIGN KEY (`rol`) REFERENCES `rolessistema` (`claverol`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

