/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `solicitudticketsrh`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `solicitudticketsrh` (
  `referencia` varchar(11) NOT NULL DEFAULT '0',
  `puestosoli` varchar(4) DEFAULT NULL,
  `tipoBaja` varchar(4) DEFAULT NULL,
  `nombreEmpleado` varchar(80) DEFAULT NULL,
  `numvacantes` int(3) DEFAULT NULL,
  `tiporeemplazo` tinyint(1) DEFAULT NULL,
  `escolaridad` tinyint(1) NOT NULL DEFAULT 0,
  `sexo` tinyint(1) DEFAULT NULL,
  `sueldo` varchar(50) DEFAULT NULL,
  `horariocubrir` varchar(50) DEFAULT NULL,
  `diastrabajo` varchar(50) DEFAULT NULL,
  `descansoentresemana` tinyint(1) NOT NULL DEFAULT 0,
  `laboradomingo` tinyint(1) NOT NULL DEFAULT 0,
  `actividadespuesto` text DEFAULT NULL,
  `capacitacion` tinyint(1) NOT NULL DEFAULT 0,
  `fechainicapacitacion` date DEFAULT NULL,
  `fechafincapacitacion` date DEFAULT NULL,
  `otrosconocimientos` text DEFAULT NULL,
  PRIMARY KEY (`referencia`) USING BTREE,
  KEY `solicitudticketsrh_ibfk_2` (`puestosoli`),
  KEY `FK_solicitudticketsrh_catalogobajasrh` (`tipoBaja`),
  CONSTRAINT `FK_solicitudticketsrh_catalogobajasrh` FOREIGN KEY (`tipoBaja`) REFERENCES `catalogobajasrh` (`clave`) ON UPDATE CASCADE,
  CONSTRAINT `solicitudticketsrh_ibfk_1` FOREIGN KEY (`referencia`) REFERENCES `solicitudtickets` (`referencia`) ON UPDATE CASCADE,
  CONSTRAINT `solicitudticketsrh_ibfk_2` FOREIGN KEY (`puestosoli`) REFERENCES `puestos` (`puesto`) ON DELETE NO ACTION ON UPDATE NO ACTION
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

