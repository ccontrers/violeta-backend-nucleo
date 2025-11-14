//---------------------------------------------------------------------------

#ifndef ClassServidorEcommerceH
#define ClassServidorEcommerceH
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
class ServidorEcommerce {
    private:
        ServidorVioleta *mServidorVioleta; /**< Puntero al objeto servidor principal*/
        FuncionesGenericas mFg; /**< Objeto con funciones genéricas */

    public:
        /** Constructor, lo único que hace es unir este objeto con el servidor principal.
        */
		ServidorEcommerce(ServidorVioleta *Servidor) {mServidorVioleta=Servidor;};

		void GrabaGenerico(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros, AnsiString Tabla, AnsiString CampoClave);
		void BajaGenerico(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros, AnsiString Tabla, AnsiString CampoClave);
        void ConsultaGenerico(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros, AnsiString Tabla, AnsiString CampoClave);

		void GrabaClasificacionEcom1(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaClasificacionEcom1(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaClasificacionEcom1(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaClasificacionEcom2(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaClasificacionEcom2(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaClasificacionEcom2(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaClasificacionEcom3(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaClasificacionEcom3(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaClasificacionEcom3(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void AplicaReclasificaProductosEcommerce(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GuardaTagsArticulosEcommerce(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaTagsArticulosEcommerce(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaArticulosEcomTags(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaArticulosEcomTags(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaArticulosEcomTags(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaParametroEcom(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaParametroEcom(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaParametroEcom(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaBanners(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaBanners(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaBanner(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaBanner(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ReordenarBanner(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void GrabaHorarioEntrega(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaHorarioEntrega(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaHorarioEntrega(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ConsultaImagenClasif(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaImagenClasif(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void BajaImagenClasif(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

		void ConsultaOrdenesEcomm(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void ConsultaDetalleOrdenesEcomm(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
		void GrabaDetPedidoSurtidoEcomm(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void EjecutaActualizaEstatusPedidoEcomm(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);
        void EjecutaActualizaVentaPedidoEcomm(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros);

};
#endif
