/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `metodosdepagoecommcliente`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `metodosdepagoecommcliente` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `cliente` varchar(11) NOT NULL DEFAULT '',
  `tipopago` int(11) DEFAULT NULL,
  `tipovale` int(11) DEFAULT NULL,
  `numtarjeta` varchar(16) NOT NULL DEFAULT '',
  `fechaexpi` varchar(5) NOT NULL DEFAULT '',
  `clave` varchar(64) DEFAULT NULL,
  `titular` varchar(50) DEFAULT NULL,
  `esdefault` tinyint(1) NOT NULL DEFAULT 0,
  `espagoentrega` tinyint(1) NOT NULL DEFAULT 0,
  `idcardopenpay` varchar(50) DEFAULT NULL,
  PRIMARY KEY (`cliente`,`numtarjeta`),
  KEY `id` (`id`),
  KEY `metodosdepagoecommcliente_ibfk_1` (`tipopago`),
  KEY `metodosdepagoecommcliente_ibfk_2` (`tipovale`),
  CONSTRAINT `metodosdepagoecommcliente_ibfk_1` FOREIGN KEY (`tipopago`) REFERENCES `metodosdepagoecomm` (`id`) ON UPDATE CASCADE,
  CONSTRAINT `metodosdepagoecommcliente_ibfk_2` FOREIGN KEY (`tipovale`) REFERENCES `tiposvales` (`id`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

