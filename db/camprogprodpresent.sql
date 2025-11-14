/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `camprogprodpresent`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `camprogprodpresent` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `tipo` varchar(4) NOT NULL CHECK (`tipo` in ('RC','EA','CPP','MPRO','MPRE','MMUL')),
  `articulo` varchar(9) DEFAULT NULL,
  `prod_origen` varchar(8) DEFAULT NULL,
  `prod_destino` varchar(8) DEFAULT NULL,
  `present_origen` varchar(13) DEFAULT NULL,
  `present_destino` varchar(13) DEFAULT NULL,
  `mult_origen` varchar(10) DEFAULT NULL,
  `mult_destino` varchar(10) DEFAULT NULL,
  `factor_nuevo` decimal(10,3) DEFAULT NULL,
  `ean13` varchar(14) DEFAULT NULL,
  `nombre_origen` varchar(60) DEFAULT NULL,
  `nombre_destino` varchar(60) DEFAULT NULL,
  `usuario` varchar(10) NOT NULL,
  `fecha_prog` datetime NOT NULL DEFAULT current_timestamp(),
  `suc_prog` varchar(2) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `ind_bpp_tipo` (`tipo`) USING BTREE,
  KEY `ind_bpp_prod_destino` (`prod_destino`) USING BTREE,
  KEY `ind_bpp_present_destino` (`present_destino`) USING BTREE,
  KEY `ind_bpp_mult_origen` (`mult_origen`) USING BTREE,
  KEY `ind_bpp_mult_destino` (`mult_destino`) USING BTREE,
  KEY `cppp_fk_art` (`articulo`),
  KEY `´cppp_fk_prod´` (`prod_origen`,`present_origen`),
  CONSTRAINT `cppp_fk_art` FOREIGN KEY (`articulo`) REFERENCES `articulos` (`articulo`) ON UPDATE CASCADE,
  CONSTRAINT `´cppp_fk_prod´` FOREIGN KEY (`prod_origen`, `present_origen`) REFERENCES `presentaciones` (`producto`, `present`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

