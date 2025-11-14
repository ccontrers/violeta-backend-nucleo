/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `bitacoratransaccionesbilleto`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `bitacoratransaccionesbilleto` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `idBilleto` varchar(50) DEFAULT '',
  `fecha` date NOT NULL,
  `hora` time NOT NULL,
  `location_id` varchar(50) DEFAULT '',
  `idempotency_key` varchar(11) DEFAULT '',
  `merchant_transaction_id` varchar(11) NOT NULL DEFAULT '0',
  `total_cobrado` double(10,2) DEFAULT 0.00,
  `total_reembolsado` double(10,2) DEFAULT 0.00,
  `estatus` varchar(1) DEFAULT '' COMMENT 'F - Fallado, C - Completado, A - Autorizado, V - Anulado, R - Reembolsado',
  `cliente` text DEFAULT '',
  `whatsappcliente` varchar(15) DEFAULT '',
  `codigocliente` varchar(10) NOT NULL,
  `usuario` varchar(10) DEFAULT NULL,
  `mensajeservidor` text DEFAULT '',
  `codigorespuesta` int(11) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `merchant_transaction_id` (`merchant_transaction_id`),
  KEY `bitacoratransbilleto_usuario_fk` (`usuario`),
  CONSTRAINT `bitacoratransbilleto_usuario_fk` FOREIGN KEY (`usuario`) REFERENCES `empleados` (`empleado`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

