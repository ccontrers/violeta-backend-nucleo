/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `bitacora_articulos_oferta`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `bitacora_articulos_oferta` (
  `id_bit_articulos_oferta` int(11) NOT NULL AUTO_INCREMENT,
  `folio_oferta` varchar(9) DEFAULT NULL,
  `articulo` varchar(9) DEFAULT NULL,
  `idempresa` int(2) DEFAULT NULL,
  `precio_oferta` decimal(13,6) DEFAULT NULL,
  `usuario_alta` varchar(10) DEFAULT NULL,
  `usuario_modi` varchar(10) DEFAULT NULL,
  `fecha_creacion` date DEFAULT NULL,
  `hora_creacion` time DEFAULT NULL,
  `fecha_modificacion` date DEFAULT NULL,
  `hora_modificacion` time DEFAULT NULL,
  `fecha_vigencia_inicio` date DEFAULT NULL,
  `fecha_vigencia_fin` date DEFAULT NULL,
  `activo` tinyint(1) DEFAULT NULL,
  PRIMARY KEY (`id_bit_articulos_oferta`) USING BTREE,
  KEY `bit_articulos_oferta_5` (`folio_oferta`,`articulo`) USING BTREE,
  KEY `usuario` (`usuario_alta`) USING BTREE,
  KEY `bit_articulos_oferta_ibfk_3` (`idempresa`) USING BTREE,
  KEY `bit_articulos_oferta_ibfk_1` (`articulo`) USING BTREE,
  KEY `bit_articulos_oferta_ibfk_5` (`usuario_modi`),
  CONSTRAINT `bit_articulos_oferta_ibfk_1` FOREIGN KEY (`articulo`) REFERENCES `articulos` (`articulo`) ON DELETE SET NULL ON UPDATE CASCADE,
  CONSTRAINT `bit_articulos_oferta_ibfk_2` FOREIGN KEY (`usuario_alta`) REFERENCES `usuarios` (`empleado`) ON DELETE SET NULL ON UPDATE CASCADE,
  CONSTRAINT `bit_articulos_oferta_ibfk_3` FOREIGN KEY (`idempresa`) REFERENCES `empresas` (`idempresa`) ON DELETE SET NULL ON UPDATE CASCADE,
  CONSTRAINT `bit_articulos_oferta_ibfk_5` FOREIGN KEY (`usuario_modi`) REFERENCES `usuarios` (`empleado`) ON DELETE SET NULL ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

