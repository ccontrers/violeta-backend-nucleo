/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `dsurtidopedidos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `dsurtidopedidos` (
  `pedido` varchar(11) NOT NULL,
  `surtido` varchar(11) NOT NULL DEFAULT '0',
  `articulo` varchar(9) NOT NULL,
  `cantidad` decimal(12,3) NOT NULL DEFAULT 0.000,
  `observacion` text DEFAULT NULL,
  `reemplazado` tinyint(4) NOT NULL DEFAULT 0,
  `division` tinyint(3) unsigned NOT NULL DEFAULT 1,
  `almacen` varchar(4) NOT NULL DEFAULT '',
  PRIMARY KEY (`surtido`,`articulo`,`division`,`almacen`,`pedido`) USING BTREE,
  KEY `surtido` (`surtido`) USING BTREE,
  KEY `articulo` (`articulo`) USING BTREE,
  KEY `dsurtidopedidos_ibfk_3` (`pedido`),
  CONSTRAINT `dsurtidopedidos_ibfk_1` FOREIGN KEY (`surtido`) REFERENCES `surtidopedidos` (`surtido`) ON UPDATE CASCADE,
  CONSTRAINT `dsurtidopedidos_ibfk_2` FOREIGN KEY (`articulo`) REFERENCES `articulos` (`articulo`) ON UPDATE CASCADE,
  CONSTRAINT `dsurtidopedidos_ibfk_3` FOREIGN KEY (`pedido`) REFERENCES `pedidosventa` (`referencia`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

