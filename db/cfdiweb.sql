/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `cfdiweb`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `cfdiweb` (
  `refticket` varchar(11) DEFAULT NULL,
  `factgenerada` varchar(11) DEFAULT NULL,
  `rfc` varchar(15) DEFAULT NULL,
  `codigopostal` int(11) DEFAULT NULL,
  `usocfdi` varchar(11) DEFAULT NULL,
  `fechaalta` date DEFAULT NULL,
  `horaalta` time DEFAULT NULL,
  `pdfgenerado` longtext DEFAULT NULL,
  `activo` tinyint(1) NOT NULL DEFAULT 1,
  KEY `FK1_cfdiweb_refticket` (`refticket`),
  KEY `FK2_cfdiweb_factgenerada` (`factgenerada`),
  KEY `FK3_cfdiweb_usocfdi` (`usocfdi`),
  CONSTRAINT `FK1_cfdiweb_refticket` FOREIGN KEY (`refticket`) REFERENCES `ventas` (`referencia`) ON UPDATE CASCADE,
  CONSTRAINT `FK2_cfdiweb_factgenerada` FOREIGN KEY (`factgenerada`) REFERENCES `ventas` (`referencia`) ON UPDATE CASCADE,
  CONSTRAINT `FK3_cfdiweb_usocfdi` FOREIGN KEY (`usocfdi`) REFERENCES `cusocfdi` (`usocfdi`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

