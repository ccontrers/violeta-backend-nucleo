/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `bitacoraclientes`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `bitacoraclientes` (
  `id_bitacoraclientes` int(11) NOT NULL AUTO_INCREMENT,
  `empleado` varchar(10) DEFAULT NULL,
  `tipo_modificacion` varchar(20) DEFAULT NULL,
  `fecha` datetime DEFAULT NULL,
  `cliente` varchar(11) DEFAULT NULL,
  `razon_social` varchar(255) DEFAULT NULL,
  `rfc` varchar(15) DEFAULT NULL,
  `giro_negocio` varchar(4) DEFAULT NULL,
  `clasif_canal` varchar(4) DEFAULT NULL,
  `activo_ventas` tinyint(1) DEFAULT NULL,
  `superv_gerencia` tinyint(1) DEFAULT NULL,
  `parte_relacionada` tinyint(1) DEFAULT NULL,
  `tipo_empresa` varchar(1) DEFAULT NULL,
  `credito_autorizado` tinyint(1) DEFAULT NULL,
  `plazo_credito` int(4) DEFAULT NULL,
  `limite_credito` decimal(16,2) DEFAULT NULL,
  `permit_exce_limite` tinyint(1) DEFAULT NULL,
  `vent_credit_permit` varchar(10) DEFAULT NULL,
  `imp_doc_saldo_CFDI` tinyint(1) DEFAULT NULL,
  `status_cliente` varchar(2) DEFAULT NULL,
  `agente_venta` varchar(10) DEFAULT NULL,
  `agente_cobrador` varchar(11) DEFAULT NULL,
  `tipo_precio_asignado` varchar(2) DEFAULT NULL,
  `ventas_volumen` tinyint(1) DEFAULT NULL,
  `email1` varchar(50) DEFAULT NULL,
  `email2` varchar(50) DEFAULT NULL,
  `env_comprov_cfd` varchar(1) DEFAULT NULL,
  `forma_pago_def` varchar(2) DEFAULT NULL,
  `forma_pago_vent_may` varchar(2) DEFAULT NULL,
  `valor_vent_form_pago` decimal(16,2) DEFAULT NULL,
  `uso_CFDI` varchar(4) DEFAULT NULL,
  `agrupar_concep_CFDI` tinyint(1) DEFAULT NULL,
  `regimenfiscal` varchar(3) DEFAULT NULL,
  `credito_maximo` decimal(16,2) DEFAULT NULL,
  PRIMARY KEY (`id_bitacoraclientes`),
  KEY `empleado` (`empleado`),
  KEY `fecha` (`fecha`),
  KEY `tipo_modificacion` (`tipo_modificacion`),
  KEY `cliente` (`cliente`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

