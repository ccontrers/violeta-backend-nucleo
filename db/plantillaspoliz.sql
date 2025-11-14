/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `plantillaspoliz`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `plantillaspoliz` (
  `plantilla` int(11) NOT NULL AUTO_INCREMENT,
  `tipocomprobante` varchar(4) NOT NULL,
  `tipoplantilla` varchar(4) NOT NULL,
  `tipopartida` varchar(4) DEFAULT NULL,
  `orden` int(11) NOT NULL,
  `numcuenta` varchar(30) NOT NULL,
  `tipoafectacion` varchar(1) NOT NULL,
  `expresion` varchar(50) DEFAULT NULL,
  `sucursal` varchar(2) NOT NULL,
  `parametros` varchar(255) DEFAULT NULL,
  `clasif1` varchar(10) DEFAULT NULL,
  `clasif2` varchar(10) DEFAULT NULL,
  `clasif3` varchar(10) DEFAULT NULL,
  `termino` varchar(5) DEFAULT NULL,
  `impuesto` int(2) DEFAULT NULL,
  `acredito` tinyint(1) DEFAULT NULL,
  `clasifcont` varchar(5) DEFAULT NULL,
  `clasifcont2` varchar(5) DEFAULT NULL,
  `tipoimpu` varchar(10) DEFAULT NULL,
  `negimpuesto` tinyint(1) NOT NULL DEFAULT 0,
  `impuesto2` int(2) DEFAULT NULL,
  `tipoimpu2` varchar(10) DEFAULT NULL,
  `negimpuesto2` tinyint(1) NOT NULL DEFAULT 0,
  `parterel` tinyint(1) DEFAULT NULL,
  `fechainivent` date NOT NULL DEFAULT '2000-01-01',
  `fechafinvent` date NOT NULL DEFAULT '2099-12-31',
  `sucdetalle` varchar(2) DEFAULT NULL,
  `formaspago` varchar(256) DEFAULT NULL,
  `tiporfc` tinyint(1) DEFAULT NULL,
  `idnumcuenta` int(11) DEFAULT NULL,
  `agrupabanco` tinyint(1) NOT NULL DEFAULT 0,
  `segmentodefault` int(11) DEFAULT NULL,
  `tipogasto` varchar(11) DEFAULT NULL,
  PRIMARY KEY (`plantilla`),
  KEY `suctipo` (`sucursal`,`tipocomprobante`),
  KEY `FK_plantillaspoliz1` (`clasif1`),
  KEY `FK_plantillaspoliz2` (`clasif2`),
  KEY `FK_plantillaspoliz3` (`clasif3`),
  KEY `FK_plantillaspoliz_ter` (`termino`),
  KEY `FK_plantillaspoliz_ccont1` (`clasifcont`),
  KEY `FK_plantillaspoliz_ccont2` (`clasifcont2`),
  KEY `FK_plantillaspoliz_bancoscuentas` (`idnumcuenta`),
  CONSTRAINT `FK_plantillaspoliz1` FOREIGN KEY (`clasif1`) REFERENCES `clasificacion1` (`clasif1`) ON UPDATE CASCADE,
  CONSTRAINT `FK_plantillaspoliz2` FOREIGN KEY (`clasif2`) REFERENCES `clasificacion2` (`clasif2`) ON UPDATE CASCADE,
  CONSTRAINT `FK_plantillaspoliz3` FOREIGN KEY (`clasif3`) REFERENCES `clasificacion3` (`clasif3`) ON UPDATE CASCADE,
  CONSTRAINT `FK_plantillaspoliz_bancoscuentas` FOREIGN KEY (`idnumcuenta`) REFERENCES `bancoscuentas` (`idnumcuenta`) ON UPDATE CASCADE,
  CONSTRAINT `FK_plantillaspoliz_ccont1` FOREIGN KEY (`clasifcont`) REFERENCES `clasifcont` (`clasif`) ON UPDATE CASCADE,
  CONSTRAINT `FK_plantillaspoliz_ccont2` FOREIGN KEY (`clasifcont2`) REFERENCES `clasifcont2` (`clasif2`) ON UPDATE CASCADE,
  CONSTRAINT `FK_plantillaspoliz_ter` FOREIGN KEY (`termino`) REFERENCES `terminosdepago` (`termino`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

