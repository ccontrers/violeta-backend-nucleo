/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `puntoscorteexistencias`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `puntoscorteexistencias` (
  `fecha` date NOT NULL DEFAULT '0000-00-00',
  `producto` varchar(8) NOT NULL,
  `present` varchar(13) NOT NULL,
  `almacen` varchar(4) NOT NULL DEFAULT '',
  `cantidad` decimal(12,3) DEFAULT NULL,
  KEY `fecha` (`fecha`),
  KEY `almacen` (`almacen`),
  KEY `prodpres` (`producto`,`present`),
  CONSTRAINT `FK_pce_prodpresent` FOREIGN KEY (`producto`, `present`) REFERENCES `presentaciones` (`producto`, `present`) ON UPDATE CASCADE,
  CONSTRAINT `FK_puntoscorteexistencias` FOREIGN KEY (`fecha`) REFERENCES `puntoscorte` (`fecha`) ON UPDATE CASCADE,
  CONSTRAINT `puntoscorteexistencias_ibfk_1` FOREIGN KEY (`almacen`) REFERENCES `almacenes` (`almacen`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

