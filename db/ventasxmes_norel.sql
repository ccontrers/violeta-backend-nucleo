/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `ventasxmes_norel`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ventasxmes_norel` (
  `almacen` varchar(4) NOT NULL,
  `producto` varchar(8) NOT NULL,
  `present` varchar(13) NOT NULL,
  `cant1` decimal(12,3) NOT NULL,
  `cant2` decimal(12,3) NOT NULL,
  `cant3` decimal(12,3) NOT NULL,
  `cant4` decimal(12,3) NOT NULL,
  `cant5` decimal(12,3) NOT NULL,
  `cant6` decimal(12,3) NOT NULL,
  `cant7` decimal(12,3) NOT NULL,
  `cant8` decimal(12,3) NOT NULL,
  `cant9` decimal(12,3) NOT NULL,
  `cant10` decimal(12,3) NOT NULL,
  `cant11` decimal(12,3) NOT NULL,
  `cant12` decimal(12,3) NOT NULL,
  `cant13` decimal(12,3) NOT NULL,
  `cant14` decimal(12,3) NOT NULL,
  `cant15` decimal(12,3) NOT NULL,
  `cant16` decimal(12,3) NOT NULL,
  `cant17` decimal(12,3) NOT NULL,
  `cant18` decimal(12,3) NOT NULL,
  `cant19` decimal(12,3) NOT NULL,
  `cant20` decimal(12,3) NOT NULL,
  `cant21` decimal(12,3) NOT NULL,
  `cant22` decimal(12,3) NOT NULL,
  `cant23` decimal(12,3) NOT NULL,
  `cant24` decimal(12,3) NOT NULL,
  `cant25` decimal(12,3) NOT NULL,
  `cant26` decimal(12,3) NOT NULL,
  `cant27` decimal(12,3) NOT NULL,
  `cant28` decimal(12,3) NOT NULL,
  `cant29` decimal(12,3) NOT NULL,
  `cant30` decimal(12,3) NOT NULL,
  `cant31` decimal(12,3) NOT NULL,
  `cant32` decimal(12,3) NOT NULL,
  `cant33` decimal(12,3) NOT NULL,
  `cant34` decimal(12,3) NOT NULL,
  `cant35` decimal(12,3) NOT NULL,
  `cant36` decimal(12,3) NOT NULL,
  `ventas30` decimal(12,3) DEFAULT 0.000,
  `ventas60` decimal(12,3) DEFAULT 0.000,
  `ventas90` decimal(12,3) DEFAULT 0.000,
  `ventas180` decimal(12,3) NOT NULL,
  `ventascorte` decimal(12,3) NOT NULL,
  PRIMARY KEY (`almacen`,`producto`,`present`),
  KEY `prodpres` (`producto`,`present`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

