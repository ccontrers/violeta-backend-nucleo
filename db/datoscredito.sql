/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `datoscredito`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `datoscredito` (
  `cliente` varchar(11) NOT NULL DEFAULT '',
  `cliautos` int(2) DEFAULT NULL,
  `cliingre` decimal(16,2) DEFAULT NULL,
  `cliegre` decimal(16,2) DEFAULT NULL,
  `p1tipo` varchar(1) DEFAULT NULL,
  `p1valor` decimal(16,2) DEFAULT NULL,
  `p1hipoteca` decimal(16,2) DEFAULT NULL,
  `p1hnombre` varchar(40) DEFAULT NULL,
  `p1dir` varchar(40) DEFAULT NULL,
  `p2tipo` varchar(1) DEFAULT NULL,
  `p2valor` decimal(16,2) DEFAULT NULL,
  `p2hipoteca` decimal(16,2) DEFAULT NULL,
  `p2hnombre` varchar(40) DEFAULT NULL,
  `p2dir` varchar(40) DEFAULT NULL,
  `a1nombre` varchar(40) DEFAULT NULL,
  `a1dir` varchar(40) DEFAULT NULL,
  `a1tel` varchar(20) DEFAULT NULL,
  `a1tra` varchar(40) DEFAULT NULL,
  `a1puesto` varchar(20) DEFAULT NULL,
  `a1antig` int(2) DEFAULT NULL,
  `a1teltra` varchar(20) DEFAULT NULL,
  `a1dirtra` varchar(60) DEFAULT NULL,
  `a1casa` tinyint(1) DEFAULT NULL,
  `a1autos` int(2) DEFAULT NULL,
  `a1ingre` decimal(16,2) DEFAULT NULL,
  `a1egre` decimal(16,2) DEFAULT NULL,
  `a2nombre` varchar(40) DEFAULT NULL,
  `a2dir` varchar(40) DEFAULT NULL,
  `a2tel` varchar(20) DEFAULT NULL,
  `a2tra` varchar(40) DEFAULT NULL,
  `a2puesto` varchar(20) DEFAULT NULL,
  `a2antig` int(2) DEFAULT NULL,
  `a2teltra` varchar(20) DEFAULT NULL,
  `a2dirtra` varchar(60) DEFAULT NULL,
  `a2casa` tinyint(1) DEFAULT NULL,
  `a2autos` int(2) DEFAULT NULL,
  `a2ingre` decimal(16,2) DEFAULT NULL,
  `a2egre` decimal(16,2) DEFAULT NULL,
  `rf1nom` varchar(40) DEFAULT NULL,
  `rf1dir` varchar(60) DEFAULT NULL,
  `rf1tel` varchar(20) DEFAULT NULL,
  `rf1par` varchar(10) DEFAULT NULL,
  `rf2nom` varchar(40) DEFAULT NULL,
  `rf2dir` varchar(60) DEFAULT NULL,
  `rf2tel` varchar(20) DEFAULT NULL,
  `rf2par` varchar(10) DEFAULT NULL,
  `rnf1nom` varchar(40) DEFAULT NULL,
  `rnf1dir` varchar(60) DEFAULT NULL,
  `rnf1tel` varchar(20) DEFAULT NULL,
  `rnf1rel` varchar(10) DEFAULT NULL,
  `rnf2nom` varchar(40) DEFAULT NULL,
  `rnf2dir` varchar(60) DEFAULT NULL,
  `rnf2tel` varchar(20) DEFAULT NULL,
  `rnf2rel` varchar(10) DEFAULT NULL,
  `rb1banco` varchar(40) DEFAULT NULL,
  `rb1suc` varchar(40) DEFAULT NULL,
  `rb1telsuc` varchar(20) DEFAULT NULL,
  `rb1cuenta` varchar(20) DEFAULT NULL,
  `rb1antig` int(2) DEFAULT NULL,
  `rb1limcred` decimal(16,2) DEFAULT NULL,
  `rb1adeudos` decimal(16,2) DEFAULT NULL,
  `rb2banco` varchar(40) DEFAULT NULL,
  `rb2suc` varchar(40) DEFAULT NULL,
  `rb2telsuc` varchar(20) DEFAULT NULL,
  `rb2cuenta` varchar(20) DEFAULT NULL,
  `rb2antig` int(2) DEFAULT NULL,
  `rb2limcred` decimal(16,2) DEFAULT NULL,
  `rb2adeudos` decimal(16,2) DEFAULT NULL,
  `rc1emp` varchar(60) DEFAULT NULL,
  `rc1contacto` varchar(40) DEFAULT NULL,
  `rc1telefono` varchar(20) DEFAULT NULL,
  `rc2emp` varchar(60) DEFAULT NULL,
  `rc2contacto` varchar(40) DEFAULT NULL,
  `rc2telefono` varchar(20) DEFAULT NULL,
  `podernom` varchar(40) DEFAULT NULL,
  `poderpar` varchar(20) DEFAULT NULL,
  `poderpuest` varchar(20) DEFAULT NULL,
  `aut1nom` varchar(40) DEFAULT NULL,
  `aut1puest` varchar(20) DEFAULT NULL,
  `aut2nom` varchar(40) DEFAULT NULL,
  `aut2puest` varchar(20) DEFAULT NULL,
  `aut3nom` varchar(40) DEFAULT NULL,
  `aut3puest` varchar(20) DEFAULT NULL,
  `ventapm` decimal(16,2) DEFAULT NULL,
  `comentcr` varchar(80) DEFAULT NULL,
  PRIMARY KEY (`cliente`),
  CONSTRAINT `datoscredito_ibfk_1` FOREIGN KEY (`cliente`) REFERENCES `clientes` (`cliente`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

