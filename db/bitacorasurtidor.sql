/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `bitacorasurtidor`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `bitacorasurtidor` (
  `id_surtidor` int(11) NOT NULL AUTO_INCREMENT,
  `embarque` varchar(11) DEFAULT NULL,
  `usumodi` varchar(10) DEFAULT NULL,
  `fechamodi` date DEFAULT NULL,
  `horamodi` time DEFAULT NULL,
  `surt_antes` varchar(10) DEFAULT NULL,
  `surt_desp` varchar(10) DEFAULT NULL,
  `evento` varchar(1) DEFAULT NULL,
  `sucursal` varchar(10) DEFAULT NULL,
  `terminal` varchar(10) DEFAULT NULL,
  `formulario` varchar(3) DEFAULT NULL,
  PRIMARY KEY (`id_surtidor`),
  KEY `embarque` (`embarque`),
  KEY `usumodi` (`usumodi`),
  KEY `surt_antes` (`surt_antes`),
  KEY `surt_desp` (`surt_desp`),
  KEY `sucursal` (`sucursal`),
  KEY `terminal` (`terminal`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_spanish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

