//---------------------------------------------------------------------------

#ifndef ClassServidorVentasH
#define ClassServidorVentasH
//---------------------------------------------------------------------------


#include "ClassDatosTabla.h"
#include "ClassFuncionesGenericas.h"
#include "ClassRespuestasServidor.h"
#include "ClassBufferRespuestas.h"



#include <mysql.h>

class ServidorVioleta;
//---------------------------------------------------------------------------
/** Clase destinada a ser la parte del servidor que se encarga de despachar
* las peticiones relacionadas con las VENTAS
*/
class ServidorVentas {
	private:
		ServidorVioleta *mServidorVioleta; /**< Puntero al objeto servidor principal*/
		FuncionesGenericas mFg; /**< Objeto con funciones genéricas */
		AnsiString	mFechaReporteGlobal; /**< ID_GRA_FACTURAGLOBALCFD_33 > */

	public:
        RespuestasServidor *Respuestas; /**< Puntero a un objeto manejador de respuestas*/
		/** Constructor, lo único que hace es unir este objeto con el servidor principal.
		*/
		ServidorVentas(ServidorVioleta *Servidor) {mServidorVioleta=Servidor;};

		/* --
		* @param Respuesta Puntero al objeto donde se va a poner el resultado.
		* @param MySQL Puntero a la conexión MySql que se debe usar para esta petición.
		* @param parametros Puntero al buffer de parámetros.
		*/

        void DeshaceCancelacionVenta(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void RevisaExistenciasVenta(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaVenta(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void CancelaVenta(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaVenta(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void CancelaCfd(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaKit(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void CancelaKit(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaKit(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaPedidoCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void CancelaPedidoCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaPedidoCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaDevolCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros, bool factura_web=false, bool pendientegenerar = false
				, bool generardeticket = false);

		void GrabaCfdiXmlWeb(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaNotadeCreditoPorTicket(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void CancelaDevolCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaDevolCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaPagoCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaPagoCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void CancelaPagoCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ModificaFechaPagoCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaPagosCliDelDia(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaAuxiliarCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ConsolidaChequesCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaChequesxfechaCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaChequesxfechaCliNC(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ModificaChequeCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ConsultaVentasCliente(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaVentasClienteParaNCredito(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaSaldoCliente(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaClienteParaVenta(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaCargoCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void CancelaCargoCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaCargoCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaCargoReboteCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void CancelaCargoReboteCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaCargoReboteCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ConsultaPedidoParaImprimir(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaVentaParaImprimir(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaVentaParaImprimirSurt(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaPagoCliParaImprimir(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaNCredCliParaImprimir(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaNCarCliParaImprimir(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaColaImpresion(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaColaImpresionSurt(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ModificaColaImpresion(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ModificaColaImpresionSurt(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaComisionesVendedor(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaCobranzaCobrador(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void CambiaCobradorVenta(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void CambiaStatusVenta(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void AsignaFolioFisicoVenta(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ModificaSigFolioFisico(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaVentaConcentradaParaImprimir(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ConsultaCartaPorteParaImprimir(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaFacturaGlobalCfd(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ConsultaPagosProveedor(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaCartaPorte(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void CancelaCartaPorte(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaCartaPorte(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ReintentaTimbrado(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaSimilarVxvol(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ConsultaPedidoCliTotal(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaFacturaGlobalCfd33(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ExportaPedidosCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);


		void GrabaDireccionEntrega(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaDireccionEntrega(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ActualizaEmbarqueCliente(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ServidorVentas::ModificaFechaNota(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ServidorVentas::BitacoraCONTPAQ(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ObtenerCantAcumulada(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ActualizarRetiroEfectivo(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaConsultaCorteCaja(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaGuardaCorteCaja(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaCerrarCorteCaja(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void VerificaCortesAbiertos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaCerrarCorteCajaPendientes(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GuardaTranVenta(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GuardaBitacoraTranVenta(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaTranVenta(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GeneraFolioTransaccion(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void GrabaFacturaWeb(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaCartaPorte_V20(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void CancelaCartaPorte_V20(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaCartaPorte_V20(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void RevisaExistenciasRemotasCalculadorPedidos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ConsultaPrePagoCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ConsultaTrnVenta(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ConsultaPdfFactura(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ReintentaTimbradoRegXML(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GuardaenBitacoraWeb(RespuestaServidor *Respuesta, MYSQL *MySQL,
			AnsiString clave_bitacora, AnsiString mensaje_bitacora,AnsiString detalles_bitacora, AnsiString msg);

		void GeneraPagosProveedorTxt(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ConsultaNCDetalle(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void CambiaMotEmpNC(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ConsultaSurtidos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GuardaBitacoraVentaPinPadBBVA(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void WSIntermediarioAPIKey(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
};

#endif
