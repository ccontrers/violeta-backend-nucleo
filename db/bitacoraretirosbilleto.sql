/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `bitacoraretirosbilleto`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `bitacoraretirosbilleto` (
  `keyretiro` varchar(50) NOT NULL,
  `refretiro` varchar(50) NOT NULL,
  `terminal` varchar(10) NOT NULL DEFAULT '',
  `curp` varchar(20) DEFAULT NULL,
  `fecha` date NOT NULL,
  `hora` time NOT NULL,
  `nombrebenefi` varchar(50) NOT NULL DEFAULT '',
  `numeroext` varchar(10) DEFAULT NULL,
  `numeroint` varchar(10) DEFAULT '',
  `colonia` text DEFAULT NULL,
  `calle` text DEFAULT NULL,
  `telefono` varchar(15) DEFAULT NULL,
  `primernombre` varchar(60) DEFAULT NULL,
  `segundonombre` varchar(60) DEFAULT NULL,
  `apellpater` varchar(40) DEFAULT NULL,
  `apellmater` varchar(40) DEFAULT NULL,
  `fechanac` date DEFAULT NULL,
  `nacionalidad` varchar(5) DEFAULT NULL,
  `monto` double(10,2) NOT NULL,
  `montomodi` double(10,2) NOT NULL,
  `direccreq` int(11) NOT NULL,
  `curpreq` int(11) NOT NULL,
  `telreq` int(11) NOT NULL,
  `cantajustable` int(11) NOT NULL,
  `comision` double(10,2) NOT NULL,
  `location_id` varchar(50) NOT NULL,
  `usuario` varchar(10) NOT NULL,
  `estatus` varchar(1) NOT NULL COMMENT 'F - Fallado, C - Completado, I - Informacion Requerida, L- Limite excedido de depositos, A - Autorizado',
  `mensajeservidor` text DEFAULT '',
  `codigorespuesta` int(11) DEFAULT 0,
  KEY `keyretiro` (`keyretiro`),
  KEY `bitacoraretirosbilleto_terminal_fk` (`terminal`),
  KEY `bitacoraretirosbilleto_usuario_fk` (`usuario`),
  CONSTRAINT `bitacoraretirosbilleto_terminal_fk` FOREIGN KEY (`terminal`) REFERENCES `terminales` (`terminal`) ON UPDATE CASCADE,
  CONSTRAINT `bitacoraretirosbilleto_usuario_fk` FOREIGN KEY (`usuario`) REFERENCES `empleados` (`empleado`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

