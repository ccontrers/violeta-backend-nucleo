
#ifndef ClassServidorAdminSistemaH
#define ClassServidorAdminSistemaH

#include "ClassDatosTabla.h"
#include "ClassFuncionesGenericas.h"
#include "ClassRespuestasServidor.h"
#include <mysql.h>

class ServidorVioleta;
//---------------------------------------------------------------------------
/** Clase destinada a ser la parte del servidor que se encarga de despachar
* las peticiones relacionadas con las compras, como COMPRAS, PEDIDOS, NOTAS DE CREDITO
* DE PROVEEDOR, etc.
*/
class ServidorAdminSistema {
	private:
		ServidorVioleta *mServidorVioleta; /**< Puntero al objeto servidor principal*/
		FuncionesGenericas mFg; /**< Objeto con funciones genéricas */

	public:
		/** Constructor, lo único que hace es unir este objeto con el servidor principal.
		*/
		ServidorAdminSistema(ServidorVioleta *Servidor) {mServidorVioleta=Servidor;};

		/* --
		* @param Respuesta Puntero al objeto donde se va a poner el resultado.
		* @param MySQL Puntero a la conexión MySql que se debe usar para esta petición.
		* @param parametros Puntero al buffer de parámetros.
		*/

		void CalculaCostosPromedioAlDia(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void CalculaVentasAcumuladas(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void CalculaVentasMensuales(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ConsultaAsigPriv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaPriv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConcedeAsigPriv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void QuitaAsigPriv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void CopiaAsigPriv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ObtieneAsigPriv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConcedeAsigRol(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void QuitaAsigRol(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConcedeAsigSuc(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void QuitaAsigSuc(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ObtieneAsigPrivUsuaRol(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ObtieneAsigPrivUsuaRolEspec(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void AsignaClave(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void CambiaClave(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void AsignaUsuarioPassContpaq(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void VerificaAcceso(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void VerificaAccesoEspecifico(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void PideListaUsuarios(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void PideListaFormas(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaCorte(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void CancelaCorte(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaCorte(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaCorteCostos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaForma(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaForma(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaForma(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EnvioDatosPocket(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void EnvioDatosPocketNew(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void EnvioDatosAppRutas(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaMsgsMoviles(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void CancelaMsgsMoviles(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaMsgsMoviles(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EnvioDatosInvMovil(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EnvioDatosArticulo(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaCfdiCompra(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaCfdiPagosProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaCfdiNcarProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaCfdiNcreProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void PrecalculoCostosMensual(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaEstadoServidor(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaReplicacionServidor(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaExistenciasActuales(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GuardarEnBitacora(char *parametros);

		void GrabaBitacoraFechaCorte(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ConsultaBitacoraUnificada(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GoogleMapsAPIKey(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ShopifyAPIKey(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ShopifyUserShop(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void PrecalculoCostoVentas(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void PrecalculoCostoMov(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void PrecalculoHistExis(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);


		void EnvioDatosPocketNewB(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaEtiquetaAutomatico(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void RappiKeyShop(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void SimpliRoutesKey(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void BilletoClientID(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BilletoClientSecret(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BilletoLocationID(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaBitacoraFechaVencimiento(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaCfdiPagosGastos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);


        void GrabaBitaConfigPinpad(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EnvioDatosInvMovilB(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ActualizaPreciosManual(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ConsultaAsigPrivRol(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConcedeAsigPrivRol(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void QuitaAsigPrivRol(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void CopiaAsigPrivRol(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void CopiaAsigPrivGloUsuario(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ObtieneAsigPrivRol(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ConsultaPrivRol(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void PideListaRoles(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void ConsultaAsigUsuaRol(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ConsultaPagRfcEmiXML(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ConsultaTransacPag(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void FireBaseAPIKey(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaPaquetesDesCfdi(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaPaquetesDesCfdi(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaSolicitudesDesCfdi(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaSolicitudesDesCfdi(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ActualizaSolicitudesDesCfdi(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ActualizaPaquetesDesCfdi(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void QuitaPaquetesDesCfdi(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ConsultaFacturasCompras(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaFacturasNCC(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ConsultaFacturasNCP(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaFacturasVentas(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void DiferenciasVentasMensuales(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void CalculaPresentacionesMinMax(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void InicioCalculosSistema(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void FinCalculosSistema(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void VentasConSaldo(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaVigenciaPedidos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ObtieneTokenRPM(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaExistenciasIniciales(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void CalculaCorteInicialCostos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void CalculaPresentacionesActivas(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void AsignaPassword(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ClasificaArticulos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ConsultaConfPinpadBBVA(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaConfPinpadBBVA(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BorraConfPinpadBBVA(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaBitaConfPinpadBBVA(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
};

//---------------------------------------------------------------------------
#endif

