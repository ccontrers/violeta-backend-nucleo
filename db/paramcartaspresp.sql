/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `paramcartaspresp`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `paramcartaspresp` (
  `parametro` int(11) NOT NULL DEFAULT 0,
  `empresatrans` varchar(2) NOT NULL,
  `certificado` mediumtext DEFAULT NULL,
  `llaveprivada` mediumtext DEFAULT NULL,
  `passcert` varchar(200) DEFAULT NULL,
  `numseriecert` varchar(60) DEFAULT NULL,
  `requerimiento` mediumtext DEFAULT NULL,
  `nombrecert` varchar(70) DEFAULT NULL,
  `nombrellavepriv` varchar(70) DEFAULT NULL,
  `nombrereq` varchar(70) DEFAULT NULL,
  `fechaalta` date DEFAULT NULL,
  `fechamodi` date DEFAULT NULL,
  `horaalta` time DEFAULT NULL,
  `horamodi` time DEFAULT NULL,
  `usualta` varchar(10) DEFAULT NULL,
  `usumodi` varchar(10) DEFAULT NULL,
  `rfcemisor` varchar(13) DEFAULT NULL,
  `nombreemisor` varchar(255) DEFAULT NULL,
  `calleemisor` varchar(128) DEFAULT NULL,
  `numextemisor` varchar(10) DEFAULT NULL,
  `numintemisor` varchar(10) DEFAULT NULL,
  `coloniaemisor` varchar(128) DEFAULT NULL,
  `municipioemisor` varchar(128) DEFAULT NULL,
  `localidademisor` varchar(128) DEFAULT NULL,
  `estadoemisor` varchar(128) DEFAULT NULL,
  `referenemisor` varchar(100) DEFAULT NULL,
  `cpemisor` varchar(5) DEFAULT NULL,
  `regimenfiscal` varchar(255) DEFAULT NULL,
  `incluirexp` tinyint(1) DEFAULT NULL,
  `calleexp` varchar(128) DEFAULT NULL,
  `numextexp` varchar(10) DEFAULT NULL,
  `numintexp` varchar(10) DEFAULT NULL,
  `coloniaexp` varchar(128) DEFAULT NULL,
  `municipioexp` varchar(128) DEFAULT NULL,
  `localidadexp` varchar(128) DEFAULT NULL,
  `estadoexp` varchar(128) DEFAULT NULL,
  `cpexp` varchar(5) DEFAULT NULL,
  `referenexp` varchar(100) DEFAULT NULL,
  `telefonos` varchar(50) DEFAULT NULL,
  `telefonosexp` varchar(50) DEFAULT NULL,
  `email` varchar(50) DEFAULT NULL,
  `emailexp` varchar(50) DEFAULT NULL,
  `pagweb` varchar(50) DEFAULT NULL,
  `pagwebexp` varchar(50) DEFAULT NULL,
  `imagen1` mediumtext DEFAULT NULL,
  `imagen2` mediumtext DEFAULT NULL,
  `imagen3` mediumtext DEFAULT NULL,
  `nombreimg1` varchar(70) DEFAULT NULL,
  `nombreimg2` varchar(70) DEFAULT NULL,
  `nombreimg3` varchar(70) DEFAULT NULL,
  `fechaexpicert` date DEFAULT NULL,
  `fechavenccert` date DEFAULT NULL,
  `nomenviocfd` varchar(50) DEFAULT NULL,
  `emailenviocfd` varchar(50) DEFAULT NULL,
  `htmlemail` mediumtext DEFAULT NULL,
  `hostsmtp` varchar(50) DEFAULT NULL,
  `portsmtp` varchar(5) DEFAULT NULL,
  `usersmtp` varchar(50) DEFAULT NULL,
  `passsmtp` varchar(50) DEFAULT NULL,
  `envioauto` tinyint(1) DEFAULT 0,
  `urltimb` varchar(128) DEFAULT NULL,
  `usuariotimb` varchar(70) DEFAULT NULL,
  `passwordtimb` varchar(128) DEFAULT NULL,
  `nomcerttimb` varchar(70) DEFAULT NULL,
  `certtimb` mediumtext DEFAULT NULL,
  `passcerttimb` varchar(128) DEFAULT NULL,
  `timbradoprueba` tinyint(1) DEFAULT 1,
  `maniobras` decimal(16,6) DEFAULT 0.000000,
  `cartasiva` decimal(16,6) DEFAULT 16.000000,
  `cartasret` decimal(16,6) DEFAULT 4.000000,
  `clientepaga` varchar(11) DEFAULT NULL,
  `versioncfdi` varchar(4) DEFAULT '3.2',
  `cveprodflete` varchar(10) NOT NULL DEFAULT '',
  `cveprodmaniobra` varchar(10) NOT NULL DEFAULT '',
  `cveunidadflete` varchar(10) NOT NULL DEFAULT '',
  `cveunidadmaniobra` varchar(10) NOT NULL DEFAULT '',
  `formapago` varchar(2) DEFAULT NULL,
  `usocfdi` varchar(4) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

