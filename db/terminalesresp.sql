/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `terminalesresp`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `terminalesresp` (
  `terminal` varchar(10) NOT NULL DEFAULT '',
  `nombre` varchar(40) NOT NULL DEFAULT '',
  `usuario` varchar(10) DEFAULT NULL,
  `depart` varchar(10) DEFAULT NULL,
  `seccion` varchar(4) NOT NULL DEFAULT '',
  `almadefa` varchar(4) NOT NULL DEFAULT '',
  `uso` varchar(40) DEFAULT NULL,
  `ubicacion` varchar(40) DEFAULT NULL,
  `puntoventa` tinyint(1) DEFAULT NULL,
  `tipoimprfo` varchar(1) DEFAULT NULL,
  `asigfolvta` int(1) DEFAULT 0,
  `destimprvta` varchar(10) DEFAULT '',
  `serievta` varchar(2) DEFAULT '',
  `folvta` int(9) DEFAULT 0,
  `imprautovta` tinyint(1) DEFAULT 0,
  `tipoimprti` varchar(1) DEFAULT NULL,
  `destimprncred` varchar(10) DEFAULT NULL,
  `asigfolncred` int(1) DEFAULT NULL,
  `seriencred` varchar(2) DEFAULT NULL,
  `folncred` int(9) DEFAULT NULL,
  `imprautoncred` tinyint(1) DEFAULT NULL,
  `destimprncar` varchar(10) DEFAULT NULL,
  `asigfolncar` int(1) DEFAULT NULL,
  `seriencar` varchar(2) DEFAULT NULL,
  `folncar` int(9) DEFAULT NULL,
  `foltickets` varchar(21) DEFAULT '',
  `validusuvta` tinyint(1) DEFAULT 0,
  `imprautoncar` tinyint(1) DEFAULT NULL,
  `imprticksur` tinyint(1) DEFAULT NULL,
  `esmovil` tinyint(1) DEFAULT 0,
  `anchofolvta` int(2) DEFAULT 0,
  `anchofolncar` int(2) DEFAULT 0,
  `anchofolncred` int(2) DEFAULT 0,
  `imprecpagcli` tinyint(1) DEFAULT 0,
  `nomimpresoracfd` varchar(50) DEFAULT NULL,
  `rotuloventc1` varchar(30) DEFAULT '',
  `rotuloventc2` varchar(30) DEFAULT '',
  `rotuloventc3` varchar(30) DEFAULT '',
  `snselloventc1` tinyint(1) DEFAULT 0,
  `snselloventc2` tinyint(1) DEFAULT 0,
  `snselloventc3` tinyint(1) DEFAULT 0,
  `impcfdventc1` tinyint(1) DEFAULT 0,
  `impcfdventc2` tinyint(1) DEFAULT 0,
  `impcfdventc3` tinyint(1) DEFAULT 0,
  `rotuloncarc1` varchar(30) DEFAULT '',
  `rotuloncarc2` varchar(30) DEFAULT '',
  `rotuloncarc3` varchar(30) DEFAULT '',
  `snselloncarc1` tinyint(1) DEFAULT 0,
  `snselloncarc2` tinyint(1) DEFAULT 0,
  `snselloncarc3` tinyint(1) DEFAULT 0,
  `impcfdncarc1` tinyint(1) DEFAULT 0,
  `impcfdncarc2` tinyint(1) DEFAULT 0,
  `impcfdncarc3` tinyint(1) DEFAULT 0,
  `rotuloncrec1` varchar(30) DEFAULT '',
  `rotuloncrec2` varchar(30) DEFAULT '',
  `rotuloncrec3` varchar(30) DEFAULT '',
  `snselloncrec1` tinyint(1) DEFAULT 0,
  `snselloncrec2` tinyint(1) DEFAULT 0,
  `snselloncrec3` tinyint(1) DEFAULT 0,
  `impcfdncrec1` tinyint(1) DEFAULT 0,
  `impcfdncrec2` tinyint(1) DEFAULT 0,
  `impcfdncrec3` tinyint(1) DEFAULT 0,
  `solocredvent1` tinyint(1) DEFAULT 0,
  `solocredvent2` tinyint(1) DEFAULT 0,
  `solocredvent3` tinyint(1) DEFAULT 0,
  `nomimpresoracfd2` varchar(50) DEFAULT NULL,
  `nomimpresoracfd3` varchar(50) DEFAULT NULL,
  `nomimpresorancar` varchar(50) DEFAULT NULL,
  `nomimpresorancar2` varchar(50) DEFAULT NULL,
  `nomimpresorancar3` varchar(50) DEFAULT NULL,
  `nomimpresorancre` varchar(50) DEFAULT NULL,
  `nomimpresorancre2` varchar(50) DEFAULT NULL,
  `nomimpresorancre3` varchar(50) DEFAULT NULL,
  `impresoratickets` varchar(50) DEFAULT 'tickets',
  `tipobascula` varchar(10) DEFAULT 'No',
  `puertobascula` varchar(10) DEFAULT 'COM10',
  `nomimpresorapago1` varchar(50) DEFAULT NULL,
  `nomimpresorapago2` varchar(50) DEFAULT NULL,
  `nomimpresorapago3` varchar(50) DEFAULT NULL,
  `rotulopago1` varchar(30) DEFAULT '',
  `rotulopago2` varchar(30) DEFAULT '',
  `rotulopago3` varchar(30) DEFAULT '',
  `impcfdpago1` tinyint(1) DEFAULT 0,
  `impcfdpago2` tinyint(1) DEFAULT 0,
  `impcfdpago3` tinyint(1) DEFAULT 0,
  `forzarcortetickets` tinyint(1) DEFAULT 1,
  `impresoracred` varchar(50) DEFAULT NULL,
  `nomimpresorakiosko` varchar(50) DEFAULT NULL,
  `numintentoretiros` int(11) NOT NULL DEFAULT 1
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

