/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `mensajes`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `mensajes` (
  `mensaje` varchar(11) NOT NULL DEFAULT '',
  `remitente` varchar(10) DEFAULT NULL,
  `destino` varchar(10) DEFAULT NULL,
  `fechaenvio` date NOT NULL DEFAULT '0000-00-00',
  `horaenvio` time DEFAULT NULL,
  `fechalect` date NOT NULL DEFAULT '0000-00-00',
  `horalect` time DEFAULT NULL,
  `urgente` tinyint(1) DEFAULT NULL,
  `directorio` varchar(10) DEFAULT NULL,
  `leido` tinyint(1) DEFAULT NULL,
  `recibido` tinyint(1) DEFAULT NULL,
  `destmovil` tinyint(1) DEFAULT NULL,
  `enviadomovil` tinyint(1) DEFAULT NULL,
  `contenido` text DEFAULT NULL,
  `grupo` varchar(11) DEFAULT NULL,
  `asunto` varchar(80) DEFAULT NULL,
  PRIMARY KEY (`mensaje`),
  KEY `remitente` (`remitente`),
  KEY `destino` (`destino`),
  KEY `fechaenvio` (`fechaenvio`),
  KEY `directorio` (`directorio`),
  CONSTRAINT `mensajes_ibfk_1` FOREIGN KEY (`remitente`) REFERENCES `usuarios` (`empleado`) ON UPDATE CASCADE,
  CONSTRAINT `mensajes_ibfk_2` FOREIGN KEY (`destino`) REFERENCES `usuarios` (`empleado`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

