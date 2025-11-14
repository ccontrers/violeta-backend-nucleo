/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `bitacoracontpaqi`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `bitacoracontpaqi` (
  `id_bitacora_contpaqi` int(11) NOT NULL AUTO_INCREMENT,
  `sucursal_poliza` varchar(5) DEFAULT NULL,
  `usuario` varchar(10) DEFAULT NULL,
  `fecha` datetime DEFAULT NULL,
  `nPoliza` varchar(50) DEFAULT NULL,
  `GuidPoliza` varchar(50) DEFAULT NULL,
  `tipo_poliza` varchar(50) DEFAULT NULL,
  `folio_asociado` varchar(30) DEFAULT NULL,
  `tipo_folio_asociado` varchar(30) DEFAULT NULL,
  `muuid_asociado` varchar(50) DEFAULT NULL,
  `valor` varchar(20) DEFAULT NULL,
  `resumen` mediumtext DEFAULT NULL,
  PRIMARY KEY (`id_bitacora_contpaqi`),
  KEY `usuario` (`usuario`),
  KEY `fecha` (`fecha`),
  KEY `tipo_poliza` (`tipo_poliza`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

