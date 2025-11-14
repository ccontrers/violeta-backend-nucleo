//---------------------------------------------------------------------------

#ifndef ClassServidorAlmacenH
#define ClassServidorAlmacenH
//---------------------------------------------------------------------------
#include "ClassDatosTabla.h"
#include "ClassFuncionesGenericas.h"
#include "ClassRespuestasServidor.h"
#include <mysql.h>

class ServidorVioleta;
//---------------------------------------------------------------------------
/** Clase destinada a ser la parte del servidor que se encarga de despachar
* las peticiones relacionadas con las VENTAS
*/
class ServidorAlmacen {
    private:
        ServidorVioleta *mServidorVioleta; /**< Puntero al objeto servidor principal*/
        FuncionesGenericas mFg; /**< Objeto con funciones genéricas */

    public:
        /** Constructor, lo único que hace es unir este objeto con el servidor principal.
        */
        ServidorAlmacen(ServidorVioleta *Servidor) {mServidorVioleta=Servidor;};

        /* --
        * @param Respuesta Puntero al objeto donde se va a poner el resultado.
        * @param MySQL Puntero a la conexión MySql que se debe usar para esta petición.
        * @param parametros Puntero al buffer de parámetros.
        */
        void GrabaMovimientoAlmacen(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void CancelaMovimientoAlmacen(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ConsultaMovimientoAlmacen(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void GrabaTrasformacionAlmacen(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void CancelaTrasformacionAlmacen(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ConsultaTrasformacionAlmacen(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void GrabaInventario(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BorraInventario(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ConsultaInventario(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaLotInventario(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaLoteinvMovil(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BorraLotInventario(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ConsultaLotInventario(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void GrabaArtInventario(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BorraArtInventario(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ConsultaArtInventario(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void GrabaEmbarque(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void CancelaEmbarque(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);             

        void ConsultaExistenciasArticulo(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaFrameArticulo(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaRecepcionesMovil(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaRecepcionEditada(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaAjustesMovimientoAlmacen(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ResumenCargaEmbarque(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaMovimientoSurtidor(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaErrorEmbarque(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaErrorEmbarque(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void CancelarErrorEmbarque(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ReporteErrorEmbarque(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaAlmacenDistribucionFrente(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaAlmacenDistribucionFrente(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaAlmacenDistribucionFamilia(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaAlmacenDistribucionFamilia(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ModificarEstadoUbicaciones(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ModificarUbicacionesEspeciales(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ModLoteInventario(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void MoverLoteInventario(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void AuditarMovTranfs(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void UpdateAuditado(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepBitacoraMovAlma(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaRepBitacoraDetMovAlma(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaAplicaMovAlma(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaAlmacenDistribucionProductos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaAlmacenDistribucionProductos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EditaNombreCalle(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaAlmacenDistribucionProductosFamilia(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

};

#endif
