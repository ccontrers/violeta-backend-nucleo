/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `cfd`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `cfd` (
  `compfiscal` int(11) NOT NULL AUTO_INCREMENT,
  `sucursal` varchar(2) NOT NULL,
  `folio` varchar(20) NOT NULL,
  `monto` decimal(12,2) NOT NULL,
  `fechaalta` datetime NOT NULL,
  `estado` varchar(2) NOT NULL,
  `tipocomprobante` varchar(10) NOT NULL,
  `referencia` varchar(11) NOT NULL,
  `serie` varchar(10) DEFAULT NULL,
  `seriefolio` varchar(30) NOT NULL,
  `fechacancela` datetime DEFAULT NULL,
  `motivocanc` varchar(2) DEFAULT NULL,
  `uuidrelcanc` varchar(36) DEFAULT NULL,
  `vtacontab` tinyint(1) NOT NULL DEFAULT 0,
  `cancvtacontab` tinyint(1) NOT NULL DEFAULT 0,
  `ivarm` decimal(12,2) DEFAULT NULL,
  `ticksub0` decimal(12,2) NOT NULL DEFAULT 0.00,
  `tickdesglose2014` tinyint(1) NOT NULL DEFAULT 0,
  `ticksubiva0` decimal(12,2) NOT NULL DEFAULT 0.00,
  `ticksubiva0ieps` decimal(12,2) NOT NULL DEFAULT 0.00,
  `ticksubiva16` decimal(12,2) NOT NULL DEFAULT 0.00,
  `ticksubiva16ieps` decimal(12,2) NOT NULL DEFAULT 0.00,
  `noaprobacion` varchar(20) DEFAULT NULL,
  `anoAprobacion` int(4) DEFAULT NULL,
  `numseriecert` varchar(60) DEFAULT NULL,
  `fechaaltamin` date DEFAULT NULL,
  `fechacancelamin` date DEFAULT NULL,
  `version` varchar(4) DEFAULT '2.2',
  `muuid` varchar(36) DEFAULT NULL,
  `pactimbrador` tinyint(1) DEFAULT NULL,
  `metodopago` varchar(32) NOT NULL DEFAULT 'NA',
  `digitos` varchar(4) NOT NULL DEFAULT '',
  `usocfdi` varchar(4) DEFAULT NULL,
  `cfdirelacionado` varchar(36) DEFAULT NULL,
  `metodopago33` varchar(3) DEFAULT NULL,
  `formapago33` varchar(2) DEFAULT NULL,
  `rfcreceptor` varchar(13) DEFAULT NULL,
  `desgloseimpuestos33` tinyint(1) DEFAULT NULL,
  `agruparncre` tinyint(1) NOT NULL DEFAULT 0,
  PRIMARY KEY (`compfiscal`),
  UNIQUE KEY `seriefolio` (`seriefolio`),
  UNIQUE KEY `sucserfolio` (`sucursal`,`serie`,`folio`),
  KEY `stiporefer` (`sucursal`,`referencia`,`tipocomprobante`),
  KEY `refertipo` (`referencia`,`tipocomprobante`),
  KEY `tipocomprobante` (`tipocomprobante`),
  KEY `fechaaltamin` (`fechaaltamin`),
  KEY `muuid` (`muuid`),
  KEY `fechaalta` (`fechaalta`),
  KEY `usocfdi` (`usocfdi`),
  KEY `cfdirelacionado` (`cfdirelacionado`),
  KEY `seriefoliosep` (`serie`,`folio`),
  KEY `FK3_motivocanc` (`motivocanc`),
  CONSTRAINT `FK3_motivocanc` FOREIGN KEY (`motivocanc`) REFERENCES `ccancmotivo` (`motivo`) ON UPDATE CASCADE,
  CONSTRAINT `FK_cfd_sucursales` FOREIGN KEY (`sucursal`) REFERENCES `sucursales` (`sucursal`) ON UPDATE CASCADE,
  CONSTRAINT `cfd_usocfd` FOREIGN KEY (`usocfdi`) REFERENCES `cusocfdi` (`usocfdi`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

