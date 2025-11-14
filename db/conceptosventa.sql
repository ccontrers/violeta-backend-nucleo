/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `conceptosventa`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `conceptosventa` (
  `clave` varchar(10) NOT NULL,
  `concepto` varchar(40) DEFAULT NULL,
  `mindef` int(9) DEFAULT 0,
  `maxdef` int(9) DEFAULT 0,
  `tipoprec` varchar(2) NOT NULL,
  `idempresa` int(2) NOT NULL,
  `imprimir` tinyint(1) NOT NULL DEFAULT 0,
  PRIMARY KEY (`clave`) USING BTREE,
  KEY `tipoprec` (`tipoprec`),
  KEY `conceptventaempresa_ibfk_1` (`idempresa`),
  CONSTRAINT `conceptosventa_ibfk_1` FOREIGN KEY (`tipoprec`) REFERENCES `tiposdeprecios` (`tipoprec`) ON UPDATE CASCADE,
  CONSTRAINT `conceptventaempresa_ibfk_1` FOREIGN KEY (`idempresa`) REFERENCES `empresas` (`idempresa`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

