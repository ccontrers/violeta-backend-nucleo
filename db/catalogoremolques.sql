/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `catalogoremolques`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `catalogoremolques` (
  `idremolque` int(11) NOT NULL AUTO_INCREMENT,
  `numserie` varchar(50) NOT NULL DEFAULT '00000000000',
  `numeroplaca` varchar(50) NOT NULL,
  `ccvesubtiporem` int(11) NOT NULL,
  `pesolimite` decimal(16,6) NOT NULL DEFAULT 1000.000000,
  `volumenlimite` decimal(16,6) NOT NULL DEFAULT 100.000000,
  `senasparticulares` varchar(150) DEFAULT NULL,
  `activo` tinyint(1) NOT NULL,
  `sucasignada` varchar(2) NOT NULL,
  `fechaalta` date NOT NULL DEFAULT curdate(),
  `horaalta` time NOT NULL DEFAULT curtime(),
  `fechamod` date NOT NULL DEFAULT curdate(),
  `horamod` time NOT NULL DEFAULT curtime(),
  `usualta` varchar(10) NOT NULL,
  `usumod` varchar(10) NOT NULL,
  PRIMARY KEY (`idremolque`),
  KEY `numserie` (`numserie`),
  KEY `numeroplaca` (`numeroplaca`),
  KEY `usualta` (`usualta`),
  KEY `usumod` (`usumod`),
  KEY `remolques_ibfk_1` (`ccvesubtiporem`),
  KEY `remolques_ibfk_2` (`sucasignada`),
  CONSTRAINT `remolques_ibfk_1` FOREIGN KEY (`ccvesubtiporem`) REFERENCES `ccpsubtiporem` (`idclavesubtiprem`) ON UPDATE CASCADE,
  CONSTRAINT `remolques_ibfk_2` FOREIGN KEY (`sucasignada`) REFERENCES `sucursales` (`sucursal`) ON UPDATE CASCADE,
  CONSTRAINT `remolques_ibfk_3` FOREIGN KEY (`usualta`) REFERENCES `usuarios` (`empleado`) ON UPDATE CASCADE,
  CONSTRAINT `remolques_ibfk_4` FOREIGN KEY (`usumod`) REFERENCES `usuarios` (`empleado`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

