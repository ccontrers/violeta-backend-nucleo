/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `cartasporte20`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `cartasporte20` (
  `cartaporte20` varchar(20) NOT NULL,
  `referencia` varchar(11) DEFAULT NULL,
  `pedido` varchar(11) DEFAULT NULL,
  `pedidocompra` varchar(11) DEFAULT NULL,
  `referenciacompra` varchar(11) DEFAULT NULL,
  `rfcremitente` varchar(13) NOT NULL,
  `cpremitente` int(11) NOT NULL DEFAULT 6510,
  `fechasalida` date DEFAULT curtime(),
  `horasalida` time DEFAULT curtime(),
  `rfcdestinatario` varchar(15) DEFAULT NULL,
  `cpdestinatario` int(11) NOT NULL DEFAULT 6510,
  `fechallegada` date DEFAULT curdate(),
  `horallegada` time DEFAULT curtime(),
  `viaembarq` varchar(10) DEFAULT NULL,
  `placavm` varchar(10) DEFAULT NULL,
  `aniomodelovm` varchar(11) DEFAULT NULL,
  `asegurcivil` varchar(10) DEFAULT NULL,
  `polizarespcivil` varchar(50) DEFAULT NULL,
  `idremolque` int(11) DEFAULT NULL,
  `tiporemolque` varchar(11) DEFAULT NULL,
  `placaremolque` varchar(11) DEFAULT NULL,
  `permsct` varchar(20) DEFAULT NULL,
  `numpermsct` varchar(50) DEFAULT NULL,
  `pesobruttotal` decimal(16,6) DEFAULT NULL,
  `unidadpeso` varchar(3) DEFAULT NULL,
  `pesonettot` decimal(16,6) DEFAULT NULL,
  `numtotmerc` int(11) DEFAULT NULL,
  `tipofig` varchar(2) DEFAULT '01',
  `chofer` varchar(10) DEFAULT NULL,
  `rfcfig` varchar(15) NOT NULL DEFAULT 'XAXX010101000',
  `numlicfig` varchar(20) NOT NULL DEFAULT '000000',
  `version` decimal(16,2) DEFAULT 2.00,
  `transpinter` tinyint(1) DEFAULT 0,
  `usualta` varchar(10) DEFAULT NULL,
  `usumodi` varchar(10) DEFAULT NULL,
  `fechaalta` date DEFAULT curdate(),
  `horaalta` time DEFAULT curtime(),
  `fechacp` date DEFAULT curdate(),
  `horacp` time DEFAULT curtime(),
  `fechamod` date DEFAULT curdate(),
  `horamod` time DEFAULT curtime(),
  `terminal` varchar(10) DEFAULT '',
  `embarque` varchar(11) DEFAULT NULL,
  `impresa` tinyint(1) DEFAULT 0,
  `cancelada` tinyint(1) DEFAULT 0,
  `idconfigvehicular` tinyint(11) NOT NULL,
  `configvehicular` varchar(8) NOT NULL,
  PRIMARY KEY (`cartaporte20`),
  UNIQUE KEY `cartaporte20` (`cartaporte20`),
  KEY `referencia` (`referencia`),
  KEY `pedido` (`pedido`),
  KEY `rfcdestinatario` (`rfcdestinatario`),
  KEY `terminal` (`terminal`),
  KEY `fechacp` (`fechacp`),
  KEY `usualta` (`usualta`),
  KEY `usumodi` (`usumodi`),
  KEY `embarque` (`embarque`),
  KEY `cartasp20_ibfk_4` (`viaembarq`),
  KEY `cartasp20_ibfk_9` (`referenciacompra`),
  KEY `cartasp20_ibfk_10` (`pedidocompra`),
  KEY `rfcfig` (`rfcfig`) USING BTREE,
  KEY `cpremitente` (`cpremitente`) USING BTREE,
  KEY `cpdestinatario` (`cpdestinatario`) USING BTREE,
  KEY `chofer` (`chofer`) USING BTREE,
  KEY `idremolque` (`idremolque`) USING BTREE,
  CONSTRAINT `cartasp20_ibfk_1` FOREIGN KEY (`terminal`) REFERENCES `terminales` (`terminal`) ON UPDATE CASCADE,
  CONSTRAINT `cartasp20_ibfk_10` FOREIGN KEY (`pedidocompra`) REFERENCES `pedidos` (`referencia`) ON UPDATE CASCADE,
  CONSTRAINT `cartasp20_ibfk_11` FOREIGN KEY (`cpremitente`) REFERENCES `ccpclavecodpos` (`idclavecodpos`) ON UPDATE CASCADE,
  CONSTRAINT `cartasp20_ibfk_12` FOREIGN KEY (`cpdestinatario`) REFERENCES `ccpclavecodpos` (`idclavecodpos`) ON UPDATE CASCADE,
  CONSTRAINT `cartasp20_ibfk_13` FOREIGN KEY (`chofer`) REFERENCES `choferes` (`empleado`) ON UPDATE CASCADE,
  CONSTRAINT `cartasp20_ibfk_14` FOREIGN KEY (`idremolque`) REFERENCES `catalogoremolques` (`idremolque`) ON UPDATE CASCADE,
  CONSTRAINT `cartasp20_ibfk_2` FOREIGN KEY (`usualta`) REFERENCES `usuarios` (`empleado`) ON UPDATE CASCADE,
  CONSTRAINT `cartasp20_ibfk_3` FOREIGN KEY (`usumodi`) REFERENCES `usuarios` (`empleado`) ON UPDATE CASCADE,
  CONSTRAINT `cartasp20_ibfk_4` FOREIGN KEY (`viaembarq`) REFERENCES `viasembarque` (`viaembarq`) ON UPDATE CASCADE,
  CONSTRAINT `cartasp20_ibfk_5` FOREIGN KEY (`embarque`) REFERENCES `embarques` (`embarque`) ON UPDATE CASCADE,
  CONSTRAINT `cartasp20_ibfk_6` FOREIGN KEY (`referencia`) REFERENCES `ventas` (`referencia`) ON UPDATE CASCADE,
  CONSTRAINT `cartasp20_ibfk_7` FOREIGN KEY (`pedido`) REFERENCES `pedidosventa` (`referencia`) ON UPDATE CASCADE,
  CONSTRAINT `cartasp20_ibfk_9` FOREIGN KEY (`referenciacompra`) REFERENCES `compras` (`referencia`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

