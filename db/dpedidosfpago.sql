/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `dpedidosfpago`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `dpedidosfpago` (
  `referencia` varchar(11) NOT NULL,
  `formapag` varchar(6) DEFAULT NULL,
  `valor` decimal(16,2) DEFAULT NULL,
  `porcentaje` double DEFAULT NULL,
  `trn_id` varchar(50) DEFAULT NULL,
  KEY `formapag` (`formapag`) USING BTREE,
  KEY `referencia` (`referencia`),
  KEY `trn_id` (`trn_id`),
  CONSTRAINT `dpedidosfpago_ibfk_1` FOREIGN KEY (`formapag`) REFERENCES `formasdepago` (`formapag`) ON UPDATE CASCADE,
  CONSTRAINT `dpedidosfpago_ibfk_2` FOREIGN KEY (`referencia`) REFERENCES `pedidosecomm` (`referencia`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

