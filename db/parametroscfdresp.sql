/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `parametroscfdresp`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `parametroscfdresp` (
  `parametro` int(11) NOT NULL DEFAULT 0,
  `sucursal` varchar(2) NOT NULL,
  `certificado` text DEFAULT NULL,
  `llaveprivada` text NOT NULL,
  `passcert` varchar(80) NOT NULL,
  `numseriecert` varchar(30) DEFAULT NULL,
  `requerimiento` text NOT NULL,
  `nombrecert` varchar(70) DEFAULT NULL,
  `nombrellavepriv` varchar(70) NOT NULL,
  `nombrereq` varchar(70) NOT NULL,
  `fechaalta` date NOT NULL,
  `fechamodi` date NOT NULL,
  `horaalta` time NOT NULL,
  `horamodi` time NOT NULL,
  `usualta` varchar(10) NOT NULL,
  `usumodi` varchar(10) NOT NULL,
  `rfcemisor` varchar(13) NOT NULL,
  `nombreemisor` varchar(50) NOT NULL,
  `calleemisor` varchar(80) NOT NULL,
  `numextemisor` varchar(10) NOT NULL,
  `numintemisor` varchar(10) DEFAULT NULL,
  `coloniaemisor` varchar(80) NOT NULL,
  `municipioemisor` varchar(80) NOT NULL,
  `localidademisor` varchar(80) DEFAULT NULL,
  `estadoemisor` varchar(40) DEFAULT NULL,
  `referenemisor` varchar(50) DEFAULT NULL,
  `cpemisor` varchar(5) NOT NULL,
  `incluirexp` tinyint(1) NOT NULL,
  `calleexp` varchar(80) NOT NULL,
  `numextexp` varchar(10) NOT NULL,
  `numintexp` varchar(10) DEFAULT NULL,
  `coloniaexp` varchar(80) NOT NULL,
  `municipioexp` varchar(80) NOT NULL,
  `localidadexp` varchar(80) DEFAULT NULL,
  `estadoexp` varchar(40) DEFAULT NULL,
  `cpexp` varchar(5) NOT NULL,
  `referenexp` varchar(50) DEFAULT NULL,
  `servidor` varchar(20) NOT NULL,
  `usuario` varchar(20) NOT NULL,
  `clave` varchar(20) NOT NULL,
  `empresa` varchar(60) NOT NULL,
  `telefonos` varchar(50) DEFAULT NULL,
  `telefonosexp` varchar(50) DEFAULT NULL,
  `email` varchar(50) DEFAULT NULL,
  `emailexp` varchar(50) DEFAULT NULL,
  `pagweb` varchar(50) DEFAULT NULL,
  `pagwebexp` varchar(50) DEFAULT NULL,
  `usuariocontpaq` varchar(20) DEFAULT NULL,
  `instancia` varchar(20) DEFAULT NULL,
  `imagen1` text DEFAULT NULL,
  `imagen2` text DEFAULT NULL,
  `imagen3` text DEFAULT NULL,
  `nombreimg1` varchar(40) DEFAULT NULL,
  `nombreimg2` varchar(40) DEFAULT NULL,
  `nombreimg3` varchar(40) DEFAULT NULL,
  `fechaexpicert` date DEFAULT NULL,
  `fechavenccert` date DEFAULT NULL,
  `poltipovent` int(11) DEFAULT 1,
  `poltiponcarcli` int(11) DEFAULT 1,
  `poltiponcrecli` int(11) DEFAULT 2,
  `poltipocomp` int(11) DEFAULT 0,
  `poltiponcarprov` int(11) DEFAULT 0,
  `poltiponcreprov` int(11) DEFAULT 0,
  `poltipocobr` int(11) DEFAULT 0,
  `poltipopago` int(11) DEFAULT 0,
  `poltipogasto` int(11) DEFAULT 0,
  `poltipopagogasto` int(11) DEFAULT 0,
  `poltiponcredgasto` int(11) DEFAULT 0,
  `nomenviocfd` varchar(50) DEFAULT NULL,
  `emailenviocfd` varchar(50) DEFAULT NULL,
  `htmlemail` mediumtext DEFAULT NULL,
  `hostsmtp` varchar(50) DEFAULT NULL,
  `portsmtp` varchar(5) DEFAULT NULL,
  `usersmtp` varchar(50) DEFAULT NULL,
  `passsmtp` varchar(50) DEFAULT NULL,
  `envioauto` tinyint(1) NOT NULL DEFAULT 0,
  `segmento` int(11) DEFAULT 0,
  `regimenfiscal` varchar(3) NOT NULL,
  `urledicom` varchar(80) DEFAULT NULL,
  `usuarioedicom` varchar(50) DEFAULT NULL,
  `passwordedicom` varchar(80) DEFAULT NULL,
  `nomcertedicom` varchar(70) DEFAULT NULL,
  `certedicom` mediumtext DEFAULT NULL,
  `passcertedicom` varchar(70) DEFAULT NULL,
  `urlwfactura` varchar(80) DEFAULT NULL,
  `urlcancwfactura` varchar(80) DEFAULT NULL,
  `usuariowfactura` varchar(20) DEFAULT NULL,
  `timbradoprueba` tinyint(1) NOT NULL DEFAULT 1,
  `pacseleccionado` tinyint(1) NOT NULL DEFAULT 0,
  `retimbrarauto` tinyint(1) NOT NULL DEFAULT 0,
  `imprdocspend` tinyint(1) NOT NULL DEFAULT 0,
  `htmlemailpedidos` mediumtext DEFAULT NULL,
  `urlcomerciodigital` varchar(100) DEFAULT NULL,
  `urlcanccomerciodigital` varchar(100) DEFAULT NULL,
  `usuariocomerciodigital` varchar(70) DEFAULT NULL,
  `passcomerciodigital` varchar(70) DEFAULT NULL,
  `pfxnombre_efirma` varchar(70) DEFAULT NULL,
  `pfx_efirma` mediumtext DEFAULT NULL,
  `pfxpass_efirma` varchar(30) DEFAULT NULL,
  `pem_efirma` mediumtext DEFAULT NULL,
  `htmlemailprepagos` mediumtext DEFAULT NULL,
  `imagencotped` mediumtext DEFAULT NULL,
  `nombreimgcotped` varchar(10) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

