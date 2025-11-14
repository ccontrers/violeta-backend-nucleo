/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `surtidores`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `surtidores` (
  `empleado` varchar(10) NOT NULL DEFAULT '',
  `tipo` varchar(1) DEFAULT NULL,
  `activo` tinyint(1) DEFAULT NULL,
  `fechaalta` date NOT NULL DEFAULT '0000-00-00',
  `fechabaja` date DEFAULT NULL,
  `turno` int(1) DEFAULT NULL,
  `terminal_movil` varchar(10) DEFAULT NULL,
  `almacen_asignado` varchar(10) DEFAULT NULL,
  PRIMARY KEY (`empleado`),
  KEY `surtidores_ibfk_terminal` (`terminal_movil`),
  KEY `surtidores_ibfk_almacen` (`almacen_asignado`),
  CONSTRAINT `surtidores_ibfk_1` FOREIGN KEY (`empleado`) REFERENCES `empleados` (`empleado`) ON UPDATE CASCADE,
  CONSTRAINT `surtidores_ibfk_almacen` FOREIGN KEY (`almacen_asignado`) REFERENCES `clasificaciones_almacenes` (`clasificacion`) ON UPDATE CASCADE,
  CONSTRAINT `surtidores_ibfk_terminal` FOREIGN KEY (`terminal_movil`) REFERENCES `terminales` (`terminal`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

