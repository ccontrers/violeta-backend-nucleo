/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `bitacoraempleados`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `bitacoraempleados` (
  `empleado` varchar(10) NOT NULL DEFAULT '',
  `nombre` varchar(40) NOT NULL DEFAULT '',
  `appat` varchar(40) NOT NULL DEFAULT '',
  `apmat` varchar(40) NOT NULL DEFAULT '',
  `puesto` varchar(4) DEFAULT NULL,
  `puestoant` varchar(10) DEFAULT NULL,
  `depart` varchar(10) NOT NULL DEFAULT '',
  `departant` varchar(10) DEFAULT NULL,
  `jefe` varchar(10) DEFAULT NULL,
  `jefeant` varchar(10) DEFAULT NULL,
  `sucursal` varchar(2) NOT NULL,
  `sucursalant` varchar(2) DEFAULT NULL,
  `activo` tinyint(1) DEFAULT NULL,
  `numempleado` varchar(11) DEFAULT NULL,
  `nss` varchar(15) DEFAULT NULL,
  `fechamodi` date DEFAULT NULL,
  `solicitagasto` tinyint(4) DEFAULT NULL,
  `solicitagastoant` tinyint(4) DEFAULT NULL,
  `autorizagasto` tinyint(4) DEFAULT NULL,
  `autorizagastoant` tinyint(4) DEFAULT NULL,
  `rfc` varchar(15) DEFAULT NULL,
  `fechaalta` date DEFAULT '0000-00-00',
  `fechabaja` date DEFAULT '0000-00-00',
  `usuariomodi` varchar(10) NOT NULL,
  `tarea` varchar(1) NOT NULL,
  `horamodi` time DEFAULT '00:00:00'
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

