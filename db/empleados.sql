/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `empleados`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `empleados` (
  `empleado` varchar(10) NOT NULL DEFAULT '',
  `nombre` varchar(40) NOT NULL DEFAULT '',
  `appat` varchar(40) NOT NULL DEFAULT '',
  `apmat` varchar(40) NOT NULL DEFAULT '',
  `calle` varchar(60) DEFAULT NULL,
  `colonia` varchar(40) DEFAULT NULL,
  `cp` varchar(10) DEFAULT NULL,
  `localidad` varchar(40) DEFAULT NULL,
  `telefono1` varchar(20) DEFAULT NULL,
  `telefono2` varchar(20) DEFAULT NULL,
  `fechaalta` date NOT NULL DEFAULT '0000-00-00',
  `fechabaja` date NOT NULL DEFAULT '0000-00-00',
  `puesto` varchar(4) DEFAULT NULL,
  `depart` varchar(5) DEFAULT NULL,
  `jefe` varchar(10) DEFAULT NULL,
  `sucursal` varchar(2) NOT NULL,
  `activo` tinyint(1) DEFAULT NULL,
  `rfc` varchar(15) DEFAULT NULL,
  `solicitagasto` tinyint(1) DEFAULT NULL,
  `autorizagasto` tinyint(1) DEFAULT NULL,
  `numempleado` varchar(10) DEFAULT NULL,
  `nss` varchar(15) DEFAULT NULL,
  `imagen` mediumtext DEFAULT NULL,
  `contactoemer` varchar(50) NOT NULL DEFAULT '',
  `domicilioemer` varchar(50) NOT NULL DEFAULT '',
  `esSistema` tinyint(1) NOT NULL DEFAULT 0,
  `empleadoidcorto` varchar(6) DEFAULT NULL,
  PRIMARY KEY (`empleado`),
  UNIQUE KEY `empleadoidcorto` (`empleadoidcorto`),
  KEY `apellido` (`appat`,`apmat`,`nombre`),
  KEY `nombre` (`nombre`,`appat`,`apmat`),
  KEY `puesto` (`puesto`),
  KEY `depart` (`depart`),
  KEY `empleados_ibfk_4` (`sucursal`),
  KEY `rfc` (`rfc`) USING BTREE,
  CONSTRAINT `empleados_depart` FOREIGN KEY (`depart`) REFERENCES `departamentosrh` (`depart`) ON UPDATE CASCADE,
  CONSTRAINT `empleados_ibfk_1` FOREIGN KEY (`puesto`) REFERENCES `puestos` (`puesto`) ON UPDATE CASCADE,
  CONSTRAINT `empleados_ibfk_4` FOREIGN KEY (`sucursal`) REFERENCES `sucursales` (`sucursal`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

