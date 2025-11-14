/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `movimientoscont`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `movimientoscont` (
  `nEje` int(11) DEFAULT NULL,
  `nPer` int(11) DEFAULT NULL,
  `nTipoPol` int(11) DEFAULT NULL,
  `nPoliza` bigint(20) DEFAULT NULL,
  `nMovto` int(11) DEFAULT 0,
  `szCuenta` varchar(21) DEFAULT NULL,
  `bTipoMov` varchar(255) DEFAULT NULL,
  `szRefer` varchar(11) DEFAULT NULL,
  `Importe` double DEFAULT NULL,
  `nDiario` bigint(20) DEFAULT NULL,
  `nMoneda` double DEFAULT NULL,
  `szDescrip` varchar(100) DEFAULT NULL,
  `dFecha` varchar(9) DEFAULT NULL,
  `idMovimiento` int(11) NOT NULL AUTO_INCREMENT,
  `sucdetalle` varchar(2) DEFAULT NULL,
  `segmento` int(11) DEFAULT NULL,
  `Guid` varchar(36) DEFAULT NULL,
  PRIMARY KEY (`idMovimiento`),
  KEY `nPoliza` (`nPoliza`),
  KEY `Guid` (`Guid`),
  CONSTRAINT `FK_movimientoscont_polizascont` FOREIGN KEY (`nPoliza`) REFERENCES `polizascont` (`nPoliza`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

