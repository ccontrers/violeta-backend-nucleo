/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
DROP TABLE IF EXISTS `dettrnxventa`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `dettrnxventa` (
  `dcs_form` varchar(10) NOT NULL DEFAULT '',
  `trn_amount` decimal(16,2) NOT NULL DEFAULT 0.00,
  `trn_qty_pay` int(11) NOT NULL DEFAULT 0,
  `trn_orig_id` bigint(20) DEFAULT 0,
  `trn_id` bigint(20) NOT NULL,
  `trn_auth_code` varchar(6) NOT NULL DEFAULT '',
  `trn_internal_respcode` smallint(6) NOT NULL DEFAULT 0,
  `mer_legend1` varchar(80) DEFAULT '',
  `mer_legend2` varchar(80) DEFAULT '',
  `mer_legend3` varchar(80) DEFAULT '',
  `trn_internal_mer_id` varchar(16) NOT NULL DEFAULT '',
  `trn_internal_ter_id` varchar(16) NOT NULL DEFAULT '',
  `trn_host_date` varchar(10) NOT NULL DEFAULT '',
  `trn_host_hour` varchar(8) NOT NULL DEFAULT '',
  `trn_type_id` bigint(20) NOT NULL DEFAULT 0,
  `trn_subtype_id` bigint(20) NOT NULL DEFAULT 0,
  `trn_aco_id` varchar(20) NOT NULL DEFAULT '',
  `trn_cur_id1` int(11) NOT NULL DEFAULT 0,
  `trn_external_mer_id` varchar(16) DEFAULT '',
  `trn_external_ter_id` varchar(16) NOT NULL DEFAULT '',
  `trn_pro_name` varchar(80) NOT NULL DEFAULT '',
  `trn_pre_type` int(11) NOT NULL DEFAULT 0 COMMENT '1 - tarjeta de credito, 2 - tarjeta de debito',
  `trn_input_mode` int(11) NOT NULL DEFAULT 0,
  `trn_label` varchar(50) NOT NULL DEFAULT '',
  `trn_aprnam` varchar(50) NOT NULL DEFAULT '',
  `trn_emv_cryptogram` varchar(50) NOT NULL DEFAULT '',
  `trn_fe` varchar(5) NOT NULL DEFAULT '',
  `trn_key` varchar(12) DEFAULT '',
  `trn_msg_host` varchar(150) NOT NULL DEFAULT '',
  `trn_reauths` varchar(50) NOT NULL DEFAULT '',
  `trn_pro_id` varchar(255) NOT NULL DEFAULT '0',
  `trn_tip_amount` decimal(16,2) DEFAULT 0.00,
  `trn_cashback_amount` decimal(16,2) NOT NULL DEFAULT 0.00,
  `trn_bat_number_external` varchar(150) NOT NULL DEFAULT '',
  `trn_cardholder_name` varchar(250) NOT NULL DEFAULT '',
  `trn_fechaTrans` varchar(50) NOT NULL DEFAULT '',
  `trn_AID` varchar(250) NOT NULL DEFAULT '',
  `trn_assembly` varchar(250) NOT NULL DEFAULT '',
  `trn_relacionada` bigint(20) DEFAULT 0,
  `trn_referencia_1` varchar(11) DEFAULT '',
  `trn_fii` varchar(50) NOT NULL DEFAULT '',
  `terminal` varchar(10) NOT NULL DEFAULT '',
  `sucursal` varchar(2) NOT NULL DEFAULT '',
  `fecha_trn` date NOT NULL DEFAULT curdate(),
  `hora_trn` time NOT NULL DEFAULT curtime(),
  `usu_trn` varchar(10) DEFAULT '0',
  PRIMARY KEY (`trn_id`,`trn_auth_code`),
  KEY `dcs_form` (`dcs_form`),
  KEY `sucursal_dettrnxventa_fk2` (`sucursal`),
  KEY `terminal_dettrnxventa_fk1` (`terminal`),
  KEY `usu_trn_dettrnxventa_fk3` (`usu_trn`),
  CONSTRAINT `dcs_form_dettrnxventa_fk4` FOREIGN KEY (`dcs_form`) REFERENCES `tipostransacciones` (`dcs_form`) ON UPDATE CASCADE,
  CONSTRAINT `sucursal_dettrnxventa_fk2` FOREIGN KEY (`sucursal`) REFERENCES `sucursales` (`sucursal`) ON UPDATE CASCADE,
  CONSTRAINT `terminal_dettrnxventa_fk1` FOREIGN KEY (`terminal`) REFERENCES `terminales` (`terminal`) ON UPDATE CASCADE,
  CONSTRAINT `usu_trn_dettrnxventa_fk3` FOREIGN KEY (`usu_trn`) REFERENCES `empleados` (`empleado`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

