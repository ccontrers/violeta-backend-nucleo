/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `ecommerceproductos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ecommerceproductos` (
  `articulo` varchar(9) NOT NULL DEFAULT '0',
  `idProducto` bigint(20) NOT NULL DEFAULT 0,
  `idInventario` bigint(20) NOT NULL DEFAULT 0,
  `idVariant` bigint(20) NOT NULL DEFAULT 0,
  `fechaalta` date NOT NULL,
  `fechamodi` date NOT NULL,
  `horaalta` time NOT NULL,
  `horamodi` time NOT NULL,
  `usualta` varchar(10) NOT NULL DEFAULT '',
  `usumodi` varchar(10) NOT NULL DEFAULT '',
  `activo` tinyint(1) unsigned NOT NULL DEFAULT 0,
  `precio` decimal(10,2) DEFAULT NULL,
  `inventario` int(11) NOT NULL DEFAULT 0,
  `codigobarrasis` varchar(14) DEFAULT NULL,
  `cantsis` int(11) DEFAULT NULL,
  `tag` varchar(30) DEFAULT NULL,
  `fraccionar` double NOT NULL DEFAULT 1,
  `tipodeprec` varchar(2) NOT NULL DEFAULT ' ',
  `shopify` int(1) NOT NULL DEFAULT 1,
  `rappi` int(1) NOT NULL DEFAULT 0,
  `aplicadescrappi` int(11) NOT NULL DEFAULT 0,
  `fechainidesc` date DEFAULT NULL,
  `fechafindesc` date DEFAULT NULL,
  `porcdescrappi` double DEFAULT NULL,
  PRIMARY KEY (`articulo`),
  KEY `usualta` (`usualta`),
  KEY `usumodi_ibk2` (`usumodi`),
  CONSTRAINT `ecommerceproductos_FK1` FOREIGN KEY (`articulo`) REFERENCES `articulos` (`articulo`) ON UPDATE CASCADE,
  CONSTRAINT `usualta_ibk1` FOREIGN KEY (`usualta`) REFERENCES `empleados` (`empleado`) ON UPDATE CASCADE,
  CONSTRAINT `usumodi_ibk2` FOREIGN KEY (`usumodi`) REFERENCES `empleados` (`empleado`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

