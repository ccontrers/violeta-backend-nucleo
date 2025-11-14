/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `bancosventasdef`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `bancosventasdef` (
  `idbancosventas` int(11) NOT NULL AUTO_INCREMENT,
  `sucursal` varchar(2) NOT NULL,
  `fechaventas` date NOT NULL,
  `fechaalta` date NOT NULL,
  `horaalta` time NOT NULL,
  `fechamodi` date NOT NULL,
  `horamodi` time NOT NULL,
  `usualta` varchar(10) NOT NULL,
  `usumodi` varchar(10) NOT NULL,
  `termino` varchar(10) NOT NULL,
  `total` decimal(16,2) NOT NULL,
  `totalventas` decimal(16,2) NOT NULL DEFAULT 0.00,
  `cancelado` tinyint(1) NOT NULL,
  PRIMARY KEY (`idbancosventas`),
  KEY `fechaventas` (`fechaventas`),
  KEY `fechaalta` (`fechaalta`),
  KEY `sucursal` (`sucursal`),
  KEY `FK_bancosventasdef_usualta` (`usualta`),
  KEY `FK_bancosventasdef_usumodi` (`usumodi`),
  KEY `FK_bancosventasdef_termino` (`termino`),
  CONSTRAINT `FK_bancosventasdef_termino` FOREIGN KEY (`termino`) REFERENCES `terminosdepago` (`termino`) ON UPDATE CASCADE,
  CONSTRAINT `FK_bancosventasdef_usualta` FOREIGN KEY (`usualta`) REFERENCES `usuarios` (`empleado`) ON UPDATE CASCADE,
  CONSTRAINT `FK_bancosventasdef_usumodi` FOREIGN KEY (`usumodi`) REFERENCES `usuarios` (`empleado`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

