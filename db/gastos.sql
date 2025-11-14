/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `gastos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `gastos` (
  `referencia` varchar(11) NOT NULL DEFAULT '0',
  `folioprov` varchar(25) NOT NULL DEFAULT '',
  `cancelado` tinyint(1) DEFAULT NULL,
  `acredito` tinyint(1) DEFAULT NULL,
  `proveedor` varchar(11) DEFAULT NULL,
  `sucursal` varchar(2) DEFAULT NULL,
  `solicita` varchar(10) DEFAULT NULL,
  `depart` varchar(10) DEFAULT NULL,
  `autoriza` varchar(10) DEFAULT NULL,
  `usualta` varchar(10) DEFAULT NULL,
  `usumodi` varchar(10) DEFAULT NULL,
  `fechaalta` date NOT NULL DEFAULT '0000-00-00',
  `horaalta` time DEFAULT NULL,
  `fechamodi` date NOT NULL DEFAULT '0000-00-00',
  `horamodi` time DEFAULT NULL,
  `fechagas` date NOT NULL DEFAULT '0000-00-00',
  `descuento` decimal(16,6) DEFAULT NULL,
  `subtotal` decimal(16,6) DEFAULT NULL,
  `anticipo` decimal(16,2) DEFAULT NULL,
  `fechainic` date NOT NULL DEFAULT '0000-00-00',
  `fechavenc` date NOT NULL DEFAULT '0000-00-00',
  `porcint` decimal(5,2) DEFAULT NULL,
  `valor` decimal(16,2) DEFAULT NULL,
  `letras` int(2) DEFAULT NULL,
  `plazo` int(3) NOT NULL DEFAULT 0,
  `periodic` varchar(3) DEFAULT NULL,
  `docdepart` varchar(10) NOT NULL DEFAULT '',
  `terminal` varchar(10) NOT NULL DEFAULT '',
  `docseccion` varchar(4) NOT NULL DEFAULT '',
  `tienexml` tinyint(1) NOT NULL DEFAULT 1,
  `muuid` varchar(36) DEFAULT NULL,
  `redondeoantiguo` tinyint(1) NOT NULL DEFAULT 1,
  `tipogasto` int(11) NOT NULL DEFAULT 1,
  `viaembarq` varchar(10) DEFAULT NULL,
  `impuestoret` int(11) DEFAULT NULL,
  `totalret` decimal(16,2) NOT NULL DEFAULT 0.00,
  `numcuenta` varchar(30) DEFAULT NULL,
  `refpagprov` varchar(25) DEFAULT NULL,
  PRIMARY KEY (`referencia`),
  KEY `fechagas` (`fechagas`),
  KEY `proveedor` (`proveedor`),
  KEY `sucursal` (`sucursal`),
  KEY `solicita` (`solicita`),
  KEY `autoriza` (`autoriza`),
  KEY `usualta` (`usualta`),
  KEY `usumodi` (`usumodi`),
  KEY `docdepart` (`docdepart`),
  KEY `terminal` (`terminal`),
  KEY `docseccion` (`docseccion`),
  KEY `muuid` (`muuid`),
  KEY `fechamodi` (`fechamodi`),
  KEY `gastos_ibfk_10` (`depart`),
  KEY `gastos_ibfk_11` (`viaembarq`),
  KEY `gastos_ibfk_12` (`tipogasto`),
  KEY `gastos_ibfk_13` (`impuestoret`),
  KEY `gastos_ibfk_14` (`numcuenta`),
  CONSTRAINT `gastos_ibfk_1` FOREIGN KEY (`proveedor`) REFERENCES `proveedores` (`proveedor`) ON UPDATE CASCADE,
  CONSTRAINT `gastos_ibfk_10` FOREIGN KEY (`depart`) REFERENCES `departamentos` (`depart`) ON UPDATE CASCADE,
  CONSTRAINT `gastos_ibfk_11` FOREIGN KEY (`viaembarq`) REFERENCES `viasembarque` (`viaembarq`) ON UPDATE CASCADE,
  CONSTRAINT `gastos_ibfk_12` FOREIGN KEY (`tipogasto`) REFERENCES `tiposdegastos` (`tipo`) ON UPDATE CASCADE,
  CONSTRAINT `gastos_ibfk_13` FOREIGN KEY (`impuestoret`) REFERENCES `impuestos` (`impuesto`) ON UPDATE CASCADE,
  CONSTRAINT `gastos_ibfk_2` FOREIGN KEY (`sucursal`) REFERENCES `sucursales` (`sucursal`) ON UPDATE CASCADE,
  CONSTRAINT `gastos_ibfk_3` FOREIGN KEY (`solicita`) REFERENCES `empleados` (`empleado`) ON UPDATE CASCADE,
  CONSTRAINT `gastos_ibfk_4` FOREIGN KEY (`autoriza`) REFERENCES `empleados` (`empleado`) ON UPDATE CASCADE,
  CONSTRAINT `gastos_ibfk_5` FOREIGN KEY (`usualta`) REFERENCES `usuarios` (`empleado`) ON UPDATE CASCADE,
  CONSTRAINT `gastos_ibfk_6` FOREIGN KEY (`usumodi`) REFERENCES `usuarios` (`empleado`) ON UPDATE CASCADE,
  CONSTRAINT `gastos_ibfk_7` FOREIGN KEY (`docdepart`) REFERENCES `departamentos` (`depart`) ON UPDATE CASCADE,
  CONSTRAINT `gastos_ibfk_8` FOREIGN KEY (`terminal`) REFERENCES `terminales` (`terminal`) ON UPDATE CASCADE,
  CONSTRAINT `gastos_ibfk_9` FOREIGN KEY (`docseccion`) REFERENCES `secciones` (`seccion`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

