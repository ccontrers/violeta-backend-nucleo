/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `tipo_concentrado_pedido_automatico`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `tipo_concentrado_pedido_automatico` (
  `concentrado` int(10) unsigned DEFAULT NULL,
  `proveedor` varchar(11) NOT NULL,
  `empresa` int(11) NOT NULL,
  `tipo` varchar(8) NOT NULL CHECK (`tipo` in ('SUCURSAL','EMPRESA')),
  PRIMARY KEY (`empresa`,`proveedor`),
  KEY `FK_CPA_CONCENTRADO_concentrado_proveedor` (`concentrado`),
  CONSTRAINT `FK_CPA_CONCENTRADO_concentrado_proveedor` FOREIGN KEY (`concentrado`) REFERENCES `concentrados_pedido_automatico` (`id`) ON UPDATE CASCADE,
  CONSTRAINT `FK_CPA_EMPRESA` FOREIGN KEY (`empresa`) REFERENCES `empresas` (`idempresa`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

