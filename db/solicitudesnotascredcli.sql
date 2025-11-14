/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `solicitudesnotascredcli`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `solicitudesnotascredcli` (
  `referencia` varchar(11) NOT NULL,
  `notacredito` varchar(11) DEFAULT NULL,
  `usuario` varchar(10) DEFAULT NULL,
  `venta` varchar(50) DEFAULT NULL,
  `terminal` varchar(10) DEFAULT NULL,
  `total` decimal(16,3) NOT NULL DEFAULT 0.000,
  `tipo` tinyint(4) NOT NULL DEFAULT 0,
  `cancelado` tinyint(4) NOT NULL DEFAULT 0,
  `fechaalta` date DEFAULT NULL,
  `horaalta` time DEFAULT NULL,
  `fechasol` date DEFAULT NULL,
  `horasol` time DEFAULT NULL,
  `fechamod` date DEFAULT NULL,
  `horamod` time DEFAULT NULL,
  `aplicado` tinyint(4) NOT NULL DEFAULT 0,
  PRIMARY KEY (`referencia`),
  KEY `usuario` (`usuario`),
  KEY `venta` (`venta`),
  KEY `tipo` (`tipo`),
  KEY `terminal` (`terminal`),
  KEY `solicitudesnotascredcli_ibfk_5` (`notacredito`),
  CONSTRAINT `solicitudesnotascredcli_ibfk_1` FOREIGN KEY (`usuario`) REFERENCES `empleados` (`empleado`) ON UPDATE CASCADE,
  CONSTRAINT `solicitudesnotascredcli_ibfk_2` FOREIGN KEY (`venta`) REFERENCES `ventas` (`referencia`) ON UPDATE CASCADE,
  CONSTRAINT `solicitudesnotascredcli_ibfk_3` FOREIGN KEY (`tipo`) REFERENCES `tiposnotascredito` (`tipo`) ON UPDATE CASCADE,
  CONSTRAINT `solicitudesnotascredcli_ibfk_4` FOREIGN KEY (`terminal`) REFERENCES `terminales` (`terminal`) ON UPDATE CASCADE,
  CONSTRAINT `solicitudesnotascredcli_ibfk_5` FOREIGN KEY (`notacredito`) REFERENCES `notascredcli` (`referencia`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

