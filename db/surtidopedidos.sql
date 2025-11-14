/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `surtidopedidos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `surtidopedidos` (
  `surtido` varchar(11) NOT NULL DEFAULT '0',
  `embarque` varchar(11) DEFAULT NULL,
  `terminal` varchar(10) DEFAULT NULL,
  `fecha_alta` date DEFAULT NULL,
  `hora_alta` time DEFAULT NULL,
  `fechasurt` date NOT NULL DEFAULT '0000-00-00',
  `fechamodi` date NOT NULL DEFAULT '0000-00-00',
  `horamodi` time DEFAULT NULL,
  `usualta` varchar(10) DEFAULT NULL,
  `usumod` varchar(10) DEFAULT NULL,
  `cancelado` int(1) DEFAULT NULL,
  `horaini` time DEFAULT NULL,
  `horafin` time DEFAULT NULL,
  `fecha_cancel` date DEFAULT NULL,
  `hora_cancel` time DEFAULT NULL,
  `aplicado` tinyint(4) NOT NULL DEFAULT 0,
  `estado` tinyint(1) NOT NULL DEFAULT 0,
  PRIMARY KEY (`surtido`) USING BTREE,
  KEY `usualta` (`usualta`) USING BTREE,
  KEY `usumod` (`usumod`) USING BTREE,
  KEY `terminal` (`terminal`) USING BTREE,
  KEY `surtidopedidos_ibfk_4` (`embarque`),
  CONSTRAINT `surtidopedidos_ibfk_1` FOREIGN KEY (`terminal`) REFERENCES `terminales` (`terminal`) ON UPDATE CASCADE,
  CONSTRAINT `surtidopedidos_ibfk_2` FOREIGN KEY (`usualta`) REFERENCES `empleados` (`empleado`) ON UPDATE CASCADE,
  CONSTRAINT `surtidopedidos_ibfk_3` FOREIGN KEY (`usumod`) REFERENCES `empleados` (`empleado`) ON UPDATE CASCADE,
  CONSTRAINT `surtidopedidos_ibfk_4` FOREIGN KEY (`embarque`) REFERENCES `embarques` (`embarque`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

