/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `pagosgastos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `pagosgastos` (
  `pago` varchar(11) NOT NULL DEFAULT '',
  `ident` varchar(20) NOT NULL DEFAULT '',
  `proveedor` varchar(11) NOT NULL DEFAULT '',
  `fecha` date NOT NULL DEFAULT '0000-00-00',
  `hora` time DEFAULT NULL,
  `fechamodi` date NOT NULL DEFAULT '0000-00-00',
  `horamodi` time DEFAULT NULL,
  `valor` decimal(16,2) DEFAULT NULL,
  `cancelado` tinyint(1) DEFAULT NULL,
  `usualta` varchar(10) DEFAULT NULL,
  `usumodi` varchar(10) DEFAULT NULL,
  `ajuste` decimal(16,2) DEFAULT NULL,
  `formapag` varchar(1) DEFAULT NULL,
  `muuid` varchar(36) DEFAULT NULL,
  `terminal` varchar(10) NOT NULL DEFAULT '',
  `esperacomplemento` tinyint(4) NOT NULL DEFAULT 1,
  PRIMARY KEY (`pago`) USING BTREE,
  KEY `proveedor` (`proveedor`) USING BTREE,
  KEY `ident` (`ident`) USING BTREE,
  KEY `fecha` (`fecha`) USING BTREE,
  KEY `usualta` (`usualta`) USING BTREE,
  KEY `usumodi` (`usumodi`) USING BTREE,
  KEY `terminal` (`terminal`) USING BTREE,
  CONSTRAINT `fk_terminal_pagosgastos` FOREIGN KEY (`terminal`) REFERENCES `terminales` (`terminal`) ON UPDATE CASCADE,
  CONSTRAINT `pagosgastos_ibfk_1` FOREIGN KEY (`proveedor`) REFERENCES `proveedores` (`proveedor`) ON UPDATE CASCADE,
  CONSTRAINT `pagosgastos_ibfk_2` FOREIGN KEY (`usualta`) REFERENCES `usuarios` (`empleado`) ON UPDATE CASCADE,
  CONSTRAINT `pagosgastos_ibfk_3` FOREIGN KEY (`usumodi`) REFERENCES `usuarios` (`empleado`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

