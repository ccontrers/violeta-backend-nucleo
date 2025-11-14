/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `objetivossucursales`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `objetivossucursales` (
  `sucursal` varchar(10) NOT NULL,
  `objetivovts_01` int(3) DEFAULT 0,
  `objetivovts_02` int(3) DEFAULT 0,
  `objetivovts_03` int(3) DEFAULT 0,
  `objetivovts_04` int(3) DEFAULT 0,
  `objetivovts_05` int(3) DEFAULT 0,
  `objetivovts_06` int(3) DEFAULT 0,
  `objetivovts_07` int(3) DEFAULT 0,
  `objetivovts_08` int(3) DEFAULT 0,
  `objetivovts_09` int(3) DEFAULT 0,
  `objetivovts_10` int(3) DEFAULT 0,
  `objetivovts_11` int(3) DEFAULT 0,
  `objetivovts_12` int(3) DEFAULT 0,
  `objetivoini_01` double(16,2) DEFAULT 0.00,
  `objetivoini_02` double(16,2) DEFAULT 0.00,
  `objetivoini_03` double(16,2) DEFAULT 0.00,
  `objetivoini_04` double(16,2) DEFAULT 0.00,
  `objetivoini_05` double(16,2) DEFAULT 0.00,
  `objetivoini_06` double(16,2) DEFAULT 0.00,
  `objetivoini_07` double(16,2) DEFAULT 0.00,
  `objetivoini_08` double(16,2) DEFAULT 0.00,
  `objetivoini_09` double(16,2) DEFAULT 0.00,
  `objetivoini_10` double(16,2) DEFAULT 0.00,
  `objetivoini_11` double(16,2) DEFAULT 0.00,
  `objetivoini_12` double(16,2) DEFAULT 0.00,
  `nuevo` int(1) DEFAULT 0,
  PRIMARY KEY (`sucursal`) USING BTREE,
  CONSTRAINT `objetivossucursal_ibfk_1` FOREIGN KEY (`sucursal`) REFERENCES `sucursales` (`sucursal`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

