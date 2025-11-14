/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `bancoscuentas`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `bancoscuentas` (
  `idnumcuenta` int(11) NOT NULL AUTO_INCREMENT,
  `numerocuenta` varchar(50) DEFAULT NULL,
  `banco` varchar(4) DEFAULT NULL,
  `naturaleza` varchar(5) DEFAULT NULL,
  `descripcion` varchar(150) DEFAULT NULL,
  `saldoinicial` decimal(16,4) DEFAULT NULL,
  `activoapp` tinyint(1) NOT NULL DEFAULT 0,
  `principal` tinyint(1) DEFAULT 0,
  `mostrarenfactura` tinyint(1) DEFAULT 0,
  `clabe` varchar(50) DEFAULT NULL,
  `idempresa` int(2) DEFAULT NULL,
  PRIMARY KEY (`idnumcuenta`),
  UNIQUE KEY `numerocuenta` (`numerocuenta`,`banco`),
  KEY `bancoscuentas_ibfk` (`banco`),
  KEY `idnumcuenta` (`idnumcuenta`,`numerocuenta`),
  KEY `bancoscuentas_ibfk2` (`naturaleza`),
  KEY `bancoscuentasempresa_ibfk_1` (`idempresa`),
  CONSTRAINT `bancoscuentas_ibfk` FOREIGN KEY (`banco`) REFERENCES `bancos` (`banco`) ON UPDATE CASCADE,
  CONSTRAINT `bancoscuentas_ibfk2` FOREIGN KEY (`naturaleza`) REFERENCES `bancosnaturalezas` (`naturaleza`) ON UPDATE CASCADE,
  CONSTRAINT `bancoscuentasempresa_ibfk_1` FOREIGN KEY (`idempresa`) REFERENCES `empresas` (`idempresa`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

