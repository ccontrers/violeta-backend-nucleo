/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `vendedores`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `vendedores` (
  `empleado` varchar(10) NOT NULL DEFAULT '',
  `tipocomi` varchar(2) DEFAULT NULL,
  `tipovend` varchar(1) DEFAULT NULL,
  `activo` tinyint(1) DEFAULT NULL,
  `fechaalta` date NOT NULL DEFAULT '0000-00-00',
  `porccomi` decimal(5,2) NOT NULL DEFAULT 0.00,
  `tolercomision` int(3) DEFAULT 0,
  `fechabaja` date NOT NULL DEFAULT '0000-00-00',
  `terminalmovil` varchar(10) DEFAULT NULL,
  `vendedor_imitar` varchar(10) DEFAULT NULL,
  PRIMARY KEY (`empleado`),
  KEY `vendedores_ibfk_2` (`terminalmovil`),
  KEY `vendedor_imitar` (`vendedor_imitar`),
  CONSTRAINT `vendedores_ibfk_1` FOREIGN KEY (`empleado`) REFERENCES `empleados` (`empleado`) ON UPDATE CASCADE,
  CONSTRAINT `vendedores_ibfk_2` FOREIGN KEY (`terminalmovil`) REFERENCES `terminales` (`terminal`) ON UPDATE CASCADE,
  CONSTRAINT `vendedores_ibfk_3` FOREIGN KEY (`vendedor_imitar`) REFERENCES `vendedores` (`empleado`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

