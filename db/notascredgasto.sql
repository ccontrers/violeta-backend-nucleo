/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `notascredgasto`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `notascredgasto` (
  `referencia` varchar(11) NOT NULL DEFAULT '0',
  `folioprov` varchar(60) NOT NULL DEFAULT '',
  `gasto` varchar(11) NOT NULL DEFAULT '0',
  `descuento` decimal(5,2) NOT NULL DEFAULT 0.00,
  `tipo` varchar(4) NOT NULL DEFAULT '',
  `cancelado` tinyint(1) DEFAULT NULL,
  `usualta` varchar(10) DEFAULT NULL,
  `usumodi` varchar(10) DEFAULT NULL,
  `fechaalta` date NOT NULL DEFAULT '0000-00-00',
  `horaalta` time DEFAULT NULL,
  `fechamodi` date NOT NULL DEFAULT '0000-00-00',
  `horamodi` time DEFAULT NULL,
  `fechanot` date NOT NULL DEFAULT '0000-00-00',
  `docdepart` varchar(10) NOT NULL DEFAULT '',
  `docseccion` varchar(4) NOT NULL DEFAULT '',
  `terminal` varchar(10) NOT NULL DEFAULT '',
  `valor` decimal(16,2) DEFAULT NULL,
  `muuid` varchar(36) DEFAULT NULL,
  `impuestoret` int(11) DEFAULT NULL,
  `totalret` decimal(16,2) NOT NULL DEFAULT 0.00,
  PRIMARY KEY (`referencia`) USING BTREE,
  KEY `folioprov` (`folioprov`) USING BTREE,
  KEY `gasto` (`gasto`) USING BTREE,
  KEY `fechanot` (`fechanot`) USING BTREE,
  KEY `tipo` (`tipo`) USING BTREE,
  KEY `usualta` (`usualta`) USING BTREE,
  KEY `usumodi` (`usumodi`) USING BTREE,
  KEY `docseccion` (`docseccion`) USING BTREE,
  KEY `docdepart` (`docdepart`) USING BTREE,
  KEY `terminal` (`terminal`) USING BTREE,
  KEY `muuid` (`muuid`) USING BTREE,
  KEY `notascredgasto_ibfk_8` (`impuestoret`),
  CONSTRAINT `notascredgasto_ibfk_1` FOREIGN KEY (`gasto`) REFERENCES `gastos` (`referencia`) ON UPDATE CASCADE,
  CONSTRAINT `notascredgasto_ibfk_2` FOREIGN KEY (`usualta`) REFERENCES `usuarios` (`empleado`) ON UPDATE CASCADE,
  CONSTRAINT `notascredgasto_ibfk_3` FOREIGN KEY (`usumodi`) REFERENCES `usuarios` (`empleado`) ON UPDATE CASCADE,
  CONSTRAINT `notascredgasto_ibfk_4` FOREIGN KEY (`docseccion`) REFERENCES `secciones` (`seccion`) ON UPDATE CASCADE,
  CONSTRAINT `notascredgasto_ibfk_5` FOREIGN KEY (`docdepart`) REFERENCES `departamentos` (`depart`) ON UPDATE CASCADE,
  CONSTRAINT `notascredgasto_ibfk_6` FOREIGN KEY (`terminal`) REFERENCES `terminales` (`terminal`) ON UPDATE CASCADE,
  CONSTRAINT `notascredgasto_ibfk_7` FOREIGN KEY (`tipo`) REFERENCES `tiposdencregastos` (`tipo`) ON UPDATE CASCADE,
  CONSTRAINT `notascredgasto_ibfk_8` FOREIGN KEY (`impuestoret`) REFERENCES `impuestos` (`impuesto`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

