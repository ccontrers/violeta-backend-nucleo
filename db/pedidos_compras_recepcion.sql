/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `pedidos_compras_recepcion`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `pedidos_compras_recepcion` (
  `pedido` varchar(11) NOT NULL DEFAULT '0',
  `recepcion` varchar(11) NOT NULL DEFAULT '0',
  `compra` varchar(11) NOT NULL DEFAULT '0',
  `descripcion` varchar(50) DEFAULT NULL,
  PRIMARY KEY (`pedido`,`recepcion`,`compra`) USING BTREE,
  KEY `pedidos_recpcion_ibfk_2` (`recepcion`),
  KEY `pedidos_recpcion_ibfk_3` (`compra`),
  CONSTRAINT `pedidos_recpcion_ibfk_1` FOREIGN KEY (`pedido`) REFERENCES `pedidos` (`referencia`) ON UPDATE CASCADE,
  CONSTRAINT `pedidos_recpcion_ibfk_2` FOREIGN KEY (`recepcion`) REFERENCES `recepciones` (`recepcion`) ON UPDATE CASCADE,
  CONSTRAINT `pedidos_recpcion_ibfk_3` FOREIGN KEY (`compra`) REFERENCES `compras` (`referencia`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

