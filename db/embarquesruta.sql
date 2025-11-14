/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `embarquesruta`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `embarquesruta` (
  `idorden` int(11) NOT NULL DEFAULT 1,
  `embarque` varchar(11) NOT NULL DEFAULT '',
  `cliente` varchar(11) NOT NULL DEFAULT '',
  `calle` varchar(60) DEFAULT NULL,
  `numext` varchar(10) DEFAULT NULL,
  `numint` varchar(10) DEFAULT NULL,
  `referenciadomic` varchar(60) DEFAULT NULL,
  `colonia` varchar(10) DEFAULT NULL,
  `cp` varchar(5) DEFAULT NULL,
  `ubicaciongis` point DEFAULT NULL,
  `fechaalta` date NOT NULL DEFAULT '0000-00-00',
  `fechadellegada` date DEFAULT '0000-00-00',
  `horadellegada` time DEFAULT '00:00:00',
  `choferllega` varchar(10) DEFAULT NULL,
  `ubicaciondellegada` point DEFAULT NULL,
  `fechadesalida` date DEFAULT '0000-00-00',
  `horadesalida` time DEFAULT '00:00:00',
  `chofersale` varchar(10) DEFAULT NULL,
  `ubicaciondesalida` point DEFAULT NULL,
  `observacioneschofer` varchar(80) DEFAULT NULL,
  `etapa` tinyint(1) DEFAULT 0,
  `mensaje` varchar(128) DEFAULT '',
  PRIMARY KEY (`embarque`,`cliente`,`idorden`),
  KEY `embarquesruta_ibfk_1` (`cliente`),
  KEY `embarquesruta_ibfk_2` (`colonia`),
  CONSTRAINT `embarquesruta_ibfk_1` FOREIGN KEY (`cliente`) REFERENCES `clientes` (`cliente`) ON UPDATE CASCADE,
  CONSTRAINT `embarquesruta_ibfk_2` FOREIGN KEY (`colonia`) REFERENCES `colonias` (`colonia`) ON UPDATE CASCADE,
  CONSTRAINT `embarquesruta_ibfk_3` FOREIGN KEY (`embarque`) REFERENCES `embarques` (`embarque`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

