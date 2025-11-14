/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `pedidoautomatico_imitar`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `pedidoautomatico_imitar` (
  `producto_incompleto` varchar(8) NOT NULL,
  `present_incompleto` varchar(13) NOT NULL,
  `producto_imitar` varchar(8) NOT NULL,
  `present_imitar` varchar(13) NOT NULL,
  `factor_imitar` int(10) unsigned DEFAULT NULL CHECK (`factor_imitar` > 0),
  `fecha_limite` date NOT NULL,
  PRIMARY KEY (`producto_incompleto`,`present_incompleto`),
  KEY `´pedidoautomatico_imitar_ibfk_2´` (`producto_imitar`,`present_imitar`),
  CONSTRAINT `´pedidoautomatico_imitar_ibfk_1´` FOREIGN KEY (`producto_incompleto`, `present_incompleto`) REFERENCES `presentaciones` (`producto`, `present`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `´pedidoautomatico_imitar_ibfk_2´` FOREIGN KEY (`producto_imitar`, `present_imitar`) REFERENCES `presentaciones` (`producto`, `present`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

