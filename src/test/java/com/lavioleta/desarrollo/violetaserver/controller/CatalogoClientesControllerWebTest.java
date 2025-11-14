package com.lavioleta.desarrollo.violetaserver.controller;

import com.fasterxml.jackson.databind.ObjectMapper;
import com.lavioleta.desarrollo.violetaserver.config.TestSecurityConfig;
import com.lavioleta.desarrollo.violetaserver.dto.ClienteDetalleEcommerceDTO;
import com.lavioleta.desarrollo.violetaserver.dto.DatosCreditoDTO;
import com.lavioleta.desarrollo.violetaserver.dto.request.BajaClienteRequest;
import com.lavioleta.desarrollo.violetaserver.dto.request.ClienteRequest;
import com.lavioleta.desarrollo.violetaserver.dto.response.ClienteListResponse;
import com.lavioleta.desarrollo.violetaserver.dto.response.ClienteResponse;
import com.lavioleta.desarrollo.violetaserver.service.CatalogoClientesService;
import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.BeforeEach;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc;
import org.springframework.boot.test.autoconfigure.web.servlet.WebMvcTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.context.annotation.Import;
import org.springframework.http.MediaType;
import org.springframework.test.context.ActiveProfiles;
import org.springframework.test.web.servlet.MockMvc;

import java.math.BigDecimal;

import static org.mockito.ArgumentMatchers.*;
import static org.mockito.Mockito.when;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.*;
import org.springframework.http.HttpMethod;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.*;

/**
 * Pruebas de capa web aislada para {@link CatalogoClientesController}.
 * Enfocadas en:
 *  - Mapeo de endpoints y status codes.
 *  - Validaciones Bean Validation vs códigos HTTP.
 *  - Diferenciación entre success=false y excepciones (400/404 vs 500).
 *  - Respuestas planas vs JSON (endpoint /status).
 */
@WebMvcTest(CatalogoClientesController.class)
@Import(TestSecurityConfig.class)
@AutoConfigureMockMvc(addFilters = false)
@ActiveProfiles("test")
@DisplayName("Web Layer: CatalogoClientesController")
class CatalogoClientesControllerWebTest {

    private static final String BASE = "/api/v1/catalogos/clientes";

    @Autowired
    MockMvc mockMvc;

    @Autowired
    ObjectMapper objectMapper;

        // @MockBean (Spring 3.4+ deprecado: alternativa futura será usar @TestConfiguration + beans Mockito)
        @MockBean
    CatalogoClientesService clientesService;

    private ClienteRequest validRequest;
    private ClienteResponse successResponse;
    private ClienteResponse.ClienteCompleto clienteMock;

    @BeforeEach
    void setup() {
        // Mocks comunes
        when(clientesService.obtenerEmpresaPorSucursal(anyString())).thenReturn(1);

        validRequest = buildValidClienteRequest();
        clienteMock = ClienteResponse.ClienteCompleto.builder()
                .cliente(validRequest.getCliente())
                .nombre(validRequest.getNombre())
                .activo(true)
                .build();
        successResponse = ClienteResponse.builder()
                .success(true)
                .message("Operación exitosa")
                .cliente(clienteMock)
                .build();
    }

    private ClienteRequest buildValidClienteRequest() {
        ClienteRequest r = new ClienteRequest();
        r.setOperacion("A");
        r.setCliente("CLT001");
        r.setIdEmpresa(1);
        r.setNombre("Cliente Test");
        r.setRsocial("Empresa Test SA");
        r.setNomnegocio("Negocio Test");
        r.setRfc("TES123456789");
        // Valores reales válidos (derivados de análisis de BD)
        r.setColonia("EPER");
        r.setGiro("OTRO");
        r.setCanal("ME");
        r.setRegimenfiscal("622");
        r.setMetododef("02");
        r.setMetodosup("02");
        r.setDigitosdef("0001");
        r.setDigitossup("0002");
        r.setUsocfdi("G01");
        r.setCalle("Calle 123");
        return r;
    }

    /* =====================================================
       CRUD PRINCIPAL (POST + DELETE)
     ===================================================== */
    @Nested
    @DisplayName("POST /clientes")
    class GrabarCliente {
        @Test
        @DisplayName("Crear cliente exitoso -> 200")
        void crearOk() throws Exception {
            when(clientesService.grabarCliente(any(ClienteRequest.class), anyString()))
                    .thenReturn(successResponse);

            mockMvc.perform(post(BASE)
                            .contentType(MediaType.APPLICATION_JSON)
                            .header("X-Sucursal", "S1")
                            .content(objectMapper.writeValueAsString(validRequest)))
                    .andExpect(status().isOk())
                    .andExpect(jsonPath("$.success").value(true))
                    .andExpect(jsonPath("$.message").value("Operación exitosa"));
        }

        @Test
        @DisplayName("Crear cliente datos inválidos -> 400 (validation handler)")
        void crearInvalido() throws Exception {
            ClienteRequest invalido = new ClienteRequest(); // Falta todo lo requerido para disparar Bean Validation
            // No stub de servicio: la validación debería ocurrir antes de invocar el service
            mockMvc.perform(post(BASE)
                            .contentType(MediaType.APPLICATION_JSON)
                            .content(objectMapper.writeValueAsString(invalido)))
                                        .andExpect(status().isBadRequest())
                                        .andExpect(jsonPath("$.errorCode").value("VALIDATION_ERROR"))
                                        .andExpect(jsonPath("$.fieldErrors").isArray());
        }

        @Test
        @DisplayName("Crear cliente servicio retorna success=false -> 400")
        void crearRespuestaNegativa400() throws Exception {
            ClienteResponse bad = ClienteResponse.builder()
                    .success(false)
                    .message("Datos inválidos")
                    .build();
            when(clientesService.grabarCliente(any(ClienteRequest.class), anyString()))
                    .thenReturn(bad);

            mockMvc.perform(post(BASE)
                            .contentType(MediaType.APPLICATION_JSON)
                            .content(objectMapper.writeValueAsString(validRequest)))
                    .andExpect(status().isBadRequest())
                    .andExpect(jsonPath("$.success").value(false));
        }

        @Test
        @DisplayName("Crear cliente excepción en servicio -> 500")
        void crearExcepcion500() throws Exception {
            when(clientesService.grabarCliente(any(ClienteRequest.class), anyString()))
                    .thenThrow(new RuntimeException("Fallo inesperado"));

            mockMvc.perform(post(BASE)
                            .contentType(MediaType.APPLICATION_JSON)
                            .content(objectMapper.writeValueAsString(validRequest)))
                    .andExpect(status().isInternalServerError())
                    .andExpect(jsonPath("$.success").value(false));
        }

        @Test
        @DisplayName("Modificar cliente (operacion M) -> 200")
        void modificarOk() throws Exception {
            ClienteRequest mod = buildValidClienteRequest();
            mod.setOperacion("M");
            ClienteResponse actualizado = ClienteResponse.builder()
                    .success(true)
                    .message("Actualizado")
                    .cliente(clienteMock)
                    .build();
            when(clientesService.grabarCliente(any(ClienteRequest.class), anyString()))
                    .thenReturn(actualizado);

            mockMvc.perform(post(BASE)
                            .contentType(MediaType.APPLICATION_JSON)
                            .content(objectMapper.writeValueAsString(mod)))
                    .andExpect(status().isOk())
                    .andExpect(jsonPath("$.message").value("Actualizado"));
        }
    }

    @Nested
    @DisplayName("DELETE /clientes/{id}")
    class EliminarCliente {
        @Test
        @DisplayName("Eliminar cliente OK -> 200")
        void eliminarOk() throws Exception {
            ClienteResponse ok = ClienteResponse.builder().success(true).message("Eliminado").build();
            when(clientesService.eliminarCliente(any(BajaClienteRequest.class))).thenReturn(ok);

            mockMvc.perform(delete(BASE + "/{codigo}", "CLT001")
                            .contentType(MediaType.APPLICATION_JSON))
                    .andExpect(status().isOk())
                    .andExpect(jsonPath("$.success").value(true));
        }

        @Test
        @DisplayName("Eliminar cliente success=false -> 400")
        void eliminarBadRequest() throws Exception {
            ClienteResponse bad = ClienteResponse.builder().success(false).message("No se puede borrar").build();
            when(clientesService.eliminarCliente(any(BajaClienteRequest.class))).thenReturn(bad);

            mockMvc.perform(delete(BASE + "/{codigo}", "CLT002")
                            .contentType(MediaType.APPLICATION_JSON))
                    .andExpect(status().isBadRequest())
                    .andExpect(jsonPath("$.success").value(false));
        }

        @Test
        @DisplayName("Eliminar cliente pasando body BajaClienteRequest -> 200")
        void eliminarConBody() throws Exception {
            ClienteResponse ok = ClienteResponse.builder().success(true).message("Eliminado").build();
            when(clientesService.eliminarCliente(any(BajaClienteRequest.class))).thenReturn(ok);
            BajaClienteRequest baja = new BajaClienteRequest();
            baja.setCliente("CLT010");
            baja.setMotivo("Depuracion");

            mockMvc.perform(delete(BASE + "/{codigo}", "CLT010")
                            .contentType(MediaType.APPLICATION_JSON)
                            .content(objectMapper.writeValueAsString(baja)))
                    .andExpect(status().isOk())
                    .andExpect(jsonPath("$.message").value("Eliminado"));
        }

        @Test
        @DisplayName("Eliminar cliente excepción -> 500")
        void eliminarExcepcion() throws Exception {
            when(clientesService.eliminarCliente(any(BajaClienteRequest.class)))
                    .thenThrow(new RuntimeException("Error grave"));

            mockMvc.perform(delete(BASE + "/{codigo}", "CLT003")
                            .contentType(MediaType.APPLICATION_JSON))
                    .andExpect(status().isInternalServerError());
        }
    }

    /* =====================================================
       CONSULTA Y LISTADO
     ===================================================== */
    @Nested
    @DisplayName("GET /clientes/{id}")
    class ConsultarCliente {
        @Test
        @DisplayName("Cliente existe -> 200")
        void existe() throws Exception {
            when(clientesService.consultarCliente(eq("CLT001"), anyInt()))
                    .thenReturn(successResponse);

            mockMvc.perform(get(BASE + "/{codigo}", "CLT001").header("X-Sucursal", "S1"))
                    .andExpect(status().isOk())
                    .andExpect(jsonPath("$.success").value(true));
        }

        @Test
        @DisplayName("Cliente no encontrado -> 404")
        void noEncontrado() throws Exception {
            ClienteResponse nf = ClienteResponse.builder().success(false).message("No existe").build();
            when(clientesService.consultarCliente(eq("CLT999"), anyInt()))
                    .thenReturn(nf);

            mockMvc.perform(get(BASE + "/{codigo}", "CLT999"))
                    .andExpect(status().isNotFound())
                    .andExpect(jsonPath("$.success").value(false));
        }

        @Test
        @DisplayName("Excepción en consulta -> 500")
        void consultaExcepcion() throws Exception {
            when(clientesService.consultarCliente(eq("CLT500"), anyInt()))
                    .thenThrow(new IllegalStateException("boom"));

            mockMvc.perform(get(BASE + "/{codigo}", "CLT500"))
                    .andExpect(status().isInternalServerError());
        }
    }

    @Nested
    @DisplayName("GET /clientes (listado)")
    class ListarClientes {
        @Test
        @DisplayName("Listado OK -> 200")
        void listadoOk() throws Exception {
            ClienteListResponse list = ClienteListResponse.builder().success(true).message("Consulta").build();
            when(clientesService.listarClientes(any(), anyInt(), anyInt(), anyInt())).thenReturn(list);

            mockMvc.perform(get(BASE).header("X-Sucursal", "S1"))
                    .andExpect(status().isOk())
                    .andExpect(jsonPath("$.success").value(true));
        }

        @Test
        @DisplayName("Listado success=false -> 400")
        void listadoBadRequest() throws Exception {
            ClienteListResponse bad = ClienteListResponse.builder().success(false).message("Error filtros").build();
            when(clientesService.listarClientes(any(), anyInt(), anyInt(), anyInt())).thenReturn(bad);

            mockMvc.perform(get(BASE))
                    .andExpect(status().isBadRequest())
                    .andExpect(jsonPath("$.success").value(false));
        }

        @Test
        @DisplayName("Listado excepción -> 500")
        void listadoExcepcion() throws Exception {
            when(clientesService.listarClientes(any(), anyInt(), anyInt(), anyInt()))
                    .thenThrow(new RuntimeException("Falla"));

            mockMvc.perform(get(BASE))
                    .andExpect(status().isInternalServerError());
        }
    }

    /* =====================================================
       HEAD EXISTENCIA
     ===================================================== */
    @Nested
    @DisplayName("HEAD /clientes/{id}")
    class HeadExistencia {
        @Test
        @DisplayName("Existe -> 200 sin body")
        void existe() throws Exception {
            when(clientesService.existeCliente("CLT001")).thenReturn(true);
            mockMvc.perform(request(HttpMethod.HEAD, BASE + "/CLT001"))
                    .andExpect(status().isOk())
                    .andExpect(content().string(""));
        }

        @Test
        @DisplayName("No existe -> 404")
        void noExiste() throws Exception {
            when(clientesService.existeCliente("CLT404")).thenReturn(false);
            mockMvc.perform(request(HttpMethod.HEAD, BASE + "/CLT404"))
                    .andExpect(status().isNotFound());
        }

        @Test
        @DisplayName("Excepción -> 500")
        void headExcepcion() throws Exception {
            when(clientesService.existeCliente("CLT500")).thenThrow(new RuntimeException("err"));
            mockMvc.perform(request(HttpMethod.HEAD, BASE + "/CLT500"))
                    .andExpect(status().isInternalServerError());
        }
    }

    /* =====================================================
       CRÉDITO
     ===================================================== */
    @Nested
    @DisplayName("/clientes/{id}/credito")
    class Credito {
        @Test
        @DisplayName("Obtener crédito existente -> 200")
        void obtenerCredito() throws Exception {
            DatosCreditoDTO dto = new DatosCreditoDTO();
            dto.setIdcliente("CLT001");
            dto.setMontosol(new BigDecimal("123.45"));
            when(clientesService.obtenerDatosCredito("CLT001")).thenReturn(dto);

            mockMvc.perform(get(BASE + "/{id}/credito", "CLT001"))
                    .andExpect(status().isOk())
                    .andExpect(jsonPath("$.idcliente").value("CLT001"));
        }

        @Test
        @DisplayName("Obtener crédito no existente -> 404")
        void obtenerCredito404() throws Exception {
            when(clientesService.obtenerDatosCredito("CLT404")).thenReturn(null);
            mockMvc.perform(get(BASE + "/{id}/credito", "CLT404"))
                    .andExpect(status().isNotFound());
        }

        @Test
        @DisplayName("Guardar crédito -> 200")
        void guardarCredito() throws Exception {
            DatosCreditoDTO dto = new DatosCreditoDTO();
            dto.setIdcliente("CLT001");
            dto.setMontosol(new BigDecimal("999.00"));
            when(clientesService.guardarDatosCredito(any(DatosCreditoDTO.class))).thenReturn(dto);

            mockMvc.perform(post(BASE + "/{id}/credito", "CLT001")
                            .contentType(MediaType.APPLICATION_JSON)
                            .content(objectMapper.writeValueAsString(dto)))
                    .andExpect(status().isOk())
                    .andExpect(jsonPath("$.montosol").value(999.00));
        }

        @Test
        @DisplayName("Guardar crédito excepción -> 500")
        void guardarCreditoExcepcion() throws Exception {
            when(clientesService.guardarDatosCredito(any(DatosCreditoDTO.class)))
                    .thenThrow(new RuntimeException("x"));
            DatosCreditoDTO dto = new DatosCreditoDTO();
            dto.setIdcliente("CLT500");

            mockMvc.perform(post(BASE + "/{id}/credito", "CLT500")
                            .contentType(MediaType.APPLICATION_JSON)
                            .content(objectMapper.writeValueAsString(dto)))
                    .andExpect(status().isInternalServerError());
        }
    }

    /* =====================================================
       ECOMMERCE
     ===================================================== */
    @Nested
    @DisplayName("/clientes/{id}/ecommerce")
    class Ecommerce {
        @Test
        @DisplayName("Obtener detalle ecommerce existente -> 200")
        void obtenerEcommerce() throws Exception {
            ClienteDetalleEcommerceDTO det = new ClienteDetalleEcommerceDTO();
            det.setIdcliente("CLT001");
            det.setActivo(true);
            when(clientesService.obtenerDetalleEcommerce("CLT001")).thenReturn(det);

            mockMvc.perform(get(BASE + "/{id}/ecommerce", "CLT001"))
                    .andExpect(status().isOk())
                    .andExpect(jsonPath("$.idcliente").value("CLT001"));
        }

        @Test
        @DisplayName("Obtener ecommerce no existente -> 404")
        void obtenerEcommerce404() throws Exception {
            when(clientesService.obtenerDetalleEcommerce("CLT404")).thenReturn(null);
            mockMvc.perform(get(BASE + "/{id}/ecommerce", "CLT404"))
                    .andExpect(status().isNotFound());
        }

        @Test
        @DisplayName("Guardar ecommerce -> 200")
        void guardarEcommerce() throws Exception {
            ClienteDetalleEcommerceDTO det = new ClienteDetalleEcommerceDTO();
            det.setIdcliente("CLT001");
            det.setActivo(true);
            when(clientesService.guardarDetalleEcommerce(any(ClienteDetalleEcommerceDTO.class))).thenReturn(det);

            mockMvc.perform(post(BASE + "/{id}/ecommerce", "CLT001")
                            .contentType(MediaType.APPLICATION_JSON)
                            .content(objectMapper.writeValueAsString(det)))
                    .andExpect(status().isOk())
                    .andExpect(jsonPath("$.idcliente").value("CLT001"));
        }

        @Test
        @DisplayName("Guardar ecommerce excepción -> 500")
        void guardarEcommerceExcepcion() throws Exception {
            when(clientesService.guardarDetalleEcommerce(any(ClienteDetalleEcommerceDTO.class)))
                    .thenThrow(new RuntimeException("err"));
            ClienteDetalleEcommerceDTO det = new ClienteDetalleEcommerceDTO();
            det.setIdcliente("CLT500");

            mockMvc.perform(post(BASE + "/{id}/ecommerce", "CLT500")
                            .contentType(MediaType.APPLICATION_JSON)
                            .content(objectMapper.writeValueAsString(det)))
                    .andExpect(status().isInternalServerError());
        }
    }

    /* =====================================================
       STATUS Y ERRORES GENERALES
     ===================================================== */
    @Nested
    @DisplayName("/status y errores comunes")
    class StatusYErrores {
        @Test
        @DisplayName("GET /status -> 200 y body plano")
        void statusOk() throws Exception {
            mockMvc.perform(get(BASE + "/status"))
                    .andExpect(status().isOk())
                    .andExpect(content().string("Servicio de catálogo de clientes disponible"));
        }

        @Test
        @DisplayName("Content-Type incorrecto -> 415 (unsupported media type)")
        void contentTypeIncorrecto() throws Exception {
            mockMvc.perform(post(BASE)
                            .contentType(MediaType.TEXT_PLAIN)
                            .content("texto"))
                                        .andExpect(status().isUnsupportedMediaType())
                                        .andExpect(jsonPath("$.errorCode").value("UNSUPPORTED_MEDIA_TYPE"));
        }

        @Test
        @DisplayName("JSON malformado -> 400 (parse error)")
        void jsonMalformado() throws Exception {
            mockMvc.perform(post(BASE)
                            .contentType(MediaType.APPLICATION_JSON)
                            .content("{malformed"))
                                        .andExpect(status().isBadRequest())
                                        .andExpect(jsonPath("$.errorCode").value("MALFORMED_JSON"));
        }

        @Test
        @DisplayName("Ruta inexistente -> 404 (NoResourceFound)")
        void rutaInexistente() throws Exception {
            mockMvc.perform(get(BASE + "/no/existe"))
                                        .andExpect(status().isNotFound())
                                        .andExpect(jsonPath("$.errorCode").value("RESOURCE_NOT_FOUND"));
        }
    }
}
