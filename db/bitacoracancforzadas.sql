/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `bitacoracancforzadas`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `bitacoracancforzadas` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `usucanc` varchar(12) NOT NULL,
  `compfiscal` int(11) DEFAULT NULL,
  `fechacanc` date DEFAULT NULL,
  `horacanc` time DEFAULT NULL,
  PRIMARY KEY (`id`) USING BTREE,
  KEY `usucanc` (`usucanc`) USING BTREE,
  KEY `compfiscal` (`compfiscal`) USING BTREE,
  CONSTRAINT `bitacoracancforzadas_ibfk_1` FOREIGN KEY (`usucanc`) REFERENCES `empleados` (`empleado`) ON UPDATE CASCADE,
  CONSTRAINT `bitacoracancforzadas_ibfk_2` FOREIGN KEY (`compfiscal`) REFERENCES `cfd` (`compfiscal`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci ROW_FORMAT=DYNAMIC;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

