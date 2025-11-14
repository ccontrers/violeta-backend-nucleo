/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `articulospedsupers`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `articulospedsupers` (
  `sucursal` varchar(2) NOT NULL,
  `producto` varchar(8) NOT NULL DEFAULT '',
  `present` varchar(13) NOT NULL DEFAULT '',
  `proveedor` varchar(11) DEFAULT NULL,
  `claveproductoproveedor` varchar(15) DEFAULT NULL,
  `duracionreorden` decimal(5,2) NOT NULL DEFAULT 7.00,
  `duracionmax` decimal(5,2) NOT NULL DEFAULT 15.00,
  `descontinuado` tinyint(4) NOT NULL DEFAULT 0,
  `redondeocaja` tinyint(4) NOT NULL DEFAULT 0,
  `multiplopedir` varchar(10) DEFAULT NULL,
  `stockminimo` tinyint(4) NOT NULL DEFAULT 0,
  PRIMARY KEY (`sucursal`,`producto`,`present`) USING BTREE,
  KEY `prodpred` (`producto`,`present`) USING BTREE,
  KEY `prov` (`proveedor`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

