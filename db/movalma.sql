/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `movalma`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `movalma` (
  `movimiento` varchar(11) NOT NULL DEFAULT '',
  `tipo` varchar(1) DEFAULT NULL,
  `almaent` varchar(4) DEFAULT NULL,
  `almasal` varchar(4) DEFAULT NULL,
  `concepto` varchar(4) DEFAULT NULL,
  `cancelado` tinyint(1) DEFAULT NULL,
  `usualta` varchar(10) DEFAULT NULL,
  `usumodi` varchar(10) DEFAULT NULL,
  `fechaalta` date NOT NULL DEFAULT '0000-00-00',
  `horaalta` time DEFAULT NULL,
  `fechamodi` date NOT NULL DEFAULT '0000-00-00',
  `horamodi` time DEFAULT NULL,
  `chofer` varchar(10) DEFAULT '',
  `folioenvio` varchar(20) DEFAULT '',
  `fechamov` date NOT NULL DEFAULT '0000-00-00',
  `proveedor` varchar(11) NOT NULL DEFAULT '',
  `transfor` varchar(11) NOT NULL DEFAULT '',
  `aplica` tinyint(1) NOT NULL DEFAULT 1,
  `terminal` varchar(10) NOT NULL DEFAULT '',
  `auditado` tinyint(1) NOT NULL DEFAULT 0,
  `auditadopor` varchar(10) DEFAULT NULL,
  `fechaaudi` date DEFAULT NULL,
  `horaaudi` time DEFAULT NULL,
  PRIMARY KEY (`movimiento`),
  KEY `fechamov` (`fechamov`),
  KEY `transfor` (`transfor`),
  KEY `usualta` (`usualta`),
  KEY `usumodi` (`usumodi`),
  KEY `concepto` (`concepto`),
  KEY `terminal` (`terminal`),
  CONSTRAINT `movalma_ibfk_1` FOREIGN KEY (`usualta`) REFERENCES `usuarios` (`empleado`) ON UPDATE CASCADE,
  CONSTRAINT `movalma_ibfk_2` FOREIGN KEY (`usumodi`) REFERENCES `usuarios` (`empleado`) ON UPDATE CASCADE,
  CONSTRAINT `movalma_ibfk_3` FOREIGN KEY (`concepto`) REFERENCES `conceptosmovalma` (`concepto`) ON UPDATE CASCADE,
  CONSTRAINT `movalma_ibfk_4` FOREIGN KEY (`terminal`) REFERENCES `terminales` (`terminal`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

