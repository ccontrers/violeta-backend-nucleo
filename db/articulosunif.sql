/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `articulosunif`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `articulosunif` (
  `idsucremota` int(11) NOT NULL,
  `articulo` varchar(9) DEFAULT NULL,
  `articulorem` varchar(9) NOT NULL,
  `productorem` varchar(8) NOT NULL,
  `nombrerem` varchar(60) NOT NULL,
  `presentrem` varchar(13) NOT NULL,
  `multiplorem` varchar(10) NOT NULL,
  `factorrem` decimal(10,3) DEFAULT NULL,
  `codigobarras` varchar(50) DEFAULT NULL,
  `esmanual` tinyint(4) NOT NULL,
  PRIMARY KEY (`idsucremota`,`articulorem`),
  KEY `unicalocal` (`idsucremota`,`articulo`),
  KEY `articulorem` (`articulorem`),
  KEY `articulo` (`articulo`),
  CONSTRAINT `fk_articulounif` FOREIGN KEY (`articulo`) REFERENCES `articulos` (`articulo`) ON UPDATE CASCADE,
  CONSTRAINT `fk_sucremotaunif` FOREIGN KEY (`idsucremota`) REFERENCES `unifsucursales` (`idsucremota`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

