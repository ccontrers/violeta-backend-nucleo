/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `surtidoartreemplazados`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `surtidoartreemplazados` (
  `surtido` varchar(11) NOT NULL DEFAULT '0',
  `pedido` varchar(11) NOT NULL,
  `division` tinyint(3) unsigned NOT NULL DEFAULT 1,
  `articuloped` varchar(9) NOT NULL,
  `articulosurt` varchar(9) NOT NULL,
  PRIMARY KEY (`surtido`,`division`,`articuloped`,`articulosurt`) USING BTREE,
  KEY `surtido` (`surtido`) USING BTREE,
  KEY `division` (`division`) USING BTREE,
  KEY `articuloped` (`articuloped`) USING BTREE,
  KEY `articulosurt` (`articulosurt`) USING BTREE,
  KEY `surtidoartreemplazados_ibfk_4` (`pedido`),
  CONSTRAINT `surtidoartreemplazados_ibfk_1` FOREIGN KEY (`surtido`) REFERENCES `surtidopedidos` (`surtido`) ON UPDATE CASCADE,
  CONSTRAINT `surtidoartreemplazados_ibfk_2` FOREIGN KEY (`articuloped`) REFERENCES `articulos` (`articulo`) ON UPDATE CASCADE,
  CONSTRAINT `surtidoartreemplazados_ibfk_3` FOREIGN KEY (`articulosurt`) REFERENCES `articulos` (`articulo`) ON UPDATE CASCADE,
  CONSTRAINT `surtidoartreemplazados_ibfk_4` FOREIGN KEY (`pedido`) REFERENCES `pedidosventa` (`referencia`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

