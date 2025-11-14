/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `presentaciones`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `presentaciones` (
  `producto` varchar(8) NOT NULL DEFAULT '',
  `present` varchar(13) NOT NULL DEFAULT '',
  `modoutil` int(1) DEFAULT NULL,
  `sgerencia` tinyint(1) DEFAULT 0,
  `permitfrac` tinyint(1) DEFAULT 0,
  `presentsat` decimal(10,3) NOT NULL DEFAULT 0.000,
  `idempaque` varchar(3) NOT NULL DEFAULT '',
  `idunidad` varchar(3) NOT NULL DEFAULT '',
  `cotizable` tinyint(1) NOT NULL DEFAULT 0,
  `iepscuota` decimal(13,6) NOT NULL DEFAULT 0.000000,
  `factortarima` decimal(10,3) DEFAULT 0.000,
  PRIMARY KEY (`producto`,`present`),
  KEY `present` (`present`),
  KEY `presentsat` (`presentsat`),
  KEY `idempaque` (`idempaque`),
  KEY `idunidad` (`idunidad`),
  CONSTRAINT `presentaciones_ibfk_1` FOREIGN KEY (`producto`) REFERENCES `productos` (`producto`) ON UPDATE CASCADE,
  CONSTRAINT `presentaciones_ibfk_2` FOREIGN KEY (`idempaque`) REFERENCES `satempaques` (`idempaque`) ON UPDATE CASCADE,
  CONSTRAINT `presentaciones_ibfk_3` FOREIGN KEY (`idunidad`) REFERENCES `satunidadmed` (`idunidad`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

