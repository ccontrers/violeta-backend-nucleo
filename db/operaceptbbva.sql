/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `operaceptbbva`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `operaceptbbva` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `criptogramaTarjeta` varchar(128) DEFAULT NULL,
  `emisorTarjeta` varchar(5) DEFAULT NULL,
  `fechaHora` varchar(16) DEFAULT NULL,
  `aplicacionTarjeta` varchar(50) DEFAULT NULL,
  `nombreTransaccion` varchar(50) DEFAULT NULL,
  `codigoOperacion` varchar(50) DEFAULT NULL,
  `razonSocial` varchar(50) DEFAULT NULL,
  `tarjetahabiente` varchar(255) DEFAULT NULL,
  `leyenda` varchar(128) DEFAULT NULL,
  `moneda` varchar(20) DEFAULT NULL,
  `productoTarjeta` varchar(5) DEFAULT NULL,
  `afiliacion` varchar(7) DEFAULT NULL,
  `autorizacion` varchar(128) DEFAULT NULL,
  `secuenciaTransaccion` varchar(128) DEFAULT NULL,
  `referenciaFin` varchar(20) DEFAULT NULL,
  `terminal` varchar(10) DEFAULT NULL,
  `direccion` varchar(255) DEFAULT NULL,
  `firma` varchar(50) DEFAULT NULL,
  `numeroTarjeta` varchar(20) DEFAULT NULL,
  `importe` varchar(20) DEFAULT NULL,
  `macTerminal` varchar(17) DEFAULT NULL,
  `operadorclave` varchar(50) DEFAULT NULL,
  `operador` varchar(6) DEFAULT NULL,
  `codigoRespuesta` varchar(2) DEFAULT NULL,
  `referencia` varchar(20) DEFAULT NULL,
  `cash` varchar(10) NOT NULL DEFAULT '',
  `puntos` varchar(10) DEFAULT NULL,
  `ant_pesos` varchar(10) NOT NULL DEFAULT '',
  `ant_puntos` varchar(10) NOT NULL DEFAULT '',
  `disp_pesos` varchar(10) NOT NULL DEFAULT '',
  `disp_puntos` varchar(10) NOT NULL DEFAULT '',
  `puntos_redim` varchar(10) NOT NULL DEFAULT '',
  `modoLectura` varchar(2) DEFAULT NULL COMMENT '05 - tarjeta insertada (chip), 01 - tarjeta digitada, 90 tarjeta deslizada (banda), 80 por error en chip (full back)',
  `idAplicacionTarjeta` varchar(128) DEFAULT NULL,
  `fecha` date NOT NULL DEFAULT curdate(),
  `hora` time NOT NULL DEFAULT curtime(),
  `terminalsist` varchar(10) NOT NULL DEFAULT '',
  `sucursal` varchar(2) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`) USING BTREE,
  KEY `sucursal_bbva_aprob_fk1` (`sucursal`) USING BTREE,
  KEY `terminal_bbva_aprob_fk1` (`terminalsist`) USING BTREE,
  KEY `referencia_financiera` (`referenciaFin`) USING BTREE,
  KEY `referencia_comercio` (`referencia`) USING BTREE,
  CONSTRAINT `sucursal_bbva_aprob_fk1` FOREIGN KEY (`sucursal`) REFERENCES `sucursales` (`sucursal`) ON UPDATE CASCADE,
  CONSTRAINT `terminal_bbva_aprob_fk1` FOREIGN KEY (`terminalsist`) REFERENCES `terminales` (`terminal`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

