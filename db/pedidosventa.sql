/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `pedidosventa`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `pedidosventa` (
  `referencia` varchar(11) NOT NULL DEFAULT '',
  `tipofac` varchar(11) NOT NULL DEFAULT '',
  `cancelado` tinyint(1) DEFAULT NULL,
  `facturado` tinyint(1) DEFAULT NULL,
  `acredito` tinyint(1) DEFAULT NULL,
  `termino` varchar(5) DEFAULT '',
  `cliente` varchar(11) DEFAULT NULL,
  `vendedor` varchar(10) DEFAULT NULL,
  `usualta` varchar(10) DEFAULT NULL,
  `usumodi` varchar(10) DEFAULT NULL,
  `fechaalta` date NOT NULL DEFAULT '0000-00-00',
  `horaalta` time DEFAULT NULL,
  `fechamodi` date NOT NULL DEFAULT '0000-00-00',
  `horamodi` time DEFAULT NULL,
  `fechaped` date NOT NULL DEFAULT '0000-00-00',
  `fechasurt` date NOT NULL DEFAULT '0000-00-00',
  `porcint` decimal(5,2) DEFAULT NULL,
  `valor` decimal(16,2) DEFAULT NULL,
  `letras` int(2) DEFAULT NULL,
  `periodic` varchar(3) DEFAULT NULL,
  `comisionada` tinyint(1) DEFAULT 0,
  `embarque` varchar(11) DEFAULT '',
  `plazo` int(3) DEFAULT NULL,
  `cotizacion` tinyint(1) DEFAULT 0,
  `terminal` varchar(11) DEFAULT NULL,
  `kit` varchar(11) NOT NULL DEFAULT '',
  `cantkits` int(5) NOT NULL DEFAULT 0,
  `almacen` varchar(4) DEFAULT NULL,
  `pedimpreso` tinyint(1) DEFAULT 0,
  `venta` varchar(11) DEFAULT NULL,
  `usocfdi33` varchar(4) DEFAULT NULL,
  `formapago33` varchar(3) DEFAULT NULL,
  `redondeoantiguo` tinyint(1) NOT NULL DEFAULT 1,
  `embarquediv` tinyint(1) unsigned NOT NULL DEFAULT 1,
  `tipoorigen` varchar(5) NOT NULL DEFAULT 'MOSTR',
  `folioapp` varchar(11) DEFAULT NULL,
  `kiosko` tinyint(1) NOT NULL DEFAULT 0,
  `cotzEspecial` tinyint(4) NOT NULL DEFAULT 0,
  PRIMARY KEY (`referencia`),
  KEY `cliente` (`cliente`),
  KEY `fechaped` (`fechaped`),
  KEY `embarque` (`embarque`),
  KEY `vendedor` (`vendedor`),
  KEY `usualta` (`usualta`),
  KEY `usumodi` (`usumodi`),
  KEY `terminal` (`terminal`),
  KEY `almacen` (`almacen`),
  KEY `venta` (`venta`),
  KEY `FK_pedidosventa_tipoorigen` (`tipoorigen`),
  KEY `fechaalta` (`fechaalta`),
  CONSTRAINT `FK_pedidosventa_almacen` FOREIGN KEY (`almacen`) REFERENCES `almacenes` (`almacen`) ON UPDATE CASCADE,
  CONSTRAINT `FK_pedidosventa_tipoorigen` FOREIGN KEY (`tipoorigen`) REFERENCES `tiposorigenventas` (`tipoorigen`) ON UPDATE CASCADE,
  CONSTRAINT `pedidosventa_ibfk_1` FOREIGN KEY (`cliente`) REFERENCES `clientes` (`cliente`) ON UPDATE CASCADE,
  CONSTRAINT `pedidosventa_ibfk_2` FOREIGN KEY (`vendedor`) REFERENCES `vendedores` (`empleado`) ON UPDATE CASCADE,
  CONSTRAINT `pedidosventa_ibfk_3` FOREIGN KEY (`usualta`) REFERENCES `usuarios` (`empleado`) ON UPDATE CASCADE,
  CONSTRAINT `pedidosventa_ibfk_4` FOREIGN KEY (`usumodi`) REFERENCES `usuarios` (`empleado`) ON UPDATE CASCADE,
  CONSTRAINT `pedidosventa_ibfk_5` FOREIGN KEY (`terminal`) REFERENCES `terminales` (`terminal`) ON UPDATE CASCADE,
  CONSTRAINT `pedidosventa_ibfk_6` FOREIGN KEY (`venta`) REFERENCES `ventas` (`referencia`) ON UPDATE CASCADE,
  CONSTRAINT `pedidosventa_ibfk_9` FOREIGN KEY (`embarque`) REFERENCES `embarques` (`embarque`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

