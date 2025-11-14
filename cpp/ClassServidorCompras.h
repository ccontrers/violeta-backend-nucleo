//---------------------------------------------------------------------------

#ifndef ClassServidorComprasH
#define ClassServidorComprasH
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
class ServidorCompras {
    private:
        ServidorVioleta *mServidorVioleta; /**< Puntero al objeto servidor principal*/
        FuncionesGenericas mFg; /**< Objeto con funciones genéricas */

    public:
        /** Constructor, lo único que hace es unir este objeto con el servidor principal.
        */
        ServidorCompras(ServidorVioleta *Servidor) {mServidorVioleta=Servidor;};

        /* --
        * @param Respuesta Puntero al objeto donde se va a poner el resultado.
        * @param MySQL Puntero a la conexión MySql que se debe usar para esta petición.
        * @param parametros Puntero al buffer de parámetros.
        */
        void GrabaCompra(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void CancelaCompra(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ConsultaCompra(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void GrabaPedidoProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void CancelaPedidoProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ConsultaPedidoProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void GrabaDevolProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void CancelaDevolProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ConsultaDevolProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void GrabaPagoProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ConsultaPagoProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void CancelaPagoProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ModificaFechaPagoProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ConsultaPagoProvImprimirCheque(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ConsultaPagosProvDelDia(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ConsultaAuxiliarProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void ConsolidaChequesProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ConsultaChequesxfechaProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ConsultaChequesxfechaProvNC(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ModificaChequeProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void ConsultaComprasProveedor(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void GrabaCargoProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void CancelaCargoProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void ConsultaCargoProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

        void GrabaCargoReboteProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void CancelaCargoReboteProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaCargoReboteProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaBloqueadosCompra(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ConsultaComprasParaImprimir(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaPedidosParaImprimir(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ModificaAlmacenEntradaCompras(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaRepRecepcion(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaRecepcionProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaInfRecepcion(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void registraAgendaRecepcion(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void modificaAgendaRecepcion(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void registraAduana(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void EjecutaDetalleRecepcion(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaRecepcionGenerada(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaFraccionaRecepcionxPedido(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaBusqPedidoRecepcionar(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void EjecutaGuardaRecepcion(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaInformacionRecepciones(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
};

#endif
