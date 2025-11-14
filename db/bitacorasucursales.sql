/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `bitacorasucursales`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `bitacorasucursales` (
  `sucursal` varchar(2) NOT NULL,
  `numid` int(2) NOT NULL DEFAULT 0,
  `numidant` int(2) NOT NULL DEFAULT 0,
  `nombre` varchar(40) NOT NULL,
  `nombreant` varchar(40) NOT NULL,
  `calle` varchar(60) DEFAULT NULL,
  `calleant` varchar(60) DEFAULT NULL,
  `colonia` varchar(40) DEFAULT NULL,
  `coloniaant` varchar(40) DEFAULT NULL,
  `cp` varchar(10) DEFAULT NULL,
  `cpant` varchar(10) DEFAULT NULL,
  `localidad` varchar(40) DEFAULT NULL,
  `localidadant` varchar(40) DEFAULT NULL,
  `email` varchar(50) DEFAULT NULL,
  `emailant` varchar(50) DEFAULT NULL,
  `activa` tinyint(4) DEFAULT NULL,
  `activaant` tinyint(4) DEFAULT NULL,
  `venxvol` tinyint(4) DEFAULT NULL,
  `venxvolant` tinyint(4) DEFAULT NULL,
  `ubicaciongis` point DEFAULT NULL,
  `ubicaciongisant` point DEFAULT NULL,
  `salidaotrasucursal` tinyint(4) DEFAULT NULL,
  `salidaotrasucursalant` tinyint(4) DEFAULT NULL,
  `vtasecom` tinyint(4) NOT NULL DEFAULT 0,
  `vtasecomant` tinyint(4) NOT NULL DEFAULT 0,
  `defaultecom` tinyint(1) NOT NULL DEFAULT 0,
  `defaultecomant` tinyint(1) NOT NULL DEFAULT 0,
  `pickup` tinyint(1) NOT NULL DEFAULT 0,
  `pickupant` tinyint(1) NOT NULL DEFAULT 0,
  `idempresa` int(2) DEFAULT NULL,
  `idempresaant` int(2) DEFAULT NULL,
  `fechamod` date NOT NULL DEFAULT curdate(),
  `horamod` time NOT NULL DEFAULT curtime(),
  `usumod` varchar(10) NOT NULL,
  `tarea` varchar(1) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

