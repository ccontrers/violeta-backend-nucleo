/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `cfdcartasporte`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `cfdcartasporte` (
  `cfdcartap` int(11) NOT NULL AUTO_INCREMENT,
  `cfduuid` varchar(36) NOT NULL,
  `empresatrans` varchar(2) NOT NULL,
  `monto` decimal(12,2) NOT NULL,
  `xmlgenerado` mediumtext NOT NULL,
  `impresionfecha` datetime DEFAULT NULL,
  `impreso` tinyint(1) DEFAULT NULL,
  `fechaalta` datetime DEFAULT NULL,
  `estado` varchar(2) DEFAULT NULL,
  `tipocomprobante` varchar(10) DEFAULT NULL,
  `referencia` varchar(11) DEFAULT NULL,
  `cadenaoriginal` text DEFAULT NULL,
  `fechacancela` datetime DEFAULT NULL,
  `ivarm` decimal(12,2) DEFAULT NULL,
  `metodopago` varchar(32) DEFAULT 'NA',
  `digitos` varchar(4) DEFAULT '',
  `VERSION` varchar(4) DEFAULT '3.2',
  `usocfdi` varchar(4) DEFAULT NULL,
  `cfdirelacionado` varchar(36) DEFAULT NULL,
  `metodopago33` varchar(3) DEFAULT NULL,
  `formapago33` varchar(2) DEFAULT NULL,
  PRIMARY KEY (`cfdcartap`),
  UNIQUE KEY `cfduuid` (`cfduuid`),
  KEY `empresatrans` (`empresatrans`),
  KEY `refer` (`referencia`,`tipocomprobante`),
  KEY `usocfdi` (`usocfdi`),
  CONSTRAINT `FK_cfdcartasporte_empre` FOREIGN KEY (`empresatrans`) REFERENCES `transportistas` (`empresatrans`) ON UPDATE CASCADE,
  CONSTRAINT `cfdcartasporte_usocfd` FOREIGN KEY (`usocfdi`) REFERENCES `cusocfdi` (`usocfdi`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

