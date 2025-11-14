/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `bitacorakits`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `bitacorakits` (
  `idBitacoraKit` int(255) NOT NULL AUTO_INCREMENT,
  `kit` varchar(9) DEFAULT NULL,
  `nombre` varchar(40) DEFAULT NULL,
  `usuario` varchar(9) DEFAULT NULL,
  `fecha` date DEFAULT curdate(),
  `hora` time DEFAULT curtime(),
  `tipo` varchar(2) DEFAULT NULL,
  `activo` tinyint(1) DEFAULT NULL,
  `desglosado` tinyint(1) DEFAULT NULL,
  `cancelado` tinyint(1) DEFAULT NULL,
  PRIMARY KEY (`idBitacoraKit`),
  KEY `kit` (`kit`),
  KEY `usuario` (`usuario`),
  CONSTRAINT `bitacorakits_ibfk_1` FOREIGN KEY (`kit`) REFERENCES `kits` (`kit`) ON UPDATE CASCADE,
  CONSTRAINT `bitacorakits_ibfk_2` FOREIGN KEY (`usuario`) REFERENCES `usuarios` (`empleado`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

