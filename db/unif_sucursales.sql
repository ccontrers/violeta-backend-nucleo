/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `unif_sucursales`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `unif_sucursales` (
  `sucursal` varchar(2) NOT NULL DEFAULT '',
  `numid` int(2) NOT NULL DEFAULT 0,
  `nombre` varchar(40) NOT NULL DEFAULT '',
  `calle` varchar(60) DEFAULT NULL,
  `colonia` varchar(40) DEFAULT NULL,
  `cp` varchar(10) DEFAULT NULL,
  `localidad` varchar(40) DEFAULT NULL,
  `telefono1` varchar(20) DEFAULT NULL,
  `telefono2` varchar(20) DEFAULT NULL,
  `telefono3` varchar(20) DEFAULT NULL,
  `telefono4` varchar(20) DEFAULT NULL,
  `email` varchar(50) DEFAULT NULL,
  `activa` tinyint(1) DEFAULT 1,
  `venxvol` tinyint(1) DEFAULT 0,
  `ubicaciongis` point DEFAULT NULL,
  `salidaotrasucursal` tinyint(1) DEFAULT 0,
  `vtasecom` tinyint(4) NOT NULL DEFAULT 0,
  `defaultecom` tinyint(1) NOT NULL DEFAULT 0,
  `pickup` tinyint(1) NOT NULL DEFAULT 0,
  `idempresa` int(2) DEFAULT NULL,
  PRIMARY KEY (`sucursal`) USING BTREE,
  UNIQUE KEY `numid` (`numid`) USING BTREE,
  KEY `nombre` (`nombre`) USING BTREE,
  KEY `sucursalempresa_ibfk_1` (`idempresa`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

