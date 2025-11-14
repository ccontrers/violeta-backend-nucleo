/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `cartasporte`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `cartasporte` (
  `referencia` varchar(11) NOT NULL,
  `empresatrans` varchar(2) NOT NULL,
  `tipofac` varchar(11) NOT NULL,
  `terminal` varchar(10) NOT NULL DEFAULT '',
  `cancelado` tinyint(1) DEFAULT NULL,
  `clientepaga` varchar(11) DEFAULT '""',
  `tipocarta` int(1) DEFAULT NULL,
  `cuota` int(11) DEFAULT NULL,
  `valorcuota` decimal(16,2) DEFAULT NULL,
  `via` varchar(10) DEFAULT NULL,
  `chofer` varchar(10) DEFAULT NULL,
  `embarque` varchar(11) DEFAULT NULL,
  `usualta` varchar(10) DEFAULT NULL,
  `usumodi` varchar(10) DEFAULT NULL,
  `clientedest` varchar(11) DEFAULT NULL,
  `proveedororig` varchar(11) DEFAULT NULL,
  `fechaalta` date DEFAULT NULL,
  `horaalta` time DEFAULT NULL,
  `fechamodi` date DEFAULT NULL,
  `horamodi` time DEFAULT NULL,
  `fechacp` date DEFAULT NULL,
  `horacp` time DEFAULT NULL,
  `valor` decimal(16,2) DEFAULT NULL,
  `vumaniobras` decimal(16,6) DEFAULT NULL,
  `porciva` decimal(16,6) DEFAULT NULL,
  `porcret` decimal(16,6) DEFAULT NULL,
  `maniobras` decimal(16,6) DEFAULT NULL,
  `flete` decimal(16,6) DEFAULT NULL,
  `numarticulos` int(2) DEFAULT NULL,
  `acredito` tinyint(1) DEFAULT 0,
  PRIMARY KEY (`referencia`),
  KEY `clientepaga` (`clientepaga`),
  KEY `terminal` (`terminal`),
  KEY `fechacp` (`fechacp`),
  KEY `embarque` (`embarque`),
  KEY `tipofac` (`tipofac`),
  KEY `usualta` (`usualta`),
  KEY `usumodi` (`usumodi`),
  CONSTRAINT `cartasp_ibfk_1` FOREIGN KEY (`terminal`) REFERENCES `terminales` (`terminal`) ON UPDATE CASCADE,
  CONSTRAINT `cartasp_ibfk_2` FOREIGN KEY (`usualta`) REFERENCES `usuarios` (`empleado`) ON UPDATE CASCADE,
  CONSTRAINT `cartasp_ibfk_3` FOREIGN KEY (`usumodi`) REFERENCES `usuarios` (`empleado`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

