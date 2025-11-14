//---------------------------------------------------------------------------

#ifndef ClassServidorReportesH
#define ClassServidorReportesH
//---------------------------------------------------------------------------

#include "ClassDatosTabla.h"
#include "ClassFuncionesGenericas.h"
#include "ClassRespuestasServidor.h"



#include <mysql.h>

class ServidorVioleta;
//---------------------------------------------------------------------------
/** Clase destinada a ser la parte del servidor que se encarga de despachar
* las peticiones relacionadas con LOS REPORTES
*/
class ServidorReportes {
    private:
        ServidorVioleta *mServidorVioleta; /**< Puntero al objeto servidor principal*/
		FuncionesGenericas mFg; /**< Objeto con funciones genéricas */

    public:
        /** Constructor, lo único que hace es unir este objeto con el servidor principal.
        */
        ServidorReportes(ServidorVioleta *Servidor) {mServidorVioleta=Servidor;};

        /* --
        * @param Respuesta Puntero al objeto donde se va a poner el resultado.
        * @param MySQL Puntero a la conexión MySql que se debe usar para esta petición.
        * @param parametros Puntero al buffer de parámetros.
        */
		void EjecutaRepVentasXFactura(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaRepVentasXCotizacion(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void EjecutaRepVentasXFacturaRedondeada(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void EjecutaRepComprasXFactura(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void EjecutaRepNotcarcliXNota(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void EjecutaRepNotcarprovXNota(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void EjecutaRepNotcredcliXNota(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void EjecutaRepNotcredcliXNotaRedondeada(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void EjecutaRepNotcredprovXNota(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void EjecutaRepVentasXArticulo(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void EjecutaRepComprasXArticulo(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void EjecutaRepVentasXCliente(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaRepComprasXProveedor(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepCartasPorte(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepListaPrecios(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaRepSeguimientoMovsArticulo(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaRepSeguimientoMovsArtOpt(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void EjecutaRepExistenciasSimple(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void EjecutaRepPagosClientes(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void EjecutaRepPagosProveedores(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void EjecutaRepNotcredcliXArticulo(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void EjecutaRepNotcredprovXArticulo(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void EjecutaRepMovAlmaXMov(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void EjecutaRepMovAlmaXArticulo(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void EjecutaRepSaldosClientes(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void EjecutaRepSaldosProveedores(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void EjecutaRepCostoExistencias(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void EjecutaRepCostoVentas(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void EjecutaRepCostoVentasExt(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaRepGeneralClientes(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaRepGeneralProveedores(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void EjecutaRepInventarioCapturado(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void EjecutaRepCompInventarioCapturado(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);        
        void EjecutaRepCostoInventarioCapturado(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaRepCostoMovimientosAlmacen(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepEtiquetasAnaqueles(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepComparaPedVent(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaReporteEmbarques(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void EjecutaReporteEmbarquesDesglosado(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaEnvioCorreo(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepDifCostoXArticulos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepRotacionInventario(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepCalcularPedidos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepGlobalIEPS(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepCostoInvcapComparativo(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaReportUtilidades(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaReportDetalleInventario(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ConsultaReporteModificacionAlm(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaEnvioCorreoCondicionComercial(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepArticulosCoincidencias(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void EjecutaRepExistenciasSucursal(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaReporteCondicionComercial(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaReporteDocumentosModificados(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaReporteVariacionPrecios(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepCostoVentasAgrupadoxCliente(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepVentasClientePorDia(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepTraspasosArticulosMensual(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepVentasArticulosMensual(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepVentasArticulosDesglosado(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepActualizaPreciosDiferidos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepClasificacionEnPlantillas(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepCostoDetExt(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepCalcularPedidosOpt(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepProvCompras(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepDetProvCompras(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepCostoMovExt(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepCamProvPrin(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepPolizaCobranza(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepExistenciasSimpleOpt(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepCodPosXSuc(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepCostoDetalleMovAlma(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepPolizasMasivas(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaEnvioCorreoPedidos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepRetiroEfectivoSupers(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepArtGeneral(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepArtTag(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepTendVtaCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void EjecutaRepTendVta(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepTransaccionesBilleto(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaRepBitacoraTransaccionesBilleto(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepArtEliminadosEmbarques(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void EjecutaRepCortesCaja(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepArtSupervisados(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ConsultaReporteModFechaVenc(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepPagosGastos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaRepSaldosGastos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaRepNotcredGastosXNota(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutarRegModCFDICom(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void EjecutaRepVentasKits(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaRepArtSinExistPedRemoto(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void EjecutaBitaConfigPinpad(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaBitaTrnxVentas(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaTrnxVentas(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaInfoReimpreTrn(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaRepCartPortXEmbarque(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaRepCartaPorte2(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjeRepVtaFacturaGlob(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjeBitacoraCompras(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjeBitacoraAppRutas(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjeBitacoraAppInventarios(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepCostoVentasMix(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepPolizaGastos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepCFD(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepPrepago(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaRepBitacoraPrepago(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaEnvioCorreoPrepagos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepErroresTimbrado(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);


		void EjecutaRepResumenCorteCaja(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaRepTicketCorteCaja(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		
		void EjecutaRepPCAD(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaRepNCPD(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaRepVCAD(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaRepPedidosXProvDoc(RespuestaServidor *Respuesta,MYSQL *MySQL, char *parametros);

		void EjecutaBitacoraModClaveArt(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaRepBitacoraServidor(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepDifPeso(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaRepDifVolumen(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaGrabaListaProductosCotizar(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaRepCPCA(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaRepSALVENT(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaRepTransXcob(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaRepRecepciones(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepSurtidoAnaqueles(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepDevRecepciones(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void bitacoraEmpleados(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void historialEmpleados(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepSolNotCredCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaRepDetSolNotCredCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void UpdtSolNotCredCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void UpdtSolNotCredCliMasiva(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaBitacoraSolicitudesNotasCredCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaBitacoraVentasTPV(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaReporteEmpleados(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaBitacoraSucursal(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepCorteCostExis(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaRepUbicacionXProducto(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ConsultarBitacoraCambiosProgramados(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaRepObjetivosAgentesVent(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaRepObjetivosAgentesCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaRepObjetivosSucursalesVent(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ServidorReportes::RepRegistrosCompras(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepFlujoEfectivo(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepSolicitudTicketsSoporte(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaBitSolicitudTicketsSoporte(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepSurtidoAud(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaDetFlujoEfectivo(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaReporteClasifProdSuc(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ConsultaRepRecepcionesxEstatus(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaBitaRecepCapturaProductos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaTrnPinpad(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaBitacoraTrnPinpad(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaConcentrTrnPinpad(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaBitacoraSurtidoCapturaProductos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void EjecutaBitacoraConfiguracionPreciosArticulos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
};

#endif

