/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `bitacoradetalleartpedmas`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `bitacoradetalleartpedmas` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `referencia` int(11) NOT NULL,
  `evento` varchar(1) NOT NULL,
  `sucursal` varchar(8) NOT NULL,
  `producto` varchar(8) NOT NULL,
  `present` varchar(13) NOT NULL,
  `proveedoranterior` varchar(12) DEFAULT NULL,
  PRIMARY KEY (`id`,`sucursal`,`referencia`,`producto`,`present`) USING BTREE,
  KEY `referencia` (`referencia`),
  KEY `producto` (`producto`,`present`),
  KEY `sucursal` (`sucursal`),
  KEY `proveedoranterior` (`proveedoranterior`),
  CONSTRAINT `bitacoradetalleartpedmas_ibfk_1` FOREIGN KEY (`referencia`) REFERENCES `bitacoraartpedmas` (`idbitacora`) ON UPDATE CASCADE,
  CONSTRAINT `bitacoradetalleartpedmas_ibfk_2` FOREIGN KEY (`producto`, `present`) REFERENCES `presentaciones` (`producto`, `present`) ON UPDATE CASCADE,
  CONSTRAINT `bitacoradetalleartpedmas_ibfk_3` FOREIGN KEY (`sucursal`) REFERENCES `sucursales` (`sucursal`) ON UPDATE CASCADE,
  CONSTRAINT `bitacoradetalleartpedmas_ibfk_4` FOREIGN KEY (`proveedoranterior`) REFERENCES `proveedores` (`proveedor`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

