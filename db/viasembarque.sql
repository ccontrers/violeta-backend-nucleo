/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `viasembarque`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `viasembarque` (
  `viaembarq` varchar(10) NOT NULL DEFAULT '',
  `descripcion` varchar(50) NOT NULL DEFAULT '',
  `marca` varchar(50) DEFAULT NULL,
  `placas` varchar(15) DEFAULT NULL,
  `tipoplacas` varchar(1) DEFAULT NULL,
  `chofer` varchar(10) DEFAULT NULL,
  `volumen` decimal(16,2) DEFAULT NULL,
  `capacidad` decimal(16,2) DEFAULT NULL,
  `capnormal` decimal(16,2) DEFAULT 0.00,
  `tipo` varchar(2) DEFAULT NULL,
  `fechamant` date NOT NULL DEFAULT '0000-00-00',
  `estado` varchar(1) DEFAULT NULL,
  `nummotor` varchar(30) DEFAULT NULL,
  `tarjeta` varchar(20) DEFAULT NULL,
  `fechasegur` date NOT NULL DEFAULT '0000-00-00',
  `modelo` int(4) DEFAULT NULL,
  `kilometraje` int(7) DEFAULT NULL,
  `transportista` tinyint(1) DEFAULT 1,
  `polizaaseguradora` varchar(15) DEFAULT NULL,
  `claveaseguradora` varchar(10) DEFAULT NULL,
  `remolque` tinyint(1) DEFAULT 0,
  `configvehicular` int(11) DEFAULT NULL,
  `numpermisosct` varchar(50) DEFAULT NULL,
  `idremolque` int(11) DEFAULT NULL,
  `tipopermiso` varchar(10) DEFAULT NULL,
  PRIMARY KEY (`viaembarq`),
  KEY `descripcion` (`descripcion`),
  KEY `chofer` (`chofer`),
  KEY `viasembarque_ibfk_2` (`claveaseguradora`),
  KEY `viasembarque_ibfk_4` (`configvehicular`),
  KEY `viasembarque_ibfk_5` (`idremolque`),
  KEY `viasembarque_ibfk_6` (`tipopermiso`),
  CONSTRAINT `viasembarque_ibfk_1` FOREIGN KEY (`chofer`) REFERENCES `choferes` (`empleado`) ON UPDATE CASCADE,
  CONSTRAINT `viasembarque_ibfk_2` FOREIGN KEY (`claveaseguradora`) REFERENCES `catalogoaseguradoras` (`clave`) ON UPDATE CASCADE,
  CONSTRAINT `viasembarque_ibfk_4` FOREIGN KEY (`configvehicular`) REFERENCES `cconfigautotransporte` (`idclaveautotrans`) ON UPDATE CASCADE,
  CONSTRAINT `viasembarque_ibfk_5` FOREIGN KEY (`idremolque`) REFERENCES `catalogoremolques` (`idremolque`) ON UPDATE CASCADE,
  CONSTRAINT `viasembarque_ibfk_6` FOREIGN KEY (`tipopermiso`) REFERENCES `ccptipopermiso` (`cvetippermiso`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

