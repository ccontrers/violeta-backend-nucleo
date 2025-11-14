/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `ventadirent`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ventadirent` (
  `id_ventaent` int(255) unsigned NOT NULL AUTO_INCREMENT,
  `referencia` varchar(11) DEFAULT NULL,
  `tipo` varchar(7) DEFAULT NULL,
  `cliente` varchar(11) DEFAULT NULL,
  `calle` varchar(60) DEFAULT NULL,
  `numext` varchar(10) DEFAULT NULL,
  `numint` varchar(10) DEFAULT NULL,
  `colonia` varchar(10) DEFAULT NULL,
  `cp` varchar(5) DEFAULT NULL,
  `ubicaciongis` point DEFAULT NULL,
  `referenciadom` varchar(60) DEFAULT NULL,
  `fechaalta` date NOT NULL DEFAULT '0000-00-00',
  `fechamodi` date NOT NULL DEFAULT '0000-00-00',
  PRIMARY KEY (`id_ventaent`),
  KEY `referencia` (`referencia`),
  KEY `ventacolonia_ibfk_2` (`colonia`),
  CONSTRAINT `ventacolonia_ibfk_2` FOREIGN KEY (`colonia`) REFERENCES `colonias` (`colonia`) ON UPDATE CASCADE,
  CONSTRAINT `ventadirent_ibfk_1` FOREIGN KEY (`referencia`) REFERENCES `ventas` (`referencia`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci CHECKSUM=1;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

