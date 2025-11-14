/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `bitacoracomprasmoduuid`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `bitacoracomprasmoduuid` (
  `referencia` varchar(11) NOT NULL,
  `foliofactura` varchar(15) NOT NULL,
  `proveedor` varchar(11) DEFAULT NULL,
  `usuario` varchar(910) DEFAULT NULL,
  `usumodi` varchar(910) DEFAULT NULL,
  `fechaini` date DEFAULT NULL,
  `horaini` time DEFAULT NULL,
  `fechaalta` date DEFAULT NULL,
  `horaalta` time DEFAULT NULL,
  `fechamodi` date DEFAULT NULL,
  `horamodi` time DEFAULT NULL,
  `tipo` varchar(2) DEFAULT NULL COMMENT 'A - Agregado, M - Modificado',
  `fechapagoantes` date DEFAULT NULL,
  `fechapagodespues` date DEFAULT NULL,
  `uuidantes` varchar(36) DEFAULT NULL,
  `uuiddespues` varchar(36) DEFAULT NULL,
  `numregistrosantes` int(4) DEFAULT NULL,
  `numregistrosdespues` int(4) DEFAULT NULL,
  KEY `bitacoracomprasmoduuid_referencia` (`referencia`),
  KEY `bitacoracomprasmoduuid_usuario` (`usuario`),
  KEY `bitacoracomprasmoduuid_ibfk_3` (`proveedor`),
  CONSTRAINT `bitacoracomprasmoduuid_ibfk_3` FOREIGN KEY (`proveedor`) REFERENCES `proveedores` (`proveedor`) ON UPDATE CASCADE,
  CONSTRAINT `bitacoracomprasmoduuid_referencia` FOREIGN KEY (`referencia`) REFERENCES `compras` (`referencia`) ON UPDATE CASCADE,
  CONSTRAINT `bitacoracomprasmoduuid_usuario` FOREIGN KEY (`usuario`) REFERENCES `empleados` (`empleado`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

