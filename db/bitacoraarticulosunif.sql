/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `bitacoraarticulosunif`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `bitacoraarticulosunif` (
  `idbitacora` int(11) NOT NULL AUTO_INCREMENT,
  `articulo` varchar(9) NOT NULL DEFAULT '0',
  `producto` varchar(8) NOT NULL DEFAULT '',
  `present` varchar(13) NOT NULL DEFAULT '',
  `multiplo` varchar(10) NOT NULL DEFAULT '',
  `ean13` varchar(14) DEFAULT NULL,
  `peso` decimal(16,3) DEFAULT NULL,
  `volumen` decimal(16,3) DEFAULT NULL,
  `altura` decimal(16,3) DEFAULT NULL,
  `longitud` decimal(16,3) DEFAULT NULL,
  `profundidad` decimal(16,3) DEFAULT NULL,
  `tipoutil` tinyint(1) DEFAULT NULL,
  `porccomi` decimal(5,2) DEFAULT NULL,
  `factor` decimal(10,3) DEFAULT NULL,
  `activo` tinyint(1) unsigned NOT NULL DEFAULT 0,
  `asigautosu` tinyint(1) unsigned NOT NULL DEFAULT 0,
  `fechaalta` date NOT NULL,
  `fechamodi` date NOT NULL,
  `horaalta` time NOT NULL,
  `horamodi` time NOT NULL,
  `usualta` varchar(10) DEFAULT NULL,
  `usumodi` varchar(10) DEFAULT NULL,
  `agregado` tinyint(1) NOT NULL,
  `descripcion` varchar(500) DEFAULT 'ERROR',
  PRIMARY KEY (`idbitacora`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

