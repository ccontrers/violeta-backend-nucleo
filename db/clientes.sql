/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `clientes`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `clientes` (
  `cliente` varchar(11) NOT NULL DEFAULT '',
  `nombre` varchar(60) NOT NULL DEFAULT '',
  `appat` varchar(40) NOT NULL DEFAULT '',
  `apmat` varchar(40) NOT NULL DEFAULT '',
  `sucursal` varchar(2) NOT NULL DEFAULT '',
  `fnaccli` date DEFAULT NULL,
  `titulo` varchar(10) DEFAULT NULL,
  `rsocial` varchar(255) DEFAULT NULL,
  `nomnegocio` varchar(60) DEFAULT NULL,
  `contacto` varchar(40) DEFAULT NULL,
  `contacfnac` date DEFAULT NULL,
  `tipoempre` varchar(1) DEFAULT NULL,
  `calle` varchar(60) DEFAULT NULL,
  `numext` varchar(10) DEFAULT NULL,
  `numint` varchar(10) DEFAULT NULL,
  `referenciadomic` varchar(60) DEFAULT NULL,
  `colonia` varchar(10) DEFAULT NULL,
  `cp` varchar(5) DEFAULT NULL,
  `ubicaciongis` point DEFAULT NULL,
  `bloqueo` varchar(2) DEFAULT NULL,
  `credito` tinyint(1) DEFAULT NULL,
  `clasifpag` int(3) DEFAULT NULL,
  `clasifcom` int(3) DEFAULT NULL,
  `fechaalta` date NOT NULL DEFAULT '0000-00-00',
  `fechamodi` date NOT NULL DEFAULT '0000-00-00',
  `fechabloq` date NOT NULL DEFAULT '0000-00-00',
  `homonimos` tinyint(1) DEFAULT NULL,
  `curp` varchar(18) DEFAULT NULL,
  `rfc` varchar(15) DEFAULT NULL,
  `fechauven` date NOT NULL DEFAULT '0000-00-00',
  `limcred` decimal(16,2) DEFAULT NULL,
  `excederlc` tinyint(1) DEFAULT NULL,
  `plazo` int(4) DEFAULT NULL,
  `riesgo` int(3) DEFAULT NULL,
  `email` varchar(50) DEFAULT NULL,
  `email2` varchar(50) DEFAULT '@',
  `medio` varchar(20) DEFAULT NULL,
  `numpedidos` varchar(10) DEFAULT NULL,
  `activo` tinyint(1) DEFAULT NULL,
  `sgerencia` tinyint(1) DEFAULT 0,
  `venxvol` tinyint(1) DEFAULT 0,
  `enviarcfd` tinyint(1) NOT NULL DEFAULT 0,
  `metododef` varchar(2) NOT NULL DEFAULT '',
  `metodosup` varchar(2) NOT NULL DEFAULT '',
  `usocfdi` varchar(4) DEFAULT NULL,
  `valorsup` decimal(16,2) NOT NULL DEFAULT 0.00,
  `digitosdef` varchar(4) NOT NULL DEFAULT '',
  `digitossup` varchar(4) NOT NULL DEFAULT '',
  `esparterelac` tinyint(1) NOT NULL DEFAULT 0,
  `imprsaldos` tinyint(1) NOT NULL DEFAULT 1,
  `giro` varchar(4) DEFAULT NULL,
  `agruparncre` tinyint(1) NOT NULL DEFAULT 0,
  `canal` varchar(4) DEFAULT NULL,
  `forzarimprimirvertical` tinyint(4) NOT NULL DEFAULT 0,
  `porccrecimiento` decimal(16,2) DEFAULT NULL,
  `sucremotarelacion` varchar(2) DEFAULT NULL,
  `usuremotorelacion` varchar(10) DEFAULT NULL,
  `regimenfiscal` varchar(3) DEFAULT NULL,
  `sociedadmercantil` int(11) DEFAULT NULL,
  `claveempleado` varchar(10) DEFAULT NULL,
  `esAsociado` tinyint(4) NOT NULL DEFAULT 0,
  `credMax` decimal(16,2) NOT NULL DEFAULT 0.00,
  PRIMARY KEY (`cliente`),
  KEY `rsocial` (`rsocial`),
  KEY `nombre` (`nombre`,`appat`,`apmat`),
  KEY `rfc` (`rfc`),
  KEY `contacto` (`contacto`),
  KEY `apellido` (`appat`,`apmat`,`nombre`),
  KEY `nomnegocio` (`nomnegocio`),
  KEY `sucursal` (`sucursal`),
  KEY `colonia` (`colonia`),
  KEY `clientes_ibfk_6` (`usocfdi`),
  KEY `giro` (`giro`),
  KEY `canal` (`canal`),
  KEY `clientes_ibfk_10` (`metododef`),
  KEY `clientes_ibfk_11` (`metodosup`),
  KEY `clientes_ibfk_13` (`sociedadmercantil`),
  KEY `clientes_ibfk_12` (`regimenfiscal`),
  CONSTRAINT `clientes_ibfk_1` FOREIGN KEY (`sucursal`) REFERENCES `sucursales` (`sucursal`) ON UPDATE CASCADE,
  CONSTRAINT `clientes_ibfk_10` FOREIGN KEY (`metododef`) REFERENCES `cformapago` (`formapago`) ON UPDATE CASCADE,
  CONSTRAINT `clientes_ibfk_11` FOREIGN KEY (`metodosup`) REFERENCES `cformapago` (`formapago`) ON UPDATE CASCADE,
  CONSTRAINT `clientes_ibfk_12` FOREIGN KEY (`regimenfiscal`) REFERENCES `cregimenfiscal` (`regimenfiscal`) ON UPDATE CASCADE,
  CONSTRAINT `clientes_ibfk_13` FOREIGN KEY (`sociedadmercantil`) REFERENCES `catsociedadesmercantiles` (`id`) ON UPDATE CASCADE,
  CONSTRAINT `clientes_ibfk_2` FOREIGN KEY (`colonia`) REFERENCES `colonias` (`colonia`) ON UPDATE CASCADE,
  CONSTRAINT `clientes_ibfk_6` FOREIGN KEY (`usocfdi`) REFERENCES `cusocfdi` (`usocfdi`) ON UPDATE CASCADE,
  CONSTRAINT `clientes_ibfk_7` FOREIGN KEY (`giro`) REFERENCES `gironegocio` (`giro`) ON UPDATE CASCADE,
  CONSTRAINT `clientes_ibfk_8` FOREIGN KEY (`canal`) REFERENCES `canalesclientes` (`canal`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

