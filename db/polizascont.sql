/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `polizascont`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `polizascont` (
  `nEje` int(11) DEFAULT NULL,
  `nPer` int(11) DEFAULT NULL,
  `nTipoPol` int(11) DEFAULT NULL,
  `nPoliza` bigint(20) NOT NULL AUTO_INCREMENT,
  `nClase` int(11) DEFAULT 1,
  `bImpresa` varchar(1) DEFAULT 'F',
  `mConcepto` varchar(101) DEFAULT NULL,
  `dFecha` varchar(10) DEFAULT NULL,
  `Cargos` double DEFAULT NULL,
  `Abonos` double DEFAULT NULL,
  `nDiario` bigint(20) DEFAULT 2,
  `nSisOrigen` int(11) DEFAULT 10,
  `CfrCtrl` varchar(26) DEFAULT NULL,
  `enviado` tinyint(1) DEFAULT NULL,
  `sucursal` varchar(2) NOT NULL,
  `tipocomprobante` varchar(4) NOT NULL,
  `tipoplantilla` varchar(4) NOT NULL,
  `fechaini` date NOT NULL,
  `fechafin` date NOT NULL,
  `proveedor` varchar(11) NOT NULL DEFAULT '',
  `pagoprov` varchar(11) NOT NULL DEFAULT '',
  `cliente` varchar(11) NOT NULL DEFAULT '',
  `pagocli` varchar(11) NOT NULL DEFAULT '',
  `Guid` varchar(36) DEFAULT NULL,
  PRIMARY KEY (`nPoliza`),
  KEY `FK_polizascont_to_plantillastipos` (`tipocomprobante`,`tipoplantilla`),
  KEY `Guid` (`Guid`),
  CONSTRAINT `FK_polizascont_to_plantillastipos` FOREIGN KEY (`tipocomprobante`, `tipoplantilla`) REFERENCES `plantillastipos` (`tipocomprobante`, `tipoplantilla`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

