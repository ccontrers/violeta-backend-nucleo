/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `auxresultadonotas`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `auxresultadonotas` (
  `cliente` varchar(11) DEFAULT NULL,
  `rsocial` varchar(255) DEFAULT NULL,
  `rfcreceptor` varchar(13) DEFAULT NULL,
  `FechaNota` date DEFAULT NULL,
  `FolioNota` varchar(11) DEFAULT NULL,
  `Articulo` varchar(11) DEFAULT NULL,
  `NombreProducto` varchar(255) DEFAULT NULL,
  `producto` varchar(8) DEFAULT NULL,
  `present` varchar(13) DEFAULT NULL,
  `multiplo` varchar(10) DEFAULT NULL,
  `uuidNotaCred` varchar(36) DEFAULT NULL,
  `FolioVenta` varchar(11) DEFAULT NULL,
  `FechaVenta` date DEFAULT NULL,
  `Factura` varchar(20) DEFAULT NULL,
  `uuidFactura` varchar(36) DEFAULT NULL,
  `tipoNota` varchar(1) DEFAULT NULL,
  `tipodescrito` varchar(50) DEFAULT NULL,
  `subtotal` decimal(16,6) DEFAULT NULL,
  `imp1` decimal(16,6) DEFAULT NULL,
  `imp2` decimal(16,6) DEFAULT NULL,
  `imp3` decimal(16,6) DEFAULT NULL,
  `imp4` decimal(16,6) DEFAULT NULL,
  `iva1` decimal(16,6) DEFAULT NULL,
  `iva2` decimal(16,6) DEFAULT NULL,
  `ivatotal` decimal(16,6) DEFAULT NULL,
  `iesps1` decimal(16,6) DEFAULT NULL,
  `iesps2` decimal(16,6) DEFAULT NULL,
  `iespstotal` decimal(16,6) DEFAULT NULL,
  `subgrabado` decimal(16,6) DEFAULT NULL,
  `subnograbado` decimal(16,6) DEFAULT NULL,
  `impcol2` decimal(16,6) DEFAULT NULL,
  `impcol8` decimal(16,6) DEFAULT NULL,
  `impcol12` decimal(16,6) DEFAULT NULL,
  `impcol13` decimal(16,6) DEFAULT NULL,
  `impcol11` decimal(16,6) DEFAULT NULL,
  `impcol3` decimal(16,6) DEFAULT NULL,
  `impcol4` decimal(16,6) DEFAULT NULL,
  `impcol9` decimal(16,6) DEFAULT NULL,
  `impcol5` decimal(16,6) DEFAULT NULL,
  `impcol6` decimal(16,6) DEFAULT NULL,
  `impcol10` decimal(16,6) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

