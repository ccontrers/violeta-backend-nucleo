/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `pedidos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `pedidos` (
  `referencia` varchar(11) NOT NULL DEFAULT '0',
  `foliofactura` varchar(255) DEFAULT NULL,
  `cancelado` tinyint(1) DEFAULT NULL,
  `acredito` tinyint(1) DEFAULT NULL,
  `proveedor` varchar(11) DEFAULT NULL,
  `almacen` varchar(4) DEFAULT NULL,
  `comprador` varchar(10) DEFAULT NULL,
  `usualta` varchar(10) DEFAULT NULL,
  `usumodi` varchar(10) DEFAULT NULL,
  `fechaalta` date NOT NULL DEFAULT '0000-00-00',
  `horaalta` time DEFAULT NULL,
  `fechamodi` date NOT NULL DEFAULT '0000-00-00',
  `horamodi` time DEFAULT NULL,
  `fechaped` date NOT NULL DEFAULT '0000-00-00',
  `fechapromesa` date NOT NULL DEFAULT '0000-00-00',
  `fechaagenda` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `fechaaduana` date NOT NULL DEFAULT '0000-00-00',
  `descuento` decimal(5,2) DEFAULT NULL,
  `porcint` decimal(5,2) DEFAULT NULL,
  `valor` decimal(16,2) DEFAULT NULL,
  `letras` int(2) DEFAULT NULL,
  `facturado` tinyint(1) NOT NULL DEFAULT 0,
  `periodic` varchar(3) DEFAULT NULL,
  `compra` varchar(11) DEFAULT NULL,
  `redondeoantiguo` tinyint(1) NOT NULL DEFAULT 1,
  `impuestoret` int(11) DEFAULT NULL,
  `totalret` decimal(16,2) NOT NULL DEFAULT 0.00,
  `fechavigencia` date NOT NULL DEFAULT '0000-00-00',
  `estatus` tinyint(4) NOT NULL DEFAULT 1,
  `origen` varchar(4) DEFAULT NULL,
  PRIMARY KEY (`referencia`),
  UNIQUE KEY `compra` (`compra`),
  KEY `fechaped` (`fechaped`),
  KEY `almacen` (`almacen`),
  KEY `proveedor` (`proveedor`),
  KEY `comprador` (`comprador`),
  KEY `usualta` (`usualta`),
  KEY `usumodi` (`usumodi`),
  KEY `pedidos_ibfk_5` (`impuestoret`),
  CONSTRAINT `pedidos_ibfk_1` FOREIGN KEY (`proveedor`) REFERENCES `proveedores` (`proveedor`) ON UPDATE CASCADE,
  CONSTRAINT `pedidos_ibfk_2` FOREIGN KEY (`comprador`) REFERENCES `compradores` (`empleado`) ON UPDATE CASCADE,
  CONSTRAINT `pedidos_ibfk_3` FOREIGN KEY (`usualta`) REFERENCES `usuarios` (`empleado`) ON UPDATE CASCADE,
  CONSTRAINT `pedidos_ibfk_4` FOREIGN KEY (`usumodi`) REFERENCES `usuarios` (`empleado`) ON UPDATE CASCADE,
  CONSTRAINT `pedidos_ibfk_5` FOREIGN KEY (`impuestoret`) REFERENCES `impuestos` (`impuesto`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

