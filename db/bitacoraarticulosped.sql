/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `bitacoraarticulosped`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `bitacoraarticulosped` (
  `producto` varchar(8) DEFAULT NULL,
  `present` varchar(13) DEFAULT NULL,
  `usuario` varchar(10) NOT NULL,
  `fechamodi` date NOT NULL,
  `horamodi` time NOT NULL,
  `claveprodprovantes` varchar(15) DEFAULT NULL,
  `claveprodprovdesp` varchar(15) DEFAULT NULL,
  `dureordenantes` decimal(5,2) DEFAULT NULL,
  `dureordendes` decimal(5,2) DEFAULT NULL,
  `durnmaxantes` decimal(5,2) DEFAULT NULL,
  `durnmaxdes` decimal(5,2) DEFAULT NULL,
  `descontinuadoant` tinyint(4) DEFAULT NULL,
  `descontinuadodesp` tinyint(4) DEFAULT NULL,
  `redondeocajaant` tinyint(4) DEFAULT NULL,
  `redondeocajades` tinyint(4) DEFAULT NULL,
  `multiplopedirant` varchar(10) DEFAULT NULL,
  `multiplopedirdes` varchar(10) DEFAULT NULL,
  `stockminimoant` tinyint(4) DEFAULT NULL,
  `stockminimodes` tinyint(4) DEFAULT NULL,
  `moduloconfigartped` tinyint(1) DEFAULT NULL,
  `todassucursales` tinyint(1) DEFAULT NULL,
  `sucursal` varchar(2) DEFAULT NULL,
  KEY `bitacoraarticulosped_ibfk_1` (`producto`,`present`),
  KEY `bitacoraarticulosped_ibfk_2` (`sucursal`),
  CONSTRAINT `bitacoraarticulosped_ibfk_1` FOREIGN KEY (`producto`, `present`) REFERENCES `presentaciones` (`producto`, `present`) ON UPDATE CASCADE,
  CONSTRAINT `bitacoraarticulosped_ibfk_2` FOREIGN KEY (`sucursal`) REFERENCES `sucursales` (`sucursal`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

