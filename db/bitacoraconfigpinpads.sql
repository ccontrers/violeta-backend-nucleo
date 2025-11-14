/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `bitacoraconfigpinpads`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `bitacoraconfigpinpads` (
  `ip` varchar(20) NOT NULL DEFAULT '',
  `puerto` varchar(20) NOT NULL DEFAULT '',
  `ip_update` varchar(20) NOT NULL DEFAULT '',
  `puerto_update` varchar(20) DEFAULT '',
  `timeout` varchar(10) NOT NULL DEFAULT '',
  `llave` varchar(150) NOT NULL DEFAULT '',
  `negocio1` varchar(20) NOT NULL DEFAULT '',
  `terminal1` varchar(20) DEFAULT '',
  `negocio2` varchar(20) DEFAULT '',
  `terminal2` varchar(20) DEFAULT '',
  `puertocom` varchar(10) NOT NULL DEFAULT '',
  `cashback` int(11) NOT NULL DEFAULT 0,
  `contatactless` int(11) NOT NULL DEFAULT 0,
  `socketip` varchar(20) NOT NULL DEFAULT '',
  `socketport` varchar(20) NOT NULL DEFAULT '',
  `http_address` varchar(20) NOT NULL DEFAULT '',
  `http_port` varchar(20) NOT NULL DEFAULT '',
  `settingippinpad` varchar(20) NOT NULL DEFAULT '',
  `settingpinpad` varchar(20) NOT NULL DEFAULT '',
  `settingprintcopy` varchar(20) NOT NULL DEFAULT '',
  `dcs_form` varchar(10) NOT NULL DEFAULT '',
  `usuariomodi` varchar(10) DEFAULT '',
  `fechamodi` date NOT NULL DEFAULT curdate(),
  `horamodi` time NOT NULL DEFAULT curtime(),
  KEY `usuariomodi_bitacoraconfigpinpads_fk1` (`usuariomodi`),
  CONSTRAINT `usuariomodi_bitacoraconfigpinpads_fk1` FOREIGN KEY (`usuariomodi`) REFERENCES `empleados` (`empleado`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

