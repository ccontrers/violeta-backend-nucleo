/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `proveedoressupers`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `proveedoressupers` (
  `proveedor` varchar(11) NOT NULL DEFAULT '',
  `razonsocial` varchar(60) NOT NULL DEFAULT '',
  `tipoempre` varchar(1) DEFAULT NULL,
  `replegal` varchar(60) DEFAULT NULL,
  `titrepleg` varchar(10) DEFAULT NULL,
  `fechnrep` date NOT NULL DEFAULT '0000-00-00',
  `rfc` varchar(15) DEFAULT NULL,
  `curp` varchar(18) DEFAULT NULL,
  `calle` varchar(60) DEFAULT NULL,
  `colonia` varchar(40) DEFAULT NULL,
  `cvecolonia` varchar(10) DEFAULT NULL,
  `cp` varchar(10) DEFAULT NULL,
  `localidad` varchar(40) DEFAULT NULL,
  `estado` varchar(4) DEFAULT NULL,
  `pais` varchar(40) DEFAULT NULL,
  `fechaalta` date NOT NULL DEFAULT '2000-01-01',
  `contacto` varchar(60) DEFAULT NULL,
  `emailcto` varchar(50) DEFAULT NULL,
  `fechncon` date NOT NULL DEFAULT '0000-00-00',
  `email` varchar(50) DEFAULT NULL,
  `limcred` decimal(16,2) DEFAULT NULL,
  `plazo` int(4) DEFAULT NULL,
  `descuento` decimal(5,2) DEFAULT NULL,
  `descppp` decimal(5,2) DEFAULT NULL,
  `bancoc1` varchar(10) DEFAULT NULL,
  `bancoc2` varchar(10) DEFAULT NULL,
  `bancoc3` varchar(10) DEFAULT NULL,
  `cuentab1` varchar(20) DEFAULT NULL,
  `cuentab2` varchar(20) DEFAULT NULL,
  `cuentab3` varchar(20) DEFAULT NULL,
  `tipocuenta1` varchar(5) DEFAULT NULL,
  `tipocuenta2` varchar(5) DEFAULT NULL,
  `tipocuenta3` varchar(5) DEFAULT NULL,
  `cuentadefault` int(3) DEFAULT NULL,
  `apoyos` varchar(60) DEFAULT NULL,
  `fechauap` date NOT NULL DEFAULT '0000-00-00',
  `credito` tinyint(1) DEFAULT NULL,
  `reduccostobase` tinyint(4) NOT NULL DEFAULT 0,
  `porcreduccosto` decimal(5,2) NOT NULL DEFAULT 0.00,
  `esparterelac` tinyint(1) NOT NULL DEFAULT 0,
  `cuadreestcomp` tinyint(1) NOT NULL DEFAULT 1,
  `cuadreestncre` tinyint(1) NOT NULL DEFAULT 1,
  `cuadreestpagos` tinyint(1) NOT NULL DEFAULT 1,
  `cuadreestncar` tinyint(1) NOT NULL DEFAULT 1,
  `activo` tinyint(1) NOT NULL DEFAULT 1 COMMENT 'cero incactivo, uno activo',
  `redondeocptecho` tinyint(1) DEFAULT 1,
  `emitencpago` tinyint(1) NOT NULL DEFAULT 1,
  `comprador` varchar(10) DEFAULT NULL,
  `provgastos` tinyint(4) DEFAULT 0,
  `provmercancia` tinyint(4) DEFAULT 1,
  `esresico` tinyint(4) DEFAULT 0,
  `impuestoret` int(11) DEFAULT NULL,
  `numcuenta` varchar(30) DEFAULT NULL,
  `fechamodi` date NOT NULL DEFAULT '2000-01-01',
  `usumodi` varchar(10) DEFAULT NULL,
  `usualta` varchar(10) DEFAULT NULL,
  `agrupapagfact` tinyint(4) DEFAULT 0,
  `agrupapaggast` tinyint(4) DEFAULT 0,
  `tiporefpago` tinyint(4) DEFAULT NULL,
  `referenciafija` varchar(18) DEFAULT NULL,
  `diasvigencia` int(10) DEFAULT NULL,
  `tiporetencion` varchar(15) DEFAULT 'No configurada',
  PRIMARY KEY (`proveedor`) USING BTREE,
  KEY `replegal` (`replegal`) USING BTREE,
  KEY `razonsocial` (`razonsocial`) USING BTREE,
  KEY `rfc` (`rfc`) USING BTREE,
  KEY `proveedores_ibfk_1` (`estado`) USING BTREE,
  KEY `proveedores_ibfk_2` (`comprador`) USING BTREE,
  KEY `proveedores_ibfk_3` (`cvecolonia`) USING BTREE,
  KEY `proveedores_ibfk_4` (`impuestoret`) USING BTREE,
  KEY `proveedores_ibfk_usumodi` (`usumodi`) USING BTREE,
  KEY `proveedores_ibfk_usualta` (`usualta`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

