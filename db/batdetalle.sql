/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `batdetalle`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `batdetalle` (
  `iddetalle` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `idenviobat` int(11) NOT NULL DEFAULT 0,
  `programaid` int(11) NOT NULL DEFAULT 0,
  `proveedorid` int(11) NOT NULL DEFAULT 0,
  `idventa` int(11) unsigned NOT NULL DEFAULT 0,
  `sucursalid` varchar(100) NOT NULL DEFAULT '',
  `numterminal` int(9) NOT NULL DEFAULT 0,
  `referencia` varchar(11) NOT NULL DEFAULT '',
  `usuario` varchar(100) NOT NULL DEFAULT '',
  `cantidad` decimal(12,2) NOT NULL,
  `idfabricante` varchar(100) NOT NULL DEFAULT '',
  `nombrefabricante` varchar(100) NOT NULL DEFAULT '',
  `nomproducto` varchar(100) NOT NULL,
  `articulo` varchar(9) NOT NULL,
  `tipotransaccion` int(1) NOT NULL DEFAULT 0,
  `fechaventa` varchar(10) NOT NULL,
  `tipoquery` int(1) NOT NULL DEFAULT 0,
  PRIMARY KEY (`iddetalle`),
  KEY `idenviobat` (`idenviobat`),
  CONSTRAINT `fk_batdetalle_id` FOREIGN KEY (`idenviobat`) REFERENCES `batenvios` (`idenviobat`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

