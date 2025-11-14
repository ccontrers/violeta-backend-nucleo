/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `pda3clientes`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `pda3clientes` (
  `cliente` varchar(11) NOT NULL DEFAULT '',
  `rsocial` varchar(100) DEFAULT NULL,
  `rfc` varchar(15) DEFAULT NULL,
  `domicilio` varchar(100) DEFAULT NULL,
  `bloqueo` varchar(2) DEFAULT NULL,
  `credito` tinyint(4) DEFAULT NULL,
  `limcred` decimal(16,2) DEFAULT NULL,
  `saldo` decimal(16,2) DEFAULT NULL,
  `pedxfact` decimal(16,2) DEFAULT NULL,
  `excederlc` tinyint(4) DEFAULT NULL,
  `plazo` int(11) DEFAULT NULL,
  `tipoprec` varchar(2) DEFAULT NULL,
  `nvolimcred` decimal(16,2) DEFAULT 0.00,
  PRIMARY KEY (`cliente`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

