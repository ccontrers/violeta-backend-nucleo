/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `cfdigastos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `cfdigastos` (
  `idcfdigastos` int(11) NOT NULL AUTO_INCREMENT,
  `referencia` varchar(11) NOT NULL,
  `totalcomprobante` varchar(20) NOT NULL,
  `fechacomprobante` varchar(28) NOT NULL,
  `fechatimbrado` varchar(28) NOT NULL,
  `fechacarga` datetime NOT NULL,
  `version` varchar(4) DEFAULT '3.2',
  `nombrearch` varchar(255) NOT NULL,
  `emisor` varchar(300) NOT NULL,
  `rfcemisor` varchar(13) NOT NULL,
  `receptor` varchar(300) NOT NULL,
  `rfcreceptor` varchar(13) NOT NULL,
  `xml` mediumtext NOT NULL,
  `pdf` mediumtext NOT NULL,
  PRIMARY KEY (`idcfdigastos`),
  UNIQUE KEY `referencia` (`referencia`),
  KEY `fechacomprobante` (`fechacomprobante`),
  KEY `fechatimbrado` (`fechatimbrado`),
  CONSTRAINT `fk_cfdigastos_gastos` FOREIGN KEY (`referencia`) REFERENCES `gastos` (`referencia`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

