/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `dcartasporte20`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `dcartasporte20` (
  `cartaporte20` varchar(20) NOT NULL,
  `bienestransp` int(11) NOT NULL,
  `descripcion` varchar(100) NOT NULL,
  `cantidad` decimal(16,6) NOT NULL,
  `claveunidad` int(11) NOT NULL,
  `pesokg` decimal(16,3) NOT NULL,
  `articulo` varchar(11) NOT NULL,
  PRIMARY KEY (`cartaporte20`,`articulo`),
  KEY `cartaporte20` (`cartaporte20`),
  KEY `articulo` (`articulo`),
  KEY `bienestransp` (`bienestransp`),
  KEY `claveunidad` (`claveunidad`),
  CONSTRAINT `dcartasp_ibfk_1` FOREIGN KEY (`cartaporte20`) REFERENCES `cartasporte20` (`cartaporte20`) ON UPDATE CASCADE,
  CONSTRAINT `dcartasp_ibfk_2` FOREIGN KEY (`articulo`) REFERENCES `articulos` (`articulo`) ON UPDATE CASCADE,
  CONSTRAINT `dcartasp_ibfk_3` FOREIGN KEY (`bienestransp`) REFERENCES `cclaveprodserv` (`idclaveprodserv`) ON UPDATE CASCADE,
  CONSTRAINT `dcartasp_ibfk_4` FOREIGN KEY (`claveunidad`) REFERENCES `cclaveunidad` (`idclaveunidad`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

