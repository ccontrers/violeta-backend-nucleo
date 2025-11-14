/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `precalculocostos1`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `precalculocostos1` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `referencia` varchar(11) DEFAULT NULL,
  `producto` varchar(8) DEFAULT NULL,
  `present` varchar(13) DEFAULT NULL,
  `tipo` varchar(2) DEFAULT NULL,
  `clasif` varchar(1) DEFAULT NULL,
  `cantidad` double DEFAULT NULL,
  `costounit` double DEFAULT NULL,
  `fecha` date DEFAULT NULL,
  `sector` varchar(10) DEFAULT NULL,
  `localidad` varchar(4) DEFAULT NULL,
  `vendedor` varchar(10) DEFAULT NULL,
  `tipofac` varchar(11) DEFAULT NULL,
  `cliente` varchar(11) DEFAULT NULL,
  `acredito` tinyint(1) DEFAULT NULL,
  `clasif1` varchar(10) DEFAULT NULL,
  `clasif2` varchar(10) DEFAULT NULL,
  `clasif3` varchar(10) DEFAULT NULL,
  `almacen` varchar(4) DEFAULT NULL,
  `tipomov` varchar(1) DEFAULT NULL,
  `conceptomov` varchar(4) DEFAULT NULL,
  `activo` tinyint(1) DEFAULT NULL,
  PRIMARY KEY (`id`) USING BTREE,
  KEY `nomproducto` (`producto`,`present`,`fecha`) USING BTREE,
  KEY `prodpres` (`producto`) USING BTREE,
  KEY `clasif1` (`clasif1`) USING BTREE,
  KEY `clasif2` (`clasif2`) USING BTREE,
  KEY `clasif3` (`clasif3`) USING BTREE,
  KEY `fecha` (`fecha`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

