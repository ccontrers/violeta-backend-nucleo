package com.lavioleta.desarrollo.violetaserver.service;

import com.lavioleta.desarrollo.violetaserver.dto.request.BusquedaProveedoresRequest;
import com.lavioleta.desarrollo.violetaserver.dto.response.BusquedaProveedoresResponse;
import com.lavioleta.desarrollo.violetaserver.dto.response.BusquedaProveedoresResponse.ProveedorResultado;
import com.lavioleta.desarrollo.violetaserver.repository.BusquedaProveedoresRepository;
import com.lavioleta.desarrollo.violetaserver.service.impl.BusquedaProveedoresServiceImpl;
import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.extension.ExtendWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.junit.jupiter.MockitoExtension;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import static org.assertj.core.api.Assertions.assertThat;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

/**
 * Tests unitarios para BusquedaProveedoresService.
 * Valida lógica de negocio, transformaciones DTO y manejo de casos edge.
 */
@ExtendWith(MockitoExtension.class)
@DisplayName("Service Layer: BusquedaProveedoresService")
class BusquedaProveedoresServiceTest {

    @Mock
    private BusquedaProveedoresRepository repository;

    @InjectMocks
    private BusquedaProveedoresServiceImpl service;

    @Nested
    @DisplayName("Búsqueda por Razón Social (RSO)")
    class BusquedaPorRazonSocialTests {

        @Test
        @DisplayName("Debería encontrar proveedores por razón social básica")
        void buscarProveedores_deberiaEncontrarPorRazonSocialBasica() {
            // ARRANGE
            BusquedaProveedoresRequest request = BusquedaProveedoresRequest.builder()
                .codcondicion("RSO")
                .condicion("ACME")
                .build();

            List<ProveedorResultado> proveedoresMock = crearProveedoresResultadoMock();
            when(repository.buscarPorRazonSocial("ACME", "activo = 1 AND ", "", "", 50))
                .thenReturn(proveedoresMock);

            // ACT
            BusquedaProveedoresResponse response = service.buscarProveedores(request);

            // ASSERT
            assertThat(response.isSuccess()).isTrue();
            assertThat(response.getTotalResultados()).isEqualTo(2);
            assertThat(response.getProveedores()).hasSize(2);
            
            verify(repository).buscarPorRazonSocial("ACME", "activo = 1 AND ", "", "", 50);
        }

        @Test
        @DisplayName("Debería aplicar filtro mostrar inactivos correctamente")
        void buscarProveedores_deberiaAplicarFiltroMostrarInactivosCorrectamente() {
            // ARRANGE
            BusquedaProveedoresRequest request = BusquedaProveedoresRequest.builder()
                .codcondicion("RSO")
                .condicion("ACME")
                .mostrarInactivos(true)
                .build();

            List<ProveedorResultado> proveedoresMock = crearProveedoresResultadoMock();
            when(repository.buscarPorRazonSocial("ACME", "", "", "", 50))
                .thenReturn(proveedoresMock);

            // ACT
            BusquedaProveedoresResponse response = service.buscarProveedores(request);

            // ASSERT
            assertThat(response.isSuccess()).isTrue();
            verify(repository).buscarPorRazonSocial("ACME", "", "", "", 50);
        }

        @Test
        @DisplayName("Debería aplicar filtro solo proveedor gastos correctamente")
        void buscarProveedores_deberiaAplicarFiltroSoloProveedorGastosCorrectamente() {
            // ARRANGE
            BusquedaProveedoresRequest request = BusquedaProveedoresRequest.builder()
                .codcondicion("RSO")
                .condicion("ACME")
                .soloProveedorGastos(true)
                .build();

            List<ProveedorResultado> proveedoresMock = crearProveedoresResultadoMock();
            when(repository.buscarPorRazonSocial("ACME", "activo = 1 AND ", "AND provgastos = 1 ", "", 50))
                .thenReturn(proveedoresMock);

            // ACT
            BusquedaProveedoresResponse response = service.buscarProveedores(request);

            // ASSERT
            assertThat(response.isSuccess()).isTrue();
            verify(repository).buscarPorRazonSocial("ACME", "activo = 1 AND ", "AND provgastos = 1 ", "", 50);
        }

        @Test
        @DisplayName("Debería validar exclusión mutua de filtros")
        void buscarProveedores_deberiaValidarExclusionMutuaFiltros() {
            // ARRANGE
            BusquedaProveedoresRequest request = BusquedaProveedoresRequest.builder()
                .codcondicion("RSO")
                .condicion("ACME")
                .soloProveedorGastos(true)
                .soloProveedorMercancia(true) // Ambos marcados - error
                .build();

            // ACT
            BusquedaProveedoresResponse response = service.buscarProveedores(request);

            // ASSERT
            assertThat(response.isSuccess()).isFalse();
            assertThat(response.getMessage()).contains("filtrar por gastos y mercancía simultáneamente");
        }

        @Test
        @DisplayName("Debería manejar búsqueda sin resultados")
        void buscarProveedores_deberiaManejarBusquedaSinResultados() {
            // ARRANGE
            BusquedaProveedoresRequest request = BusquedaProveedoresRequest.builder()
                .codcondicion("RSO")
                .condicion("NoExiste")
                .build();

            when(repository.buscarPorRazonSocial("NoExiste", "activo = 1 AND ", "", "", 50))
                .thenReturn(Collections.emptyList());

            // ACT
            BusquedaProveedoresResponse response = service.buscarProveedores(request);

            // ASSERT
            assertThat(response.isSuccess()).isTrue();
            assertThat(response.getTotalResultados()).isEqualTo(0);
            assertThat(response.getProveedores()).isEmpty();
            assertThat(response.getMessage()).contains("No se encontraron proveedores");
        }
    }

    @Nested
    @DisplayName("Búsqueda por RFC")
    class BusquedaPorRfcTests {

        @Test
        @DisplayName("Debería buscar correctamente por RFC")
        void buscarProveedores_deberiaBuscarCorrectamentePorRfc() {
            // ARRANGE
            BusquedaProveedoresRequest request = BusquedaProveedoresRequest.builder()
                .codcondicion("RFC")
                .condicion("ACM123456789")
                .build();

            List<ProveedorResultado> proveedoresMock = crearProveedoresResultadoMock();
            when(repository.buscarPorRfc("ACM123456789", "activo = 1 AND ", "", "", 50))
                .thenReturn(proveedoresMock);

            // ACT
            BusquedaProveedoresResponse response = service.buscarProveedores(request);

            // ASSERT
            assertThat(response.isSuccess()).isTrue();
            assertThat(response.getTotalResultados()).isEqualTo(2);
            verify(repository).buscarPorRfc("ACM123456789", "activo = 1 AND ", "", "", 50);
        }
    }

    @Nested
    @DisplayName("Búsqueda por Clave")
    class BusquedaPorClaveTests {

        @Test
        @DisplayName("Debería buscar correctamente por clave")
        void buscarProveedores_deberiaBuscarCorrectamentePorClave() {
            // ARRANGE
            BusquedaProveedoresRequest request = BusquedaProveedoresRequest.builder()
                .codcondicion("CLA")
                .condicion("PROV001")
                .build();

            List<ProveedorResultado> proveedoresMock = crearProveedoresResultadoMock();
            when(repository.buscarPorClave("PROV001", "activo = 1 AND ", "", "", 50))
                .thenReturn(proveedoresMock);

            // ACT
            BusquedaProveedoresResponse response = service.buscarProveedores(request);

            // ASSERT
            assertThat(response.isSuccess()).isTrue();
            assertThat(response.getTotalResultados()).isEqualTo(2);
            verify(repository).buscarPorClave("PROV001", "activo = 1 AND ", "", "", 50);
        }
    }

    @Nested
    @DisplayName("Búsqueda por Representante Legal")
    class BusquedaPorRepresentanteLegalTests {

        @Test
        @DisplayName("Debería buscar correctamente por representante legal")
        void buscarProveedores_deberiaBuscarCorrectamentePorRepresentanteLegal() {
            // ARRANGE
            BusquedaProveedoresRequest request = BusquedaProveedoresRequest.builder()
                .codcondicion("REP")
                .condicion("Juan Pérez")
                .build();

            List<ProveedorResultado> proveedoresMock = crearProveedoresResultadoMock();
            when(repository.buscarPorRepresentanteLegal("Juan Pérez", "activo = 1 AND ", "", "", 50))
                .thenReturn(proveedoresMock);

            // ACT
            BusquedaProveedoresResponse response = service.buscarProveedores(request);

            // ASSERT
            assertThat(response.isSuccess()).isTrue();
            assertThat(response.getTotalResultados()).isEqualTo(2);
            verify(repository).buscarPorRepresentanteLegal("Juan Pérez", "activo = 1 AND ", "", "", 50);
        }
    }

    @Nested
    @DisplayName("Validaciones y Casos Edge")
    class ValidacionesCasosEdge {

        @Test
        @DisplayName("Debería manejar código de condición inválido")
        void buscarProveedores_deberiaManejarCodigoCondicionInvalido() {
            // ARRANGE
            BusquedaProveedoresRequest request = BusquedaProveedoresRequest.builder()
                .codcondicion("INVALIDO")
                .condicion("ACME")
                .build();

            // ACT
            BusquedaProveedoresResponse response = service.buscarProveedores(request);

            // ASSERT
            assertThat(response.isSuccess()).isFalse();
            assertThat(response.getMessage()).contains("Tipo de búsqueda no válido");
        }

        @Test
        @DisplayName("Debería respetar límite de filas personalizado")
        void buscarProveedores_deberiaRespetarLimiteFilasPersonalizado() {
            // ARRANGE
            BusquedaProveedoresRequest request = BusquedaProveedoresRequest.builder()
                .codcondicion("RSO")
                .condicion("ACME")
                .filas("10")
                .build();

            List<ProveedorResultado> proveedoresMock = crearProveedoresResultadoMock();
            when(repository.buscarPorRazonSocial("ACME", "activo = 1 AND ", "", "", 10))
                .thenReturn(proveedoresMock);

            // ACT
            BusquedaProveedoresResponse response = service.buscarProveedores(request);

            // ASSERT
            assertThat(response.isSuccess()).isTrue();
            verify(repository).buscarPorRazonSocial("ACME", "activo = 1 AND ", "", "", 10);
        }

        @Test
        @DisplayName("Debería manejar condición null o vacía")
        void buscarProveedores_deberiaManejarCondicionNullOVacia() {
            // ARRANGE - condición null
            BusquedaProveedoresRequest request = BusquedaProveedoresRequest.builder()
                .codcondicion("RSO")
                .condicion(null)
                .build();

            // ACT
            BusquedaProveedoresResponse response = service.buscarProveedores(request);

            // ASSERT
            assertThat(response).isNotNull();
            assertThat(response.isSuccess()).isTrue();
            assertThat(response.getProveedores()).isEmpty(); // No ejecuta búsqueda
        }
    }

    // Métodos auxiliares
    private List<ProveedorResultado> crearProveedoresResultadoMock() {
        return Arrays.asList(
            crearProveedorResultadoMock("PROV001", "ACME Corporation S.A. de C.V.", "ACM123456789", true),
            crearProveedorResultadoMock("PROV002", "Distribuidora Beta S.A.", "BET987654321", true)
        );
    }

    private ProveedorResultado crearProveedorResultadoMock(String codigo, String razonSocial, String rfc, boolean activo) {
        return ProveedorResultado.builder()
                .proveedor(codigo)
                .razonsocial(razonSocial)
                .rfc(rfc)
                .estado("CDMX")
                .localidad("Ciudad de México")
                .calle("Av. Test 123")
                .colonia("Colonia Test")
                .activo(activo)
                .provgastos(true)
                .provmercancia(false)
                .redondeocptecho(false)
                .contacto("Juan Pérez")
                .email("test@example.com")
                .build();
    }
}