/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `openpaytarjetaxcliente`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `openpaytarjetaxcliente` (
  `id_tarjeta` varchar(25) DEFAULT NULL,
  `fecha_creacion` date NOT NULL DEFAULT curdate(),
  `holder_name` varchar(255) DEFAULT NULL,
  `card_number` varchar(16) DEFAULT NULL,
  `expiration_year` varchar(2) DEFAULT NULL,
  `expiration_month` varchar(2) DEFAULT NULL,
  `allows_charges` tinyint(1) DEFAULT NULL,
  `allows_payouts` tinyint(1) DEFAULT NULL,
  `tipo` varchar(10) DEFAULT NULL,
  `brand` varchar(20) DEFAULT NULL,
  `banco` varchar(50) DEFAULT NULL,
  `id_cliente` varchar(50) DEFAULT NULL,
  `bank_code` varchar(5) DEFAULT NULL,
  `points_card` tinyint(1) NOT NULL DEFAULT 0,
  `deviceSessionId` varchar(50) DEFAULT NULL,
  KEY `id_tarjeta` (`id_tarjeta`) USING BTREE,
  KEY `id_cliente` (`id_cliente`) USING BTREE,
  CONSTRAINT `openpaytarjetaxcliente_ibfk_1` FOREIGN KEY (`id_cliente`) REFERENCES `openpaycliente` (`id`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

