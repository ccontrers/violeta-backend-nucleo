/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `openpaycargostarjetaxcliente`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `openpaycargostarjetaxcliente` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `id_cargo` varchar(50) DEFAULT NULL,
  `amount` decimal(16,4) DEFAULT NULL,
  `authorization` varchar(10) DEFAULT NULL,
  `creation_date` datetime DEFAULT NULL,
  `allows_charges` tinyint(4) DEFAULT NULL,
  `allows_payouts` tinyint(4) DEFAULT NULL,
  `bank_code` varchar(20) DEFAULT NULL,
  `bank_name` varchar(20) DEFAULT NULL,
  `brand` varchar(20) DEFAULT NULL,
  `card_number` varchar(20) DEFAULT NULL,
  `expiration_month` varchar(5) DEFAULT NULL,
  `expiration_year` varchar(5) DEFAULT NULL,
  `holder_name` varchar(50) DEFAULT NULL,
  `id_tarjeta` varchar(50) DEFAULT NULL,
  `type` varchar(10) DEFAULT NULL,
  `amount_fee` decimal(16,2) NOT NULL DEFAULT 0.00,
  `currency_fee` varchar(5) DEFAULT NULL,
  `tax_fee` decimal(16,4) NOT NULL DEFAULT 0.0000,
  `currency` varchar(5) DEFAULT NULL,
  `customer_id` varchar(50) DEFAULT NULL,
  `description` varchar(250) DEFAULT NULL,
  `error_message` text DEFAULT NULL,
  `method` varchar(10) DEFAULT NULL,
  `operation_date` datetime DEFAULT NULL,
  `operation_type` varchar(10) DEFAULT NULL,
  `order_id` varchar(11) DEFAULT NULL,
  `status` varchar(15) DEFAULT NULL,
  `transaction_type` varchar(10) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `id_cargo` (`id_cargo`) USING BTREE,
  KEY `order_id` (`order_id`) USING BTREE,
  KEY `customer_id` (`customer_id`) USING BTREE,
  CONSTRAINT `openpaycargostarjetaxcliente_ibfk_1` FOREIGN KEY (`customer_id`) REFERENCES `openpaycliente` (`id`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

