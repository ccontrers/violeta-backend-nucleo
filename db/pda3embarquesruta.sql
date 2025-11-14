/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `pda3embarquesruta`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `pda3embarquesruta` (
  `idorden` int(11) NOT NULL DEFAULT 1,
  `embarque` varchar(11) NOT NULL DEFAULT '',
  `cliente` varchar(11) NOT NULL DEFAULT '',
  `calleynumero` varchar(80) DEFAULT NULL,
  `colonia` varchar(40) DEFAULT NULL,
  `localidad` varchar(40) DEFAULT NULL,
  `municipio` varchar(40) DEFAULT NULL,
  `referenciadomic` varchar(60) DEFAULT NULL,
  `cp` varchar(5) DEFAULT NULL,
  `ubicaciongisx` varchar(15) DEFAULT NULL,
  `ubicaciongisy` varchar(15) DEFAULT NULL,
  `fechaalta` date NOT NULL DEFAULT '0000-00-00',
  `fechadellegada` datetime DEFAULT '0000-00-00 00:00:00',
  `choferllega` varchar(10) DEFAULT NULL,
  `ubicaciondellegadax` varchar(15) DEFAULT NULL,
  `ubicaciondellegaday` varchar(15) DEFAULT NULL,
  `fechadesalida` datetime DEFAULT '0000-00-00 00:00:00',
  `chofersale` varchar(10) DEFAULT NULL,
  `ubicaciondesalidax` varchar(15) DEFAULT NULL,
  `ubicaciondesaliday` varchar(15) DEFAULT NULL,
  `observacioneschofer` varchar(80) DEFAULT NULL,
  `etapa` tinyint(1) DEFAULT 0,
  `mensaje` varchar(128) DEFAULT '',
  PRIMARY KEY (`embarque`,`cliente`,`idorden`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

