/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `notascarprov`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `notascarprov` (
  `referencia` varchar(11) NOT NULL DEFAULT '',
  `folioprov` varchar(15) NOT NULL DEFAULT '',
  `pago` varchar(11) DEFAULT NULL,
  `tipo` varchar(1) NOT NULL DEFAULT '',
  `cancelado` tinyint(1) DEFAULT NULL,
  `usualta` varchar(10) DEFAULT NULL,
  `usumodi` varchar(10) DEFAULT NULL,
  `fechaalta` date NOT NULL DEFAULT '0000-00-00',
  `horaalta` time DEFAULT NULL,
  `fechamodi` date NOT NULL DEFAULT '0000-00-00',
  `horamodi` time DEFAULT NULL,
  `fechanot` date NOT NULL DEFAULT '0000-00-00',
  `valor` decimal(16,2) DEFAULT NULL,
  `docseccion` varchar(4) DEFAULT NULL,
  `docdepart` varchar(10) DEFAULT NULL,
  `proveedor` varchar(11) NOT NULL DEFAULT '',
  `cveimp` int(2) NOT NULL DEFAULT 0,
  `terminal` varchar(10) DEFAULT NULL,
  `muuid` varchar(36) DEFAULT NULL,
  PRIMARY KEY (`referencia`),
  KEY `folioprov` (`folioprov`),
  KEY `pago` (`pago`),
  KEY `fechanot` (`fechanot`),
  KEY `tipo` (`tipo`),
  KEY `usualta` (`usualta`),
  KEY `usumodi` (`usumodi`),
  KEY `docseccion` (`docseccion`),
  KEY `docdepart` (`docdepart`),
  KEY `terminal` (`terminal`),
  KEY `proveedor` (`proveedor`),
  KEY `muuid` (`muuid`),
  CONSTRAINT `notascarprov_ibfk_1` FOREIGN KEY (`usualta`) REFERENCES `usuarios` (`empleado`) ON UPDATE CASCADE,
  CONSTRAINT `notascarprov_ibfk_2` FOREIGN KEY (`usumodi`) REFERENCES `usuarios` (`empleado`) ON UPDATE CASCADE,
  CONSTRAINT `notascarprov_ibfk_3` FOREIGN KEY (`docseccion`) REFERENCES `secciones` (`seccion`) ON UPDATE CASCADE,
  CONSTRAINT `notascarprov_ibfk_4` FOREIGN KEY (`docdepart`) REFERENCES `departamentos` (`depart`) ON UPDATE CASCADE,
  CONSTRAINT `notascarprov_ibfk_5` FOREIGN KEY (`terminal`) REFERENCES `terminales` (`terminal`) ON UPDATE CASCADE,
  CONSTRAINT `notascarprov_ibfk_6` FOREIGN KEY (`proveedor`) REFERENCES `proveedores` (`proveedor`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

