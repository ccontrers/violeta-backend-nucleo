/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `dprodxcotizar`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `dprodxcotizar` (
  `id` int(11) NOT NULL,
  `nombre` varchar(60) DEFAULT NULL,
  `articulo` varchar(9) NOT NULL,
  `producto` varchar(8) DEFAULT NULL,
  `present` varchar(13) DEFAULT NULL,
  `multiplo` varchar(10) DEFAULT NULL,
  `factor` decimal(10,3) DEFAULT NULL,
  `codbarras` varchar(14) DEFAULT NULL,
  `codbarrasuni` varchar(14) DEFAULT NULL,
  `costo` decimal(16,6) DEFAULT NULL,
  `observaciones` varchar(100) DEFAULT NULL,
  `fabricante` varchar(40) DEFAULT NULL,
  PRIMARY KEY (`id`,`articulo`),
  KEY `dprodxcotizar_ibfk_2` (`articulo`),
  CONSTRAINT `dprodxcotizar_ibfk_1` FOREIGN KEY (`id`) REFERENCES `prodxcotizar` (`id`) ON UPDATE CASCADE,
  CONSTRAINT `dprodxcotizar_ibfk_2` FOREIGN KEY (`articulo`) REFERENCES `articulos` (`articulo`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

