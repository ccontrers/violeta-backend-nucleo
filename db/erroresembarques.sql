/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `erroresembarques`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `erroresembarques` (
  `iderror` int(11) NOT NULL AUTO_INCREMENT,
  `usualta` varchar(10) DEFAULT NULL,
  `fechaalta` date DEFAULT NULL,
  `horaalta` time DEFAULT NULL,
  `embarque` varchar(11) DEFAULT NULL,
  `area` varchar(1) DEFAULT NULL,
  `empleado` varchar(10) DEFAULT NULL,
  `criticidad` varchar(1) DEFAULT NULL,
  `error` varchar(250) DEFAULT NULL,
  `formulario` varchar(2) DEFAULT NULL,
  `referencia_nc` varchar(11) DEFAULT NULL,
  `referencia_vta` varchar(11) DEFAULT NULL,
  `articulo` varchar(9) DEFAULT NULL,
  `cantidad` decimal(12,3) DEFAULT NULL,
  `sucursal` varchar(10) DEFAULT NULL,
  `cancelado` tinyint(1) DEFAULT 0,
  PRIMARY KEY (`iderror`),
  KEY `usuarioalta` (`usualta`),
  KEY `numembarque` (`embarque`),
  KEY `claveempleado` (`empleado`),
  KEY `clavenotac` (`referencia_nc`),
  KEY `claveventa` (`referencia_vta`),
  KEY `clavearticulo` (`articulo`),
  KEY `erroresembarques_ibfk3` (`sucursal`),
  CONSTRAINT `erroresembarques_ibfk1` FOREIGN KEY (`usualta`) REFERENCES `usuarios` (`empleado`) ON UPDATE CASCADE,
  CONSTRAINT `erroresembarques_ibfk2` FOREIGN KEY (`embarque`) REFERENCES `embarques` (`embarque`) ON UPDATE CASCADE,
  CONSTRAINT `erroresembarques_ibfk3` FOREIGN KEY (`sucursal`) REFERENCES `sucursales` (`sucursal`) ON UPDATE CASCADE,
  CONSTRAINT `erroresembarques_ibfk4` FOREIGN KEY (`empleado`) REFERENCES `empleados` (`empleado`) ON UPDATE CASCADE,
  CONSTRAINT `erroresembarques_ibfk5` FOREIGN KEY (`referencia_nc`) REFERENCES `notascredcli` (`referencia`) ON UPDATE CASCADE,
  CONSTRAINT `erroresembarques_ibfk6` FOREIGN KEY (`referencia_vta`) REFERENCES `ventas` (`referencia`) ON UPDATE CASCADE,
  CONSTRAINT `erroresembarques_ibfk7` FOREIGN KEY (`articulo`) REFERENCES `articulos` (`articulo`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

