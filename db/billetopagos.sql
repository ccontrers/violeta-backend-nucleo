/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `billetopagos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `billetopagos` (
  `transaccion` varchar(11) NOT NULL DEFAULT '0',
  `venta` varchar(11) NOT NULL DEFAULT '0',
  `fechaalta` date NOT NULL,
  `horaalta` time NOT NULL,
  `cancelado` tinyint(4) NOT NULL DEFAULT 0,
  `fechacancel` date NOT NULL DEFAULT '0000-00-00',
  `horacancel` time DEFAULT NULL,
  `usuario` varchar(10) DEFAULT NULL,
  PRIMARY KEY (`transaccion`),
  KEY `billetopagos_venta_fk` (`venta`),
  KEY `billetopagos_usuario_fk` (`usuario`),
  CONSTRAINT `billetopagos_transaccion_fk` FOREIGN KEY (`transaccion`) REFERENCES `bitacoratransaccionesbilleto` (`merchant_transaction_id`) ON UPDATE CASCADE,
  CONSTRAINT `billetopagos_usuario_fk` FOREIGN KEY (`usuario`) REFERENCES `empleados` (`empleado`) ON UPDATE CASCADE,
  CONSTRAINT `billetopagos_venta_fk` FOREIGN KEY (`venta`) REFERENCES `ventas` (`referencia`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

