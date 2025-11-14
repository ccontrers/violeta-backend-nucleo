//---------------------------------------------------------------------------

#ifndef ClassServidorBancosH
#define ClassServidorBancosH
//---------------------------------------------------------------------------

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
class ServidorBancos {
    private:
        ServidorVioleta *mServidorVioleta; /**< Puntero al objeto servidor principal*/
        FuncionesGenericas mFg; /**< Objeto con funciones genéricas */

    public:
        /** Constructor, lo único que hace es unir este objeto con el servidor principal.
        */
		ServidorBancos(ServidorVioleta *Servidor) {mServidorVioleta=Servidor;};

		/* --
		* @param Respuesta Puntero al objeto donde se va a poner el resultado.
		* @param MySQL Puntero a la conexión MySql que se debe usar para esta petición.
		* @param parametros Puntero al buffer de parámetros.
		*/
		void GrabaBancoPagosefe(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BuscaBancoPagosefe(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaBancoPagosefe(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void CancelaBancoPagosefe(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void RepRelacionPagosefeMovimientosBancarios(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaMovBanco(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BuscaMovBanco(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaMovimientoBancos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void CancelaMov(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaSucursalBancos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaConfiguracionBancos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ReporteAuxiliarBancos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ReporteSaldos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaBancoVta(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BuscaBancoVta(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaBancoVta(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void CancelaBancoVta(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaGenerico(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros, AnsiString Tabla, AnsiString CampoClave);
		void RepRelacionVentaMovimientosBancarios(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

};
#endif
