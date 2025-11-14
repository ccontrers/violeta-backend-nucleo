/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `configpinpadb`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `configpinpadb` (
  `idLogs` int(1) NOT NULL DEFAULT 1,
  `claveLogs` varchar(80) DEFAULT '',
  `pinpadConexion` varchar(10) NOT NULL DEFAULT '',
  `pinpadTimeOut` int(5) NOT NULL DEFAULT 0,
  `puertoWifi` varchar(80) DEFAULT '',
  `mensaje` varchar(80) NOT NULL DEFAULT '',
  `contactless` int(1) NOT NULL DEFAULT 0,
  `tecladoLiberado` int(1) NOT NULL DEFAULT 0,
  `binesExcepcion` varchar(80) DEFAULT '',
  `urlAutorizador` varchar(100) NOT NULL DEFAULT '',
  `urlBines` varchar(100) NOT NULL DEFAULT '',
  `urlToken` varchar(100) NOT NULL DEFAULT '',
  `urlTelecarga` varchar(100) NOT NULL DEFAULT '',
  `hostTimeOut` int(5) NOT NULL DEFAULT 0,
  `garanti` int(1) NOT NULL DEFAULT 0,
  `moto` int(1) NOT NULL DEFAULT 0,
  `afiliacion` varchar(80) NOT NULL DEFAULT '',
  `afiliacionUsd` varchar(80) DEFAULT '',
  `terminal` varchar(10) NOT NULL DEFAULT '',
  `idAplicacion` varchar(80) NOT NULL DEFAULT '',
  `claveSecreta` varchar(80) NOT NULL DEFAULT '',
  `terminalsist` varchar(10) NOT NULL DEFAULT '',
  `activo` int(1) NOT NULL DEFAULT 0,
  `permitecancel` int(1) NOT NULL DEFAULT 0,
  `permitedevolu` int(1) NOT NULL DEFAULT 0,
  `permitecash` int(1) NOT NULL DEFAULT 0,
  PRIMARY KEY (`terminalsist`) USING BTREE,
  CONSTRAINT `terminal_terminal_fk1` FOREIGN KEY (`terminalsist`) REFERENCES `terminales` (`terminal`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

