/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `bancostransacc`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `bancostransacc` (
  `transacc` int(11) NOT NULL AUTO_INCREMENT,
  `idmovbanco` int(11) NOT NULL,
  `tipodet` varchar(4) NOT NULL,
  `identificador` varchar(20) DEFAULT NULL,
  `subtotal` decimal(16,2) DEFAULT NULL,
  `ivabanco` decimal(16,2) DEFAULT NULL,
  `total` decimal(16,2) DEFAULT NULL,
  `cveimp` int(2) DEFAULT 0,
  `pagcliajuste` varchar(11) DEFAULT NULL,
  `pagprovajuste` varchar(11) DEFAULT NULL,
  `paggastoajuste` varchar(11) DEFAULT NULL,
  PRIMARY KEY (`transacc`),
  KEY `idmovbanco` (`idmovbanco`),
  KEY `bancostransacc_ibfk_2` (`pagcliajuste`),
  KEY `bancostransacc_ibfk_3` (`pagprovajuste`),
  KEY `bancostransacc_ibfk_4` (`paggastoajuste`),
  CONSTRAINT `bancostransacc_ibfk_1` FOREIGN KEY (`idmovbanco`) REFERENCES `bancosmov` (`idmovbanco`) ON UPDATE CASCADE,
  CONSTRAINT `bancostransacc_ibfk_2` FOREIGN KEY (`pagcliajuste`) REFERENCES `pagoscli` (`pago`) ON UPDATE CASCADE,
  CONSTRAINT `bancostransacc_ibfk_3` FOREIGN KEY (`pagprovajuste`) REFERENCES `pagosprov` (`pago`) ON UPDATE CASCADE,
  CONSTRAINT `bancostransacc_ibfk_4` FOREIGN KEY (`paggastoajuste`) REFERENCES `pagosgastos` (`pago`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

