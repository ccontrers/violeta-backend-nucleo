/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `pedidoautomatico_presentaciones_empresa`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `pedidoautomatico_presentaciones_empresa` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `empresa` int(2) NOT NULL,
  `producto` varchar(8) NOT NULL,
  `present` varchar(13) NOT NULL,
  `proveedor` varchar(11) DEFAULT NULL,
  `duracionreorden` smallint(5) unsigned DEFAULT NULL,
  `multiplopedir` varchar(9) DEFAULT NULL,
  PRIMARY KEY (`id`) USING BTREE,
  UNIQUE KEY `empresa_producto_present` (`empresa`,`producto`,`present`) USING BTREE,
  KEY `fk_multiplopedir_articulos` (`multiplopedir`),
  KEY `fk_present_pedidoautomaticoxemp` (`producto`),
  KEY `fk_prov_pedidoautomaticoxemp` (`proveedor`),
  CONSTRAINT `fk_emp_pedidoautomaticoxemp` FOREIGN KEY (`empresa`) REFERENCES `empresas` (`idempresa`) ON UPDATE CASCADE,
  CONSTRAINT `fk_multiplopedir_articulos` FOREIGN KEY (`multiplopedir`) REFERENCES `articulos` (`articulo`) ON UPDATE CASCADE,
  CONSTRAINT `fk_present_pedidoautomaticoxemp` FOREIGN KEY (`producto`) REFERENCES `presentaciones` (`producto`) ON UPDATE CASCADE,
  CONSTRAINT `fk_prov_pedidoautomaticoxemp` FOREIGN KEY (`proveedor`) REFERENCES `proveedores` (`proveedor`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

