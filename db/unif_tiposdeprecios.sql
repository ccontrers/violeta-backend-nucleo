/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `unif_tiposdeprecios`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `unif_tiposdeprecios` (
  `tipoprec` varchar(2) NOT NULL DEFAULT '',
  `descripcion` varchar(40) NOT NULL DEFAULT '',
  `porcutil` decimal(16,2) DEFAULT NULL,
  `porcutil2` decimal(16,2) DEFAULT NULL,
  `porcutil3` decimal(16,2) DEFAULT NULL,
  `porcutil4` decimal(16,2) DEFAULT NULL,
  `porcutil5` decimal(16,2) DEFAULT NULL,
  `listamovil` tinyint(1) DEFAULT 0,
  `verventmayoreo` tinyint(1) DEFAULT 1,
  `verprecdif` tinyint(1) DEFAULT 1,
  `idempresa` int(2) DEFAULT NULL,
  PRIMARY KEY (`tipoprec`) USING BTREE,
  KEY `descripcion` (`descripcion`) USING BTREE,
  KEY `tiposdepreciosemp_ibfk_1` (`idempresa`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

