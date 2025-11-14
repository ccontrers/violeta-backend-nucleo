/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `bitacoraprepagos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `bitacoraprepagos` (
  `referencia` varchar(11) NOT NULL,
  `cliente` varchar(11) NOT NULL,
  `tipo` varchar(10) NOT NULL,
  `estatus` varchar(2) NOT NULL COMMENT 'A: ALTA, C: CANCELADO',
  `valor` decimal(16,2) NOT NULL,
  `fecha` date NOT NULL,
  `hora` time NOT NULL,
  `terminal` varchar(10) NOT NULL,
  `usuario` varchar(10) NOT NULL,
  KEY `referencia` (`referencia`),
  KEY `terminal` (`terminal`),
  KEY `usuario` (`usuario`),
  KEY `cliente` (`cliente`),
  CONSTRAINT `bitacoraprepagos_ibfk_1` FOREIGN KEY (`referencia`) REFERENCES `prepagoscli` (`pago`) ON UPDATE CASCADE,
  CONSTRAINT `bitacoraprepagos_ibfk_2` FOREIGN KEY (`terminal`) REFERENCES `terminales` (`terminal`) ON UPDATE CASCADE,
  CONSTRAINT `bitacoraprepagos_ibfk_3` FOREIGN KEY (`usuario`) REFERENCES `empleados` (`empleado`) ON UPDATE CASCADE,
  CONSTRAINT `bitacoraprepagos_ibfk_4` FOREIGN KEY (`cliente`) REFERENCES `clientes` (`cliente`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

