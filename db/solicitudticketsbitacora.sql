/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `solicitudticketsbitacora`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `solicitudticketsbitacora` (
  `referencia` varchar(11) NOT NULL DEFAULT '0',
  `idJefatura` varchar(4) DEFAULT NULL,
  `areasolicita` varchar(50) DEFAULT NULL,
  `ususolicita` varchar(10) DEFAULT NULL,
  `usuvalida` varchar(10) DEFAULT NULL,
  `usumodi` varchar(10) DEFAULT NULL,
  `terminal` varchar(10) DEFAULT NULL,
  `fechasol` date NOT NULL DEFAULT '0000-00-00',
  `horasol` time DEFAULT NULL,
  `fechapromesa` date NOT NULL DEFAULT '0000-00-00',
  `fechaini` date NOT NULL DEFAULT '0000-00-00',
  `horaini` time DEFAULT NULL,
  `fechasolucion` date NOT NULL DEFAULT '0000-00-00',
  `fechamodi` date NOT NULL DEFAULT '0000-00-00',
  `horamodi` time DEFAULT NULL,
  `fechavalidado` date NOT NULL DEFAULT '0000-00-00',
  `horavalidado` time DEFAULT NULL,
  `prioridad` tinyint(1) NOT NULL DEFAULT 1 COMMENT '0 - Programada 1 - Urgente',
  `responsable` varchar(10) DEFAULT NULL,
  `solucionado` tinyint(1) NOT NULL DEFAULT 0 COMMENT '0 - No 1 - Si',
  `validado` tinyint(1) NOT NULL DEFAULT 0 COMMENT '0 - No 1 - Si',
  `departamento` varchar(4) NOT NULL DEFAULT '0',
  `cantabierto` int(3) NOT NULL DEFAULT 1,
  `tipo` varchar(3) DEFAULT NULL COMMENT 'A - ALTA M - MODIFICADO',
  KEY `referencia` (`referencia`) USING BTREE,
  KEY `ususolicita` (`ususolicita`) USING BTREE,
  KEY `usuvalida` (`usuvalida`) USING BTREE,
  KEY `usumodi` (`usumodi`) USING BTREE,
  KEY `areasolicita` (`areasolicita`) USING BTREE,
  KEY `terminal` (`terminal`) USING BTREE,
  KEY `responsable` (`responsable`) USING BTREE,
  KEY `departamento` (`departamento`) USING BTREE,
  KEY `solicitudticketsbitacora_ibfk_7` (`idJefatura`),
  CONSTRAINT `solicitudticketsbitacora_ibfk_1` FOREIGN KEY (`ususolicita`) REFERENCES `empleados` (`empleado`) ON UPDATE CASCADE,
  CONSTRAINT `solicitudticketsbitacora_ibfk_2` FOREIGN KEY (`usuvalida`) REFERENCES `empleados` (`empleado`) ON UPDATE CASCADE,
  CONSTRAINT `solicitudticketsbitacora_ibfk_3` FOREIGN KEY (`terminal`) REFERENCES `terminales` (`terminal`) ON UPDATE CASCADE,
  CONSTRAINT `solicitudticketsbitacora_ibfk_4` FOREIGN KEY (`responsable`) REFERENCES `empleados` (`empleado`) ON UPDATE CASCADE,
  CONSTRAINT `solicitudticketsbitacora_ibfk_5` FOREIGN KEY (`usumodi`) REFERENCES `empleados` (`empleado`) ON UPDATE CASCADE,
  CONSTRAINT `solicitudticketsbitacora_ibfk_6` FOREIGN KEY (`departamento`) REFERENCES `departamentossoporte` (`departamento`) ON UPDATE CASCADE,
  CONSTRAINT `solicitudticketsbitacora_ibfk_7` FOREIGN KEY (`idJefatura`) REFERENCES `catalogojefaturasordenestrabajo` (`clave`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

