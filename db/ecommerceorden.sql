/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `ecommerceorden`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ecommerceorden` (
  `nopedido` int(11) NOT NULL,
  `id` bigint(20) NOT NULL,
  `locationid` bigint(20) NOT NULL,
  `fchaalta` date NOT NULL,
  `fchamodi` date NOT NULL,
  `fchacancel` date NOT NULL,
  `total` decimal(10,2) NOT NULL,
  `subtotal` decimal(10,2) NOT NULL,
  `imptotal` decimal(10,2) NOT NULL,
  `costoenvio` decimal(10,2) NOT NULL,
  `peso` decimal(10,2) NOT NULL,
  `estatus_financiero` varchar(20) NOT NULL,
  `razon_cancelado` varchar(15) NOT NULL,
  `iva` decimal(10,2) NOT NULL,
  `estatus` varchar(10) NOT NULL DEFAULT 'pendiente',
  `nombre` varchar(250) NOT NULL,
  `direccion` text NOT NULL DEFAULT '',
  `telefono` varchar(18) NOT NULL,
  `codigopostal` int(11) NOT NULL,
  `ciudad` varchar(40) NOT NULL,
  `estado` varchar(50) NOT NULL,
  `reembolsado` int(11) DEFAULT 0,
  `cantreembolso` double DEFAULT 0,
  `referencia` varchar(11) NOT NULL DEFAULT '0',
  `latitude` decimal(10,6) NOT NULL DEFAULT 0.000000,
  `longitude` decimal(10,6) NOT NULL DEFAULT 0.000000,
  `notas` text NOT NULL DEFAULT '',
  `email` varchar(50) NOT NULL DEFAULT '',
  `visita` int(11) NOT NULL DEFAULT 0,
  PRIMARY KEY (`nopedido`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

