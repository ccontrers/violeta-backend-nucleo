/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `productos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `productos` (
  `producto` varchar(8) NOT NULL DEFAULT '',
  `nombre` varchar(60) DEFAULT NULL,
  `marca` varchar(10) DEFAULT NULL,
  `fabricante` varchar(10) NOT NULL,
  `clasif1` varchar(10) DEFAULT NULL,
  `clasif2` varchar(10) DEFAULT NULL,
  `clasif3` varchar(10) DEFAULT NULL,
  `claveimpv1` int(2) DEFAULT NULL,
  `claveimpv2` int(2) DEFAULT NULL,
  `claveimpv3` int(2) DEFAULT NULL,
  `claveimpv4` int(2) DEFAULT NULL,
  `claveimpc1` int(2) DEFAULT NULL,
  `claveimpc2` int(2) DEFAULT NULL,
  `claveimpc3` int(2) DEFAULT NULL,
  `claveimpc4` int(2) DEFAULT NULL,
  `clasifcont` varchar(5) DEFAULT NULL,
  `clasifcont2` varchar(5) DEFAULT NULL,
  `idgraduacion` varchar(3) NOT NULL DEFAULT '',
  `etiquetasgr` tinyint(1) DEFAULT 0,
  `idclaveprodservcfdi` int(11) DEFAULT 1,
  `clasifecom1` varchar(10) DEFAULT NULL,
  `clasifecom2` varchar(10) DEFAULT NULL,
  `clasifecom3` varchar(10) DEFAULT NULL,
  PRIMARY KEY (`producto`),
  UNIQUE KEY `nombre` (`nombre`),
  KEY `marca` (`marca`),
  KEY `clasif1` (`clasif1`),
  KEY `clasif2` (`clasif2`),
  KEY `clasif3` (`clasif3`),
  KEY `clasifcont` (`clasifcont`),
  KEY `clasifcont2` (`clasifcont2`),
  KEY `idgraduacion` (`idgraduacion`),
  KEY `productos_ibfk_7` (`idclaveprodservcfdi`),
  KEY `productos_ibfk8` (`fabricante`),
  KEY `productos_ibfk_11` (`clasifecom1`),
  KEY `productos_ibfk_12` (`clasifecom2`),
  KEY `productos_ibfk_13` (`clasifecom3`),
  CONSTRAINT `productos_ibfk8` FOREIGN KEY (`fabricante`) REFERENCES `fabricantes` (`fabricante`) ON UPDATE CASCADE,
  CONSTRAINT `productos_ibfk_1` FOREIGN KEY (`marca`) REFERENCES `marcas` (`marca`) ON UPDATE CASCADE,
  CONSTRAINT `productos_ibfk_10` FOREIGN KEY (`idgraduacion`) REFERENCES `satgraduacion` (`idgraduacion`) ON UPDATE CASCADE,
  CONSTRAINT `productos_ibfk_11` FOREIGN KEY (`clasifecom1`) REFERENCES `clasificacionecommerce1` (`clasif1`) ON UPDATE CASCADE,
  CONSTRAINT `productos_ibfk_12` FOREIGN KEY (`clasifecom2`) REFERENCES `clasificacionecommerce2` (`clasif2`) ON UPDATE CASCADE,
  CONSTRAINT `productos_ibfk_13` FOREIGN KEY (`clasifecom3`) REFERENCES `clasificacionecommerce3` (`clasif3`) ON UPDATE CASCADE,
  CONSTRAINT `productos_ibfk_2` FOREIGN KEY (`clasif1`) REFERENCES `clasificacion1` (`clasif1`) ON UPDATE CASCADE,
  CONSTRAINT `productos_ibfk_3` FOREIGN KEY (`clasif2`) REFERENCES `clasificacion2` (`clasif2`) ON UPDATE CASCADE,
  CONSTRAINT `productos_ibfk_4` FOREIGN KEY (`clasif3`) REFERENCES `clasificacion3` (`clasif3`) ON UPDATE CASCADE,
  CONSTRAINT `productos_ibfk_5` FOREIGN KEY (`clasifcont`) REFERENCES `clasifcont` (`clasif`) ON UPDATE CASCADE,
  CONSTRAINT `productos_ibfk_6` FOREIGN KEY (`clasifcont2`) REFERENCES `clasifcont2` (`clasif2`) ON UPDATE CASCADE,
  CONSTRAINT `productos_ibfk_7` FOREIGN KEY (`idclaveprodservcfdi`) REFERENCES `cclaveprodserv` (`idclaveprodserv`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

