/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `ventas`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ventas` (
  `referencia` varchar(11) NOT NULL DEFAULT '0',
  `foliofisic` varchar(11) NOT NULL DEFAULT '',
  `tipofac` varchar(11) NOT NULL DEFAULT '',
  `terminal` varchar(10) NOT NULL DEFAULT '',
  `cancelado` tinyint(1) DEFAULT NULL,
  `acredito` tinyint(1) DEFAULT NULL,
  `cliente` varchar(11) DEFAULT NULL,
  `vendedor` varchar(10) DEFAULT NULL,
  `embarque` varchar(11) DEFAULT NULL,
  `usuauto` varchar(10) DEFAULT NULL,
  `usualta` varchar(10) DEFAULT NULL,
  `usumodi` varchar(10) DEFAULT NULL,
  `fechaalta` date NOT NULL DEFAULT '0000-00-00',
  `horaalta` time DEFAULT NULL,
  `fechamodi` date NOT NULL DEFAULT '0000-00-00',
  `horamodi` time DEFAULT NULL,
  `fechavta` date NOT NULL DEFAULT '0000-00-00',
  `horavta` time DEFAULT NULL,
  `fechainic` date NOT NULL DEFAULT '0000-00-00',
  `fechavenc` date NOT NULL DEFAULT '0000-00-00',
  `porcint` decimal(5,2) DEFAULT NULL,
  `valor` decimal(16,2) DEFAULT NULL,
  `anticipo` decimal(16,2) NOT NULL DEFAULT 0.00,
  `letras` int(2) DEFAULT NULL,
  `periodic` varchar(3) DEFAULT NULL,
  `plazo` int(3) NOT NULL DEFAULT 0,
  `comisionada` tinyint(1) NOT NULL DEFAULT 0,
  `cobrador` varchar(11) NOT NULL DEFAULT '',
  `totcomision` decimal(16,4) NOT NULL DEFAULT 0.0000,
  `tolercomision` int(3) NOT NULL DEFAULT 0,
  `ticket` tinyint(1) DEFAULT 0,
  `folioticket` varchar(21) DEFAULT '',
  `kit` varchar(11) NOT NULL DEFAULT '',
  `cantkits` int(5) NOT NULL DEFAULT 0,
  `ubicacion` varchar(10) NOT NULL DEFAULT '',
  `ventasuper` tinyint(1) DEFAULT 0,
  `almacen` varchar(4) DEFAULT NULL,
  `resumirart` tinyint(1) DEFAULT 0,
  `conceptoresum` varchar(35) DEFAULT NULL,
  `redondeoantiguo` tinyint(1) NOT NULL DEFAULT 1,
  `embarquediv` tinyint(1) unsigned NOT NULL DEFAULT 1,
  `tipoorigen` varchar(5) NOT NULL DEFAULT 'MOSTR',
  `corte` varchar(15) DEFAULT NULL,
  `cortecancel` varchar(15) DEFAULT NULL,
  `terminalcancel` varchar(10) DEFAULT NULL,
  PRIMARY KEY (`referencia`),
  KEY `cliente` (`cliente`),
  KEY `terminal` (`terminal`),
  KEY `foliofisic` (`foliofisic`),
  KEY `fechavta` (`fechavta`),
  KEY `embarque` (`embarque`),
  KEY `tipofac` (`tipofac`),
  KEY `vendedor` (`vendedor`),
  KEY `usualta` (`usualta`),
  KEY `usumodi` (`usumodi`),
  KEY `cobrador` (`cobrador`),
  KEY `almacen` (`almacen`),
  KEY `ubicacion` (`ubicacion`),
  KEY `fechamodi` (`fechamodi`),
  KEY `FK_ventas_tipoorigen` (`tipoorigen`),
  KEY `corte` (`corte`),
  CONSTRAINT `FK_ventas_almacen` FOREIGN KEY (`almacen`) REFERENCES `almacenes` (`almacen`) ON UPDATE CASCADE,
  CONSTRAINT `FK_ventas_tipoorigen` FOREIGN KEY (`tipoorigen`) REFERENCES `tiposorigenventas` (`tipoorigen`) ON UPDATE CASCADE,
  CONSTRAINT `ventas_ibfk_1` FOREIGN KEY (`cliente`) REFERENCES `clientes` (`cliente`) ON UPDATE CASCADE,
  CONSTRAINT `ventas_ibfk_10` FOREIGN KEY (`cobrador`) REFERENCES `cobradores` (`empleado`) ON UPDATE CASCADE,
  CONSTRAINT `ventas_ibfk_2` FOREIGN KEY (`tipofac`) REFERENCES `tiposfacturasventas` (`tipo`) ON UPDATE CASCADE,
  CONSTRAINT `ventas_ibfk_3` FOREIGN KEY (`terminal`) REFERENCES `terminales` (`terminal`) ON UPDATE CASCADE,
  CONSTRAINT `ventas_ibfk_5` FOREIGN KEY (`vendedor`) REFERENCES `vendedores` (`empleado`) ON UPDATE CASCADE,
  CONSTRAINT `ventas_ibfk_6` FOREIGN KEY (`usualta`) REFERENCES `usuarios` (`empleado`) ON UPDATE CASCADE,
  CONSTRAINT `ventas_ibfk_7` FOREIGN KEY (`usumodi`) REFERENCES `usuarios` (`empleado`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci COMMENT='InnoDB free: 117760 kB';
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

