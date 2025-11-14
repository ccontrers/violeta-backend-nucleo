/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `pedcompdirorigen`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `pedcompdirorigen` (
  `id_pedcompdir` int(11) NOT NULL AUTO_INCREMENT,
  `referencia` varchar(11) DEFAULT NULL,
  `proveedor` varchar(11) DEFAULT NULL,
  `calle` varchar(60) DEFAULT NULL,
  `colonia` varchar(10) DEFAULT NULL,
  `id_cp` int(11) DEFAULT NULL,
  `ubicaciongis` point DEFAULT NULL,
  `fechaalta` date DEFAULT curdate(),
  PRIMARY KEY (`id_pedcompdir`) USING BTREE,
  KEY `referencia` (`referencia`) USING BTREE,
  KEY `colonia` (`colonia`) USING BTREE,
  KEY `id_cp` (`id_cp`) USING BTREE,
  KEY `proveedor` (`proveedor`) USING BTREE,
  CONSTRAINT `pedcomdirorigen_ibfk_1` FOREIGN KEY (`referencia`) REFERENCES `pedidos` (`referencia`) ON UPDATE CASCADE,
  CONSTRAINT `pedcomdirorigen_ibfk_2` FOREIGN KEY (`colonia`) REFERENCES `colonias` (`colonia`) ON UPDATE CASCADE,
  CONSTRAINT `pedcomdirorigen_ibfk_3` FOREIGN KEY (`id_cp`) REFERENCES `ccpclavecodpos` (`idclavecodpos`) ON UPDATE CASCADE,
  CONSTRAINT `pedcomdirorigen_ibfk_4` FOREIGN KEY (`proveedor`) REFERENCES `proveedores` (`proveedor`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

