/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `unif_precios`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `unif_precios` (
  `articulo` varchar(9) NOT NULL DEFAULT '0',
  `tipoprec` varchar(2) NOT NULL DEFAULT '',
  `costo` decimal(16,6) DEFAULT NULL,
  `precio` decimal(16,6) DEFAULT NULL,
  `porcutil` decimal(6,2) DEFAULT NULL,
  `fechamodi` date NOT NULL DEFAULT '0000-00-00',
  `preciofechmod` decimal(16,6) DEFAULT NULL,
  `precioproximo` decimal(16,6) DEFAULT NULL,
  `preciomod` decimal(16,6) DEFAULT NULL,
  `horamodi` time NOT NULL DEFAULT '00:00:00',
  `fechamodiprox` date NOT NULL DEFAULT '0000-00-00',
  `horamodiprox` time NOT NULL DEFAULT '00:00:00',
  `actualizarpendiente` tinyint(4) NOT NULL DEFAULT 0,
  PRIMARY KEY (`articulo`,`tipoprec`) USING BTREE,
  KEY `tipoprec` (`tipoprec`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

