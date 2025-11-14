/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `pedidosecomm`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `pedidosecomm` (
  `referencia` varchar(11) NOT NULL DEFAULT '',
  `numorden` int(11) NOT NULL DEFAULT 0,
  `tipofac` varchar(11) NOT NULL DEFAULT '',
  `cancelado` tinyint(1) DEFAULT NULL,
  `tipocompra` tinyint(1) DEFAULT NULL,
  `entregagratis` tinyint(1) NOT NULL DEFAULT 0,
  `cliente` varchar(11) DEFAULT NULL,
  `fechaalta` date NOT NULL DEFAULT '0000-00-00',
  `horaalta` time DEFAULT NULL,
  `fechamodi` date NOT NULL DEFAULT '0000-00-00',
  `horamodi` time DEFAULT NULL,
  `fechaped` date NOT NULL DEFAULT '0000-00-00',
  `horaped` time DEFAULT NULL,
  `fechasurt` date NOT NULL DEFAULT '0000-00-00',
  `horasurt` time DEFAULT NULL,
  `fechaentrega` date DEFAULT '0000-00-00',
  `horaentrega` varchar(20) DEFAULT NULL,
  `valor` decimal(16,2) DEFAULT NULL,
  `sucursal` varchar(2) DEFAULT NULL,
  `venta` varchar(11) DEFAULT NULL,
  `folioapp` varchar(11) DEFAULT NULL,
  `cupon` varchar(11) DEFAULT NULL,
  `descuento` double(16,2) DEFAULT NULL,
  `costoentrega` double(16,2) DEFAULT NULL,
  `calle` varchar(60) DEFAULT NULL,
  `numext` varchar(10) DEFAULT NULL,
  `numint` varchar(10) DEFAULT NULL,
  `colonia` varchar(50) DEFAULT NULL,
  `codp` varchar(5) DEFAULT NULL,
  `ubicaciongis` point DEFAULT NULL,
  `referenciadom` varchar(100) DEFAULT NULL,
  `estadopago` int(11) DEFAULT NULL,
  `estadopedido` int(11) DEFAULT NULL,
  `mensaje` varchar(250) DEFAULT NULL,
  `ciudad` varchar(50) DEFAULT NULL,
  `estado` varchar(25) DEFAULT NULL,
  `corte` varchar(15) DEFAULT NULL,
  PRIMARY KEY (`referencia`),
  KEY `cliente` (`cliente`),
  KEY `fechaped` (`fechaped`),
  KEY `sucursal` (`sucursal`),
  KEY `venta` (`venta`),
  KEY `estado` (`estado`),
  KEY `estadopedido` (`estadopedido`),
  KEY `estadopago` (`estadopago`),
  CONSTRAINT `pedidosecomm_ibfk_1` FOREIGN KEY (`cliente`) REFERENCES `clientes` (`cliente`) ON UPDATE CASCADE,
  CONSTRAINT `pedidosecomm_ibfk_2` FOREIGN KEY (`sucursal`) REFERENCES `sucursales` (`sucursal`) ON UPDATE CASCADE,
  CONSTRAINT `pedidosecomm_ibfk_3` FOREIGN KEY (`venta`) REFERENCES `ventas` (`referencia`) ON UPDATE CASCADE,
  CONSTRAINT `pedidosecomm_ibfk_4` FOREIGN KEY (`estado`) REFERENCES `cestados` (`c_estado`) ON UPDATE CASCADE,
  CONSTRAINT `pedidosecomm_ibfk_5` FOREIGN KEY (`estadopedido`) REFERENCES `tipoestatus` (`id`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

