
#ifndef ClassServidorBusquedasH
#define ClassServidorBusquedasH

#include "ClassDatosTabla.h"
#include "ClassFuncionesGenericas.h"
#include "ClassRespuestasServidor.h"
#include <mysql.h>

#define NUM_LIMITE_RESULTADOS_BUSQ "501"
#define NUM_LIMITE_RESULTADOS_BUSQ2 "3000"

class ServidorVioleta;
//---------------------------------------------------------------------------
/** Clase destinada a ser la parte del servidor que se encarga de despachar
* las peticiones hacerca de los catálogos del sistema.
*/
class ServidorBusquedas {
    private:
        ServidorVioleta *mServidorVioleta; /**< Puntero al objeto servidor principal*/
        FuncionesGenericas mFg; /**< Objeto con funciones genéricas */

    public:
        /** Constructor, lo único que hace es unir este objeto con el servidor principal.
        */
        ServidorBusquedas(ServidorVioleta *Servidor) {mServidorVioleta=Servidor;};

        /* --
        * @param Respuesta Puntero al objeto donde se va a poner el resultado.
        * @param MySQL Puntero a la conexión MySql que se debe usar para esta petición.
        * @param parametros Puntero al buffer de parámetros.
        */
        void BuscaArticulos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BuscaClientes(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BuscaProveedores(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BuscaUsuarios(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BuscaEmpleados(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void BuscaCompras(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BuscaPedidosProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BuscaNotasCredProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BuscaNotasCargProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BuscaPagosProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BuscaChequesProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void BuscaVentas(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BuscaPedidosCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BuscaNotasCredCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BuscaNotasCargCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BuscaPagosCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BuscaChequesCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void BuscaMovimientosAlmacen(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BuscaKits(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BuscaCortes(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BuscaInventarios(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BuscaFormasImpresion(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BuscaMarcas(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void BuscaEmbarques(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void BuscaTransformacion(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BuscaLoteInventario(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void BuscaMsgsMoviles(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void BuscaRecepciones(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void BuscaCartasPorte(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BuscaProdServ(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BuscaVendedores(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BuscaArticulosCandidatosAgregar(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void BuscaPagosGastos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BuscaNotasCredGasto(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BuscaGastos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void BuscaCartaPorte20(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void BuscaPrePagosCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BuscaPedidosEcom(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BuscaRecepcion(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void BuscaSurtido(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
};

//---------------------------------------------------------------------------
#endif
