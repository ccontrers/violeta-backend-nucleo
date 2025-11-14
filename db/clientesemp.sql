/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `clientesemp`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `clientesemp` (
  `cliente` varchar(11) NOT NULL DEFAULT '',
  `idempresa` int(2) NOT NULL,
  `tipoprecmin` varchar(2) DEFAULT NULL,
  `tipoprec` varchar(2) DEFAULT NULL,
  `vendedor` varchar(10) DEFAULT NULL,
  `cobrador` varchar(11) DEFAULT NULL,
  PRIMARY KEY (`cliente`,`idempresa`) USING BTREE,
  KEY `clienteemptp_2` (`idempresa`),
  KEY `clienteemptp_3` (`tipoprecmin`),
  KEY `clienteemptp_4` (`tipoprec`),
  KEY `vendedor_index` (`vendedor`),
  KEY `cobrador_index` (`cobrador`),
  CONSTRAINT `clienteemp_cobr1` FOREIGN KEY (`cobrador`) REFERENCES `cobradores` (`empleado`) ON UPDATE CASCADE,
  CONSTRAINT `clienteemp_vend1` FOREIGN KEY (`vendedor`) REFERENCES `vendedores` (`empleado`) ON UPDATE CASCADE,
  CONSTRAINT `clienteemptp_1` FOREIGN KEY (`cliente`) REFERENCES `clientes` (`cliente`) ON UPDATE CASCADE,
  CONSTRAINT `clienteemptp_2` FOREIGN KEY (`idempresa`) REFERENCES `empresas` (`idempresa`) ON UPDATE CASCADE,
  CONSTRAINT `clienteemptp_3` FOREIGN KEY (`tipoprecmin`) REFERENCES `tiposdeprecios` (`tipoprec`) ON UPDATE CASCADE,
  CONSTRAINT `clienteemptp_4` FOREIGN KEY (`tipoprec`) REFERENCES `tiposdeprecios` (`tipoprec`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

