/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `bitacora_pedidoautomatico_presentaciones`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `bitacora_pedidoautomatico_presentaciones` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `tipo_configuracion` char(1) NOT NULL,
  `accion` char(1) NOT NULL,
  `producto` varchar(8) NOT NULL,
  `present` varchar(13) DEFAULT NULL,
  `proveedor_anterior` varchar(11) DEFAULT NULL,
  `proveedor_nuevo` varchar(11) DEFAULT NULL,
  `duracion_reorden_anterior` tinyint(3) unsigned DEFAULT NULL,
  `duracion_reorden_nueva` tinyint(3) unsigned DEFAULT NULL,
  `multiplo_pedir_anterior` varchar(9) DEFAULT NULL,
  `multiplo_pedir_nuevo` varchar(9) DEFAULT NULL,
  `fecha_modificacion` datetime NOT NULL DEFAULT current_timestamp(),
  `usuario` varchar(10) DEFAULT NULL,
  `sucursal` varchar(2) DEFAULT NULL,
  `empresa` int(2) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `fk_usuario` (`usuario`),
  CONSTRAINT `fk_usuario` FOREIGN KEY (`usuario`) REFERENCES `usuarios` (`empleado`),
  CONSTRAINT `CONSTRAINT_1` CHECK (`tipo_configuracion` in ('G','E','S')),
  CONSTRAINT `CONSTRAINT_2` CHECK (`accion` in ('A','U','D'))
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

