/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `precios`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `precios` (
  `articulo` varchar(9) NOT NULL DEFAULT '0',
  `tipoprec` varchar(2) NOT NULL DEFAULT '',
  `costo` decimal(16,6) DEFAULT NULL,
  `precio` decimal(16,6) DEFAULT NULL,
  `porcutil` decimal(6,2) DEFAULT NULL,
  `fechamodi` date NOT NULL DEFAULT '0000-00-00',
  `preciofechmod` decimal(16,6) DEFAULT NULL,
  `precioproximo` decimal(16,6) DEFAULT NULL,
  `preciomod` decimal(16,6) DEFAULT NULL,
  `horamodi` time NOT NULL DEFAULT '00:00:00',
  `fechamodiprox` date NOT NULL DEFAULT '0000-00-00',
  `horamodiprox` time NOT NULL DEFAULT '00:00:00',
  `actualizarpendiente` tinyint(4) NOT NULL DEFAULT 0,
  PRIMARY KEY (`articulo`,`tipoprec`),
  KEY `tipoprec` (`tipoprec`),
  CONSTRAINT `precios_ibfk_1` FOREIGN KEY (`articulo`) REFERENCES `articulos` (`articulo`) ON UPDATE CASCADE,
  CONSTRAINT `precios_ibfk_2` FOREIGN KEY (`tipoprec`) REFERENCES `tiposdeprecios` (`tipoprec`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'STRICT_TRANS_TABLES,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
/*!50003 CREATE*/ /*!50017 DEFINER=`root`@`localhost`*/ /*!50003 TRIGGER `insert_precios` BEFORE INSERT ON `precios` FOR EACH ROW BEGIN
  set new.fechamodi=CURDATE();
  set new.horamodi=CURTIME();
  set new.fechamodiprox=CURDATE();
  set new.horamodiprox=CURTIME();
  set new.preciofechmod=new.precio;
END */;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = cp850 */ ;
/*!50003 SET character_set_results = cp850 */ ;
/*!50003 SET collation_connection  = cp850_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'STRICT_TRANS_TABLES,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
/*!50003 CREATE*/ /*!50017 DEFINER=`root`@`localhost`*/ /*!50003 TRIGGER `modif_precios` BEFORE UPDATE ON `precios` FOR EACH ROW BEGIN
  DECLARE TOLERANCIA FLOAT;
  if new.precio <> old.precio then
    SET TOLERANCIA = (SELECT valor FROM parametrosglobemp WHERE parametro='TOLERCAMPRE' LIMIT 1);
    if abs(new.precio - old.preciofechmod)>TOLERANCIA then
      set new.fechamodi=CURDATE();
      set new.horamodi=CURTIME();
      set new.preciofechmod=new.precio;
    end if;
  end if;
  if new.precioproximo <> old.precioproximo then
    set new.fechamodiprox=CURDATE();
    set new.horamodiprox=CURTIME();
  end if;  
END */;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
