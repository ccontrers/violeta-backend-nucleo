/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `bitacoratiposprecios`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `bitacoratiposprecios` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `tipoprec` varchar(4) DEFAULT NULL,
  `descripcion` varchar(40) DEFAULT NULL,
  `descripcionant` varchar(40) DEFAULT NULL,
  `porcutil` decimal(16,2) DEFAULT NULL,
  `porcutilant` decimal(16,2) DEFAULT NULL,
  `porcutil2` decimal(16,2) DEFAULT NULL,
  `porcutil2ant` decimal(16,2) DEFAULT NULL,
  `porcutil3` decimal(16,2) DEFAULT NULL,
  `porcutil3ant` decimal(16,2) DEFAULT NULL,
  `porcutil4` decimal(16,2) DEFAULT NULL,
  `porcutil4ant` decimal(16,2) DEFAULT NULL,
  `porcutil5` decimal(16,2) DEFAULT NULL,
  `porcutil5ant` decimal(16,2) DEFAULT NULL,
  `listamovil` tinyint(1) DEFAULT NULL,
  `listamovilant` tinyint(1) DEFAULT NULL,
  `verventmayoreo` tinyint(1) DEFAULT NULL,
  `verventmayoreoant` tinyint(1) DEFAULT NULL,
  `verprecdif` tinyint(1) DEFAULT NULL,
  `verprecdifant` tinyint(1) DEFAULT NULL,
  `usuario` varchar(10) DEFAULT NULL,
  `fecha` date NOT NULL DEFAULT '0000-00-00',
  `hora` time NOT NULL DEFAULT '00:00:00',
  PRIMARY KEY (`id`) USING BTREE,
  KEY `tipoprec` (`tipoprec`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

