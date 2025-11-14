/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `darticulospedidossinexistencia`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `darticulospedidossinexistencia` (
  `articulo_local` varchar(9) NOT NULL DEFAULT '',
  `articulo_remoto` varchar(9) NOT NULL DEFAULT '',
  `existencias_local` decimal(12,3) DEFAULT NULL COMMENT 'existencias local al calcular el pedido',
  `cantidad_calculada` decimal(12,3) DEFAULT NULL COMMENT 'cantidad local al calcular el pedido',
  `existencias_remoto` decimal(12,3) DEFAULT NULL COMMENT 'existencias remota al checar las equivalencias',
  `cantidad_pedida` decimal(12,3) DEFAULT NULL COMMENT 'cantidad que se pedira al final junto con la modificacion del usuario',
  `usuario` varchar(10) DEFAULT NULL,
  `fechamodi` date NOT NULL DEFAULT '0000-00-00',
  `horamodi` time NOT NULL,
  `sucursalsolicita` varchar(2) DEFAULT '',
  `sucursalremota` varchar(2) DEFAULT '',
  PRIMARY KEY (`articulo_local`,`fechamodi`,`horamodi`) USING BTREE,
  KEY `articulo` (`articulo_local`) USING BTREE,
  CONSTRAINT `ariculo_ local_FK1` FOREIGN KEY (`articulo_local`) REFERENCES `articulos` (`articulo`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

