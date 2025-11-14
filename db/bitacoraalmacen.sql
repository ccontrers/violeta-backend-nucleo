/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `bitacoraalmacen`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `bitacoraalmacen` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `almacen` varchar(4) DEFAULT NULL,
  `seccion` varchar(4) DEFAULT NULL,
  `seccionant` varchar(4) DEFAULT NULL,
  `nombre` varchar(40) DEFAULT NULL,
  `nombreant` varchar(40) DEFAULT NULL,
  `encargado` varchar(10) DEFAULT NULL,
  `encargadoant` varchar(10) DEFAULT NULL,
  `terminal` varchar(10) DEFAULT NULL,
  `terminalant` varchar(10) DEFAULT NULL,
  `calle` varchar(60) DEFAULT NULL,
  `calleant` varchar(60) DEFAULT NULL,
  `colonia` varchar(40) DEFAULT NULL,
  `coloniaant` varchar(40) DEFAULT NULL,
  `localidad` varchar(40) DEFAULT NULL,
  `localidadant` varchar(40) DEFAULT NULL,
  `cp` varchar(10) DEFAULT NULL,
  `cpant` varchar(10) DEFAULT NULL,
  `email` varchar(50) DEFAULT NULL,
  `emailant` varchar(50) DEFAULT NULL,
  `telefono` varchar(20) DEFAULT NULL,
  `telefonoant` varchar(20) DEFAULT NULL,
  `activo` tinyint(1) DEFAULT NULL,
  `activoant` tinyint(1) DEFAULT NULL,
  `permcompras` tinyint(1) DEFAULT NULL,
  `permcomprasant` tinyint(1) DEFAULT NULL,
  `usuario` varchar(10) DEFAULT NULL,
  `fecha` date NOT NULL DEFAULT '0000-00-00',
  `hora` time NOT NULL DEFAULT '00:00:00',
  PRIMARY KEY (`id`) USING BTREE,
  KEY `almacen` (`almacen`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

