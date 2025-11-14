/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `bitacoraproveedores`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `bitacoraproveedores` (
  `id_bitacoraproveedor` int(11) NOT NULL AUTO_INCREMENT,
  `usualta` varchar(10) DEFAULT NULL,
  `usumodi` varchar(10) DEFAULT NULL,
  `tipo_modificacion` varchar(20) DEFAULT NULL,
  `fechaalta` date NOT NULL DEFAULT '0000-00-00',
  `fechamodi` date NOT NULL DEFAULT '0000-00-00',
  `proveedor` varchar(11) DEFAULT NULL,
  `razonsocial` varchar(100) DEFAULT NULL,
  `replegal` varchar(60) DEFAULT NULL,
  `rfc` varchar(15) DEFAULT NULL,
  `curp` varchar(18) DEFAULT NULL,
  `tipo_empresa` varchar(1) DEFAULT NULL,
  `plazo_credito` int(4) DEFAULT NULL,
  `limite_credito` decimal(16,2) DEFAULT NULL,
  `credito` tinyint(1) DEFAULT NULL,
  `descuento` decimal(5,2) DEFAULT NULL,
  `email1` varchar(50) DEFAULT NULL,
  `email2` varchar(50) DEFAULT NULL,
  `reduccostobase` tinyint(4) NOT NULL DEFAULT 0,
  `porcreduccosto` decimal(5,2) NOT NULL DEFAULT 0.00,
  `esparterelac` tinyint(1) NOT NULL DEFAULT 0,
  `cuadreestcomp` tinyint(1) NOT NULL DEFAULT 1,
  `cuadreestncre` tinyint(1) NOT NULL DEFAULT 1,
  `cuadreestpagos` tinyint(1) NOT NULL DEFAULT 1,
  `cuadreestncar` tinyint(1) NOT NULL DEFAULT 1,
  `activo` tinyint(1) NOT NULL DEFAULT 1 COMMENT 'cero inactivo, uno activo',
  `redondeocptecho` tinyint(1) DEFAULT 1,
  `emitencpago` tinyint(1) NOT NULL DEFAULT 1,
  `comprador` varchar(10) DEFAULT NULL,
  PRIMARY KEY (`id_bitacoraproveedor`) USING BTREE,
  KEY `usualta` (`usualta`) USING BTREE,
  KEY `fechaalta` (`fechaalta`) USING BTREE,
  KEY `tipo_modificacion` (`tipo_modificacion`) USING BTREE,
  KEY `proveedor` (`proveedor`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

