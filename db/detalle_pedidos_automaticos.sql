/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `detalle_pedidos_automaticos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `detalle_pedidos_automaticos` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `producto` varchar(8) NOT NULL,
  `present` varchar(13) NOT NULL,
  `cantidad` decimal(12,3) NOT NULL,
  `costobase` decimal(16,6) DEFAULT 0.000000,
  `costoultimoimp` decimal(16,6) DEFAULT 0.000000,
  `costoultimo` decimal(16,6) DEFAULT 0.000000,
  `id_pedidoauto` int(11) DEFAULT NULL,
  `existenciamayor` int(11) NOT NULL DEFAULT 0,
  `ventapromediomayor` decimal(12,3) NOT NULL DEFAULT 0.000,
  `duracionexistencia` decimal(12,3) NOT NULL DEFAULT 0.000,
  PRIMARY KEY (`id`,`producto`,`present`) USING BTREE,
  KEY `dpedaut_ibfk_1` (`id`) USING BTREE,
  KEY `dpedaut_ibfk_2` (`producto`,`present`) USING BTREE,
  KEY `dpedaut_ibfk_3` (`id_pedidoauto`) USING BTREE,
  CONSTRAINT `dpedaut_ibfk_2` FOREIGN KEY (`producto`, `present`) REFERENCES `presentaciones` (`producto`, `present`) ON UPDATE CASCADE,
  CONSTRAINT `dpedaut_ibfk_3` FOREIGN KEY (`id_pedidoauto`) REFERENCES `pedidos_automaticos` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

