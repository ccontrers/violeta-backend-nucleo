/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `bancosmov`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `bancosmov` (
  `idmovbanco` int(11) NOT NULL AUTO_INCREMENT,
  `idnumcuenta` int(11) NOT NULL,
  `conceptomov` varchar(1) NOT NULL,
  `afectacion` varchar(1) DEFAULT NULL,
  `descripcion` varchar(50) DEFAULT NULL,
  `identificador` varchar(20) NOT NULL,
  `cancelado` tinyint(1) NOT NULL,
  `subtotal` decimal(16,2) NOT NULL,
  `ivabanco` decimal(16,2) NOT NULL,
  `total` decimal(16,2) NOT NULL,
  `fechaaplbanco` date NOT NULL,
  `fechaalta` date NOT NULL,
  `horaalta` time NOT NULL,
  `fechamodi` date NOT NULL,
  `horamodi` time NOT NULL,
  `usualta` varchar(10) NOT NULL,
  `usumodi` varchar(10) NOT NULL,
  `terminal` varchar(10) NOT NULL,
  `origen` varchar(5) NOT NULL,
  `sucursal` varchar(2) NOT NULL,
  `aplicado` tinyint(1) NOT NULL DEFAULT 1,
  PRIMARY KEY (`idmovbanco`),
  KEY `idnumcuenta` (`idnumcuenta`),
  KEY `usualta` (`usualta`),
  KEY `usumodi` (`usumodi`),
  KEY `identificador` (`identificador`),
  KEY `fechaalta` (`fechaalta`),
  KEY `fechamodi` (`fechamodi`),
  KEY `fechaaplbanco` (`fechaaplbanco`),
  KEY `origen` (`origen`),
  KEY `bancosmov_ibfk_3` (`conceptomov`),
  CONSTRAINT `bancosmov_ibfk_1` FOREIGN KEY (`idnumcuenta`) REFERENCES `bancoscuentas` (`idnumcuenta`) ON UPDATE CASCADE,
  CONSTRAINT `bancosmov_ibfk_2` FOREIGN KEY (`origen`) REFERENCES `bancosorigenes` (`origen`) ON UPDATE CASCADE,
  CONSTRAINT `bancosmov_ibfk_3` FOREIGN KEY (`conceptomov`) REFERENCES `bancosconceptomov` (`conceptomov`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

