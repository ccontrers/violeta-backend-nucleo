/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `cortesdecaja`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `cortesdecaja` (
  `referencia` varchar(15) NOT NULL,
  `fechaapertura` date DEFAULT '0000-00-00',
  `horaapertura` time DEFAULT NULL,
  `fechacierre` date DEFAULT '0000-00-00',
  `horacierre` time DEFAULT NULL,
  `terminal` varchar(10) NOT NULL,
  `sucursal` varchar(5) NOT NULL,
  `usuarioapertura` varchar(10) DEFAULT NULL,
  `usuariocierre` varchar(10) DEFAULT NULL,
  `estatus` varchar(2) NOT NULL DEFAULT 'A' COMMENT 'A - Apertura C - Cierre',
  PRIMARY KEY (`referencia`),
  KEY `cortesdecaja_terminal_fk` (`terminal`),
  KEY `cortesdecaja_sucursal_fk` (`sucursal`),
  KEY `cortesdecaja_usuarioapertura_fk` (`usuarioapertura`),
  KEY `cortesdecaja_usuariocierre_fk` (`usuariocierre`),
  KEY `fechacierre` (`fechacierre`),
  CONSTRAINT `cortesdecaja_sucursal_fk` FOREIGN KEY (`sucursal`) REFERENCES `sucursales` (`sucursal`) ON UPDATE CASCADE,
  CONSTRAINT `cortesdecaja_terminal_fk` FOREIGN KEY (`terminal`) REFERENCES `terminales` (`terminal`) ON UPDATE CASCADE,
  CONSTRAINT `cortesdecaja_usuarioapertura_fk` FOREIGN KEY (`usuarioapertura`) REFERENCES `empleados` (`empleado`) ON UPDATE CASCADE,
  CONSTRAINT `cortesdecaja_usuariocierre_fk` FOREIGN KEY (`usuariocierre`) REFERENCES `empleados` (`empleado`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

