/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `bitacorasolicitudesnotascredcli`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `bitacorasolicitudesnotascredcli` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `referencia` varchar(11) DEFAULT NULL,
  `notacredito` varchar(11) DEFAULT NULL,
  `autorizado` tinyint(4) DEFAULT NULL,
  `articulo` varchar(11) DEFAULT NULL,
  `aplicado` tinyint(1) NOT NULL DEFAULT 0,
  `cancelado` tinyint(1) NOT NULL DEFAULT 0,
  `usumodi` varchar(10) DEFAULT NULL,
  `fechamod` date DEFAULT NULL,
  `horamod` time DEFAULT NULL,
  `tipo` varchar(5) DEFAULT NULL,
  `terminal` varchar(10) DEFAULT NULL,
  PRIMARY KEY (`id`) USING BTREE,
  KEY `articulo` (`articulo`) USING BTREE,
  KEY `usumodi` (`usumodi`) USING BTREE,
  CONSTRAINT `bitacorasolicitudesnotascredcli_ibfk_1` FOREIGN KEY (`usumodi`) REFERENCES `empleados` (`empleado`) ON UPDATE CASCADE,
  CONSTRAINT `bitacorasolicitudesnotascredcli_ibfk_2` FOREIGN KEY (`articulo`) REFERENCES `articulos` (`articulo`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

