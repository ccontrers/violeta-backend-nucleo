//---------------------------------------------------------------------------

#ifndef ClassServidorGastosH
#define ClassServidorGastosH
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
class ServidorGastos {
    private:
        ServidorVioleta *mServidorVioleta; /**< Puntero al objeto servidor principal*/
        FuncionesGenericas mFg; /**< Objeto con funciones genéricas */

    public:
        /** Constructor, lo único que hace es unir este objeto con el servidor principal.
        */
		ServidorGastos(ServidorVioleta *Servidor) {mServidorVioleta=Servidor;};

        /* --
        * @param Respuesta Puntero al objeto donde se va a poner el resultado.
        * @param MySQL Puntero a la conexión MySql que se debe usar para esta petición.
        * @param parametros Puntero al buffer de parámetros.
        */
		void GrabaGasto(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaGasto(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void CancelaGasto(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ConsultaGastosProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaPagoGasto(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaPagoGasto(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaPagosGastoDelDia(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void CancelaPagoGasto(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ModificaFechaPagoGasto(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaConsultaFactGasto(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaDevolGasto(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaDevolGastos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void CancelaDevolGasto(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaRepGastosXFactura(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
    	void EjecutaRepGastosXProveedor(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);


};
#endif
