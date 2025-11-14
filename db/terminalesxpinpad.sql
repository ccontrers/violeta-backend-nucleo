/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `terminalesxpinpad`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `terminalesxpinpad` (
  `terminal` varchar(10) NOT NULL DEFAULT '',
  `usuario` varchar(80) NOT NULL DEFAULT '',
  `password` varchar(80) NOT NULL DEFAULT '',
  `activo` int(1) NOT NULL DEFAULT 0,
  `product` varchar(20) NOT NULL DEFAULT '',
  `pn` varchar(30) NOT NULL DEFAULT '',
  `sn` varchar(30) NOT NULL DEFAULT '',
  `fcc_id` varchar(20) NOT NULL DEFAULT '',
  `ic` varchar(20) NOT NULL DEFAULT '',
  `tipomoneda` int(11) NOT NULL DEFAULT 484,
  `permitecancel` int(1) NOT NULL DEFAULT 0,
  `permitedevolu` int(1) NOT NULL DEFAULT 0,
  `permiteajuste` int(1) NOT NULL DEFAULT 0,
  PRIMARY KEY (`terminal`),
  CONSTRAINT `terminal_terminales_fk1` FOREIGN KEY (`terminal`) REFERENCES `terminales` (`terminal`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

