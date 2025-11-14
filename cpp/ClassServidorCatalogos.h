//---------------------------------------------------------------------------
// EVITE LO MAS POSIBLE LA DEPENDENCIA A VCL, PARA QUE ESTO PUEDA
// PORTARSE FACILMENTE A OTRO SISTEMA OPERATIVO.

#ifndef ClassServidorCatalogosH
#define ClassServidorCatalogosH

#include "ClassDatosTabla.h"
#include "ClassFuncionesGenericas.h"
#include "ClassRespuestasServidor.h"
#include <mysql.h>
#include <DateUtils.hpp>

class ServidorVioleta;
//---------------------------------------------------------------------------
/** Clase destinada a ser la parte del servidor que se encarga de despachar
* las peticiones hacerca de los catálogos del sistema.
*/
class ServidorCatalogos {
    private:
        ServidorVioleta *mServidorVioleta; /**< Puntero al objeto servidor principal*/
		FuncionesGenericas mFg; /**< Objeto con funciones genéricas */

    public:
        /** Constructor, lo único que hace es unir este objeto con el servidor principal.
        */
        ServidorCatalogos(ServidorVioleta *Servidor) {mServidorVioleta=Servidor;};

        /* --
        * @param Respuesta Puntero al objeto donde se va a poner el resultado.
        * @param MySQL Puntero a la conexión MySql que se debe usar para esta petición.
        * @param parametros Puntero al buffer de parámetros.
        */
        void ConsultaParametros(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaGenerico(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros, AnsiString Tabla, AnsiString CampoClave);
		void BajaGenerico(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros, AnsiString Tabla, AnsiString CampoClave);
        void ConsultaGenerico(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros, AnsiString Tabla, AnsiString CampoClave);

		void GrabaCliente(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void GrabaRutaEmbarque(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BajaCliente(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ConsultaCliente(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ConsultaStockProducto(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaStockProducto(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaEmpleado(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaEmpleado(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ConsultaEmpleado(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void GrabaAlmacen(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BajaAlmacen(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ConsultaAlmacen(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void GrabaProveedor(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BajaProveedor(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ConsultaProveedor(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void GrabaSucursal(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BajaSucursal(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ConsultaSucursal(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaTransportistas(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaTransportistas(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaTransportistas(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void GrabaBanco(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BajaBanco(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaBanco(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaBancoNat(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaBancoNat(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaBancoNat(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaBancoCta(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaBancoCta(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaBancoCta(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaBancoOrigen(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaBancoOrigen(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaBancoOrigen(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaConceptosEmbarque(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaConceptosEmbarque(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaConceptosEmbarque(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void GrabaConceptoMovAlma(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BajaConceptoMovAlma(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ConsultaConceptoMovAlma(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void GrabaDepartamento(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BajaDepartamento(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ConsultaDepartamento(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void GrabaPuesto(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BajaPuesto(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ConsultaPuesto(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaUsuarios(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaUsuario(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaUsuario(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaUsuario(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros, AnsiString Tabla, AnsiString CampoClave);

        void GrabaTerminal(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BajaTerminal(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ConsultaTerminal(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void GrabaTipoDeBloqueo(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BajaTipoDeBloqueo(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ConsultaTipoDeBloqueo(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void GrabaTipoDeImpuesto(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BajaTipoDeImpuesto(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaTipoDeImpuesto(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaTipoDeMoneda(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaTipoDeMoneda(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaTipoDeMoneda(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void GrabaTipoDePrivilegio(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BajaTipoDePrivilegio(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ConsultaTipoDePrivilegio(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void GrabaTipoDeTranxcob(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BajaTipoDeTranxcob(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ConsultaTipoDeTranxcob(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void GrabaTipoDeTranxpag(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BajaTipoDeTranxpag(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ConsultaTipoDeTranxpag(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void GrabaTipoDePrecio(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BajaTipoDePrecio(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ConsultaTipoDePrecio(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void GrabaViaEmbarque(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BajaViaEmbarque(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ConsultaViaEmbarque(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void GrabaImpuesto(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BajaImpuesto(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ConsultaImpuesto(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void GrabaMoneda(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BajaMoneda(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ConsultaMoneda(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void GrabaFolio(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BajaFolio(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaFolio(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaParametro(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaParametro(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaParametro(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaParametroGlobal(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaParametroGlobal(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void GrabaMarca(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BajaMarca(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ConsultaMarca(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void GrabaClasificacion1(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BajaClasificacion1(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ConsultaClasificacion1(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void GrabaClasificacion2(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BajaClasificacion2(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ConsultaClasificacion2(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void GrabaClasificacion3(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BajaClasificacion3(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ConsultaClasificacion3(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void GrabaArticulo(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BajaArticulo(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ConsultaArticulo(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void ConsultaProducto(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void ConsultaPresentacion(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void ConsultaMultiplo(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void GrabaColonia(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BajaColonia(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ConsultaColonia(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void GrabaGrupoObjetos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BajaGrupoObjetos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ConsultaGrupoObjetos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void GrabaObjetoSistema(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BajaObjetoSistema(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ConsultaObjetoSistema(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void GrabaPrivilegio(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BajaPrivilegio(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ConsultaPrivilegio(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void GrabaClasifCheque(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BajaClasifCheque(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ConsultaClasifCheque(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void GrabaSeccion(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BajaSeccion(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ConsultaSeccion(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void GrabaTiposFacturas(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ConsultaTiposFacturas(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BajaTiposFacturas(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void GrabaTerminoPago(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ConsultaTerminoPago(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaTerminoPago(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ConsultaFoliosCFD(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaFoliosCFD(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaFoliosCFD(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ConsultaCuentasContables(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaCuentasContables(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaPolizaInterna(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ConsultaPlantillasCFD(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaPlantillasCFD(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaPlantillaCFD(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void CopiaPlantilla(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ConsultaDomicilios(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ConsultaDatosClientes(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ConsultaConceptosVenta(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void AplicaConceptosVenta(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void BajaVolumenes(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaVolumenes(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ConsultaCuotas(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaCuotas(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaCuotas(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaClasifcont(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaClasifcont(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaClasifcont(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaClasifcont2(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaClasifcont2(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaClasifcont2(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaModPresentacion(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaAutorizarPrecioLocal(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaPreciosBloqueados(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void BuscaBitPrecDif(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaProdServ(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaProdServ(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaProdServ(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaClaveUni(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaClaveUni(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaClaveUni(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaGiro(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaGiro(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaGiro(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaVerificadorImg(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void CargaEmbarqueRuta(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ModificarEstadoArticulo(RespuestaServidor *Respuesta, MYSQL *MySQL,char *parametros);

		void GrabaFabricante(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaFabricante(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaFabricante(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaSegmento(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaSegmento(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaSegmento(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void AplicaProdClave(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void AplicaReclasificaProductos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void AplicaReclasificatProdCon(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void AplicaReclasificatFabProd(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void AplicaReclasificatSegCom(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaAriculosVentaInternet(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaAriculosVentaInternet(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaAriculosVentaInternet(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);


		void GuardaCodigoPostal(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ActualizaCodigoPostal(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);


		void GrabaArticulosTags(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaArticulosTags(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaArticulosTags(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);


		void GuardaOrdenes(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GuardaOrdenDetalle(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void OrdenesCompra(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ActualizarPedidos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BitacoraPedidos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ActualizaProdECommerce(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaArtActxSuc(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GuardaArtActxSuc(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BorraArtActxSuc(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ArticulosOrden(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ObtenerArticulosOrden(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GuardaProdReembolsar(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ObtenerArtReembolsados(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ActualizarOrdenReembolsada(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ActualizarPedidoVtaRel(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ArticulosVentaRelacionada(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void AsignaArticuloxSuc(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaFraccionarproductos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaActuTipoPrecioProductos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaActuDescRappi(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaActuRappiProductos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaObtenerArtiPesoProm(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GuardaArtiPesoProm(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ActuArtiPesoProm(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void DeleteArtiPesoProm(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ActuaOrdenVisita(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaFolioTransfActual(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaGuardaBitacoraTransaccionBilleto(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaObtenerTransfCancelar(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaGuardaBitacoraDepositoBilleto(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaGuardaDepositoCompletoBilleto(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaGuardaBitacoraRetiroBilleto(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaGuardaRetiroCompletoBilleto(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ConsultaFormasDePago(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaFormasDePago(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaFormasDePago(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ConsultaFraccionEcommerce(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GuardaFraccionEcommerce(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaFraccionEcommerce(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GuardaArtSupervisados(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BorraArtSupervisados(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);


		void ConsultaOfertas(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaOfertas(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaOfertas(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ConsultaCancelaciones(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaCancelaciones(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaCancelaciones(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ConsultaOrigenxPago(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaOrigenxPago(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaOrigenxPago(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaModClaveProducto(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ConsultaMovsCaja(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaMovsCaja(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaMovsCaja(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void ConsultaPinpad(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaPinpad(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaPinpad(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaAseguradora(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaAseguradora(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaAseguradora(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaRemolque(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaRemolque(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaRemolque(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaActuEstatusProductos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ConsultaInfoQRTickets(RespuestaServidor *Respuesta,	MYSQL *MySQL, char *parametros);
		void GrabaInfoQRTickets(RespuestaServidor *Respuesta,	MYSQL *MySQL, char *parametros);
		void BajaInfoQRTickets(RespuestaServidor *Respuesta,	MYSQL *MySQL, char *parametros);

		void ConsultaMotivoCancelaciones(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaMotivoCancelaciones(RespuestaServidor *Respuesta,MYSQL *MySQL, char *parametros);
		void GrabaMotivoCancelaciones(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaBitacoraEcommerce(RespuestaServidor *Respuesta,	MYSQL *MySQL, char *parametros);

		void ActuaPrecioMod(RespuestaServidor *Respuesta,	MYSQL *MySQL, char *parametros);

		void GrabaProductoSinIESPS(RespuestaServidor *Respuesta,	MYSQL *MySQL, char *parametros);
		void ConsultaProductoSinIESPS(RespuestaServidor *Respuesta,	MYSQL *MySQL, char *parametros);
		void BajaProductoSinIESPS(RespuestaServidor *Respuesta,	MYSQL *MySQL, char *parametros);

		void ConsultaSociedadMercantil(RespuestaServidor *Respuesta,	MYSQL *MySQL, char *parametros);
		void GrabaSociedadMercantil(RespuestaServidor *Respuesta,	MYSQL *MySQL, char *parametros);
		void BajaSociedadMercantil(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void BajaSegmentoContAdi(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaSegmentoContAdi(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaSegmentoContAdi(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaRol(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaRol(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaRol(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaCancPrepago(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GuardaNotificacion(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaImagenArticulo(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaImagenesArticulo(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaImagenArticulo(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaImagenArticulo(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ReordenarImagen(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaModVolumen(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaModPeso(RespuestaServidor *Respuesta,MYSQL *MySQL, char *parametros);

		void ConsultaOrigenVenta(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaOrigenVenta(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaOrigenVenta(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

       	void ConsultaParametrocfdiweb(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaParametrocfdiweb(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaDescripcionArticulo(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaTipos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaRegimen(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaRegimen(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaRegimen(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaUsoCFDI(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaCFDI(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaCFDI(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaNOTCREPAG(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaTNOTCREPAG(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaTNOTCREPAG(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaTerminosCobranza(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaTerminosCobranza(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaTerminosCobranza(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaParametroBancos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaParametroBancos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BajaParametroBancos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ConsultaMsjPinPad(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaMsjPinPad(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaMsjPinPad(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ConsultaMetPagEcom(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaMetPagEcom(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaMetPagEcom(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ConsultaCartPorte20(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaCartPorte20(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaCartPorte20(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaActualizaCostosxGrupo(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ConsultaPlantillaCredEmpleado(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GuardaEmpleadoCredencial(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ConsultaCancelacionPrepago(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaCancelacionPrepago(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaCancelacionPrepago(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaCredEmpleado(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaCredEmpleado(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaBannerImg(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaBannerImg(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GuardarBitacoraAlmacen(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		
		void BuscArpedM(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GuardarBitacoraTiposPrecios(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaConsultaFactorArticulo (RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaAsociadosxCliente(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaEmpleadosAsociados(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaEmpleadosAsociados(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EliminarArticulo(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void RomperCaja(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ModificarProductoPresent(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ConsultaEmpresa(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaEmpresa(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BorraEmpresa(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaPosicionProducto(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        
		void GrabaUsuarioSistema(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaConsultaNuevoArticulo(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ModificarMultiplo(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaArticuloEmpresaCfg(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaPresentacionActiva(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaObjetivosVendedores(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ModificaObjetivosVendedores(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaObjetivosSucursales(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ModificaObjetivosSucursal(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void RedondeaPrecioCatArt(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConfPreciosCatArticulos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaArticulosxActividad(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaConfiguraArtxActividad(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaConsultaArtxActividad(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void ConsultaMotivoDevolucionNC(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void GrabaMotivoDevolucionNC(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BorraMotivoDevolucionNC(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);


		void GrabaParametroGeneral(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		//void BajaParametro(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaParametroGeneral(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaGrabaSolTicketSoporte(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaConsultaSolTicketSoporte(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BusquedaSolTicketSoporte(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaEnvioCorreoTicketSoporte(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaConsultaCatalogoDepartamentosSoporte(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaGrabaCatalogoDepartamentosSoporte(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaCambiarProductoPresentacion(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ConsultaCuentasRetenciones(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaCuentaRetencion(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaCuentaRetencion(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ReporteImagenesPresentaciones(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ImagenPresentacion(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void CancelaOrdenTrabajo(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaJefaturasOrdenesTrabajo(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaJefaturasOrdenesTrabajo(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaConsultaCatalogoBajasRH(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void EjecutaGrabaCatalogoBajasRH(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
};
#endif

