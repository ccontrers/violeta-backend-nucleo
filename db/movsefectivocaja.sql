/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `movsefectivocaja`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `movsefectivocaja` (
  `claveretiro` varchar(11) NOT NULL DEFAULT '0',
  `terminal` varchar(10) NOT NULL,
  `fechaalta` date NOT NULL,
  `horaalta` time NOT NULL,
  `fecharetiro` date DEFAULT NULL,
  `horaretiro` time DEFAULT NULL,
  `concepto` varchar(1) DEFAULT NULL COMMENT 'D - Depositos R - Retiros',
  `cantretirado` decimal(12,3) DEFAULT 0.000,
  `usuario` varchar(10) DEFAULT NULL,
  `usuarioautoriza` varchar(10) DEFAULT NULL,
  `corte` varchar(15) DEFAULT NULL,
  `conceptomovs` varchar(6) NOT NULL,
  `venta` varchar(11) DEFAULT NULL,
  PRIMARY KEY (`claveretiro`),
  KEY `usuarios_retirosventad` (`usuario`),
  KEY `usuarioautoriza_retirosventad` (`usuarioautoriza`),
  KEY `catalogomovscaja_conceptomovs` (`conceptomovs`),
  KEY `movsefectivocaja_id_fk4` (`venta`),
  CONSTRAINT `catalogomovscaja_conceptomovs` FOREIGN KEY (`conceptomovs`) REFERENCES `catalogomovscaja` (`clave`) ON UPDATE CASCADE,
  CONSTRAINT `movsefectivocaja_id_fk4` FOREIGN KEY (`venta`) REFERENCES `ventas` (`referencia`) ON UPDATE CASCADE,
  CONSTRAINT `usuarioautoriza_retirosventad` FOREIGN KEY (`usuarioautoriza`) REFERENCES `empleados` (`empleado`) ON UPDATE CASCADE,
  CONSTRAINT `usuarios_retirosventad` FOREIGN KEY (`usuario`) REFERENCES `empleados` (`empleado`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

