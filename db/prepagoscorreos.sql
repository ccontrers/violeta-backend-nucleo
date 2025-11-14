/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `prepagoscorreos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `prepagoscorreos` (
  `idcorreos` int(11) NOT NULL AUTO_INCREMENT,
  `cliente` varchar(11) NOT NULL,
  `pdf` mediumtext NOT NULL,
  `nombrepdf` varchar(50) NOT NULL,
  `adjunto` mediumtext DEFAULT NULL,
  `nombreadjunto` varchar(50) DEFAULT NULL,
  `mensaje` mediumtext NOT NULL,
  `enviado` tinyint(1) NOT NULL DEFAULT 0,
  `email` varchar(50) DEFAULT NULL,
  `email2` varchar(50) DEFAULT '',
  `fechahoraenv` datetime NOT NULL DEFAULT '2000-01-01 00:00:00',
  PRIMARY KEY (`idcorreos`),
  KEY `cliente` (`cliente`),
  KEY `enviado` (`enviado`),
  CONSTRAINT `prepagoscorreos_clientes` FOREIGN KEY (`cliente`) REFERENCES `clientes` (`cliente`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

