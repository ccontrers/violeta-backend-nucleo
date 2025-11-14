/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `empleadosdirecciondefault`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `empleadosdirecciondefault` (
  `sucursal` varchar(2) NOT NULL,
  `colonia` varchar(10) NOT NULL DEFAULT '0',
  `localidad` varchar(4) NOT NULL DEFAULT '0',
  `municipio` varchar(4) NOT NULL DEFAULT '0',
  `estado` varchar(4) NOT NULL DEFAULT '0',
  `calle` varchar(60) NOT NULL DEFAULT '0',
  `cp` varchar(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`sucursal`) USING BTREE,
  KEY `colonia` (`colonia`) USING BTREE,
  KEY `localidad` (`localidad`) USING BTREE,
  KEY `municipio` (`municipio`) USING BTREE,
  KEY `estado` (`estado`) USING BTREE,
  CONSTRAINT `empleadosdirecciondefault_ibfk_1` FOREIGN KEY (`sucursal`) REFERENCES `sucursales` (`sucursal`) ON UPDATE CASCADE,
  CONSTRAINT `empleadosdirecciondefault_ibfk_2` FOREIGN KEY (`colonia`) REFERENCES `colonias` (`colonia`) ON UPDATE CASCADE,
  CONSTRAINT `empleadosdirecciondefault_ibfk_3` FOREIGN KEY (`localidad`) REFERENCES `localidades` (`localidad`) ON UPDATE CASCADE,
  CONSTRAINT `empleadosdirecciondefault_ibfk_4` FOREIGN KEY (`municipio`) REFERENCES `municipios` (`municipio`) ON UPDATE CASCADE,
  CONSTRAINT `empleadosdirecciondefault_ibfk_5` FOREIGN KEY (`estado`) REFERENCES `estados` (`estado`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

