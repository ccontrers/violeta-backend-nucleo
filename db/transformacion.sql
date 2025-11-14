/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `transformacion`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `transformacion` (
  `transfor` varchar(11) NOT NULL DEFAULT '',
  `cancelado` tinyint(1) DEFAULT NULL,
  `fechatrans` date NOT NULL DEFAULT '0000-00-00',
  `fechaalta` date NOT NULL DEFAULT '0000-00-00',
  `horaalta` time NOT NULL DEFAULT '00:00:00',
  `fechamodi` date NOT NULL DEFAULT '0000-00-00',
  `horamodi` time NOT NULL DEFAULT '00:00:00',
  `usualta` varchar(10) DEFAULT NULL,
  `usumodi` varchar(10) DEFAULT NULL,
  `foliofisic` varchar(15) NOT NULL DEFAULT '',
  `almacen` varchar(4) NOT NULL DEFAULT '',
  `auditado` tinyint(1) NOT NULL DEFAULT 0,
  PRIMARY KEY (`transfor`),
  KEY `fechatrans` (`fechatrans`),
  KEY `foliofisic` (`foliofisic`),
  KEY `almacen` (`almacen`),
  KEY `usualta` (`usualta`),
  KEY `usumodi` (`usumodi`),
  CONSTRAINT `transformacion_ibfk_1` FOREIGN KEY (`usualta`) REFERENCES `usuarios` (`empleado`) ON UPDATE CASCADE,
  CONSTRAINT `transformacion_ibfk_2` FOREIGN KEY (`usumodi`) REFERENCES `usuarios` (`empleado`) ON UPDATE CASCADE,
  CONSTRAINT `transformacion_ibfk_3` FOREIGN KEY (`almacen`) REFERENCES `almacenes` (`almacen`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

