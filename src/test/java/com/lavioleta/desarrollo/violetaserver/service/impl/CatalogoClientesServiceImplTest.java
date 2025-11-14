package com.lavioleta.desarrollo.violetaserver.service.impl;

import com.lavioleta.desarrollo.violetaserver.dto.ClienteDetalleEcommerceDTO;
import com.lavioleta.desarrollo.violetaserver.dto.DatosCreditoDTO;
import com.lavioleta.desarrollo.violetaserver.dto.request.BajaClienteRequest;
import com.lavioleta.desarrollo.violetaserver.dto.request.ClienteRequest;
import com.lavioleta.desarrollo.violetaserver.dto.response.ClienteListResponse;
import com.lavioleta.desarrollo.violetaserver.dto.response.ClienteResponse;
import com.lavioleta.desarrollo.violetaserver.entity.ClienteDetalleEcommerce;
import com.lavioleta.desarrollo.violetaserver.entity.DatosCredito;
import com.lavioleta.desarrollo.violetaserver.repository.CatalogoClientesRepository;
import com.lavioleta.desarrollo.violetaserver.repository.ClienteDetalleEcommerceRepository;
import com.lavioleta.desarrollo.violetaserver.repository.DatosCreditoRepository;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.extension.ExtendWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.junit.jupiter.MockitoExtension;
import org.springframework.jdbc.core.simple.JdbcClient;

import java.math.BigDecimal;
import java.time.LocalDate;
import java.util.ArrayList;
import java.util.Optional;

import static org.assertj.core.api.Assertions.assertThat;
import static org.mockito.ArgumentMatchers.*;
import static org.mockito.Mockito.*;

/**
 * Tests unitarios para CatalogoClientesServiceImpl
 * Cubre los métodos principales del servicio con mocks de las dependencias
 */
@ExtendWith(MockitoExtension.class)
@DisplayName("Tests unitarios - CatalogoClientesServiceImpl")
class CatalogoClientesServiceImplTest {

    @Mock
    private CatalogoClientesRepository repository;

    @Mock
    private DatosCreditoRepository datosCreditoRepository;

    @Mock
    private ClienteDetalleEcommerceRepository clienteDetalleEcommerceRepository;

    @Mock
    private JdbcClient jdbcClient;

    @InjectMocks
    private CatalogoClientesServiceImpl service;

    private ClienteRequest clienteRequest;
    private BajaClienteRequest bajaRequest;
    private ClienteResponse.ClienteCompleto clienteCompleto;

    @BeforeEach
    void setUp() {
        clienteRequest = new ClienteRequest();
        clienteRequest.setOperacion("A");
        clienteRequest.setCliente("TEST001");
        clienteRequest.setNombre("Cliente Test");
        clienteRequest.setRsocial("Empresa Test SA");
        clienteRequest.setNomnegocio("Negocio Test");
        clienteRequest.setRfc("TES123456789");
        clienteRequest.setIdEmpresa(1);
        clienteRequest.setActivo(true);

        bajaRequest = new BajaClienteRequest();
        bajaRequest.setCliente("TEST001");
        bajaRequest.setMotivo("Test elimination");

        // Crear cliente completo mock
        clienteCompleto = mock(ClienteResponse.ClienteCompleto.class);
    }

    @Test
    @DisplayName("obtenerEmpresaPorSucursal - debe retornar ID de empresa para sucursal válida")
    void obtenerEmpresaPorSucursal_SucursalValida_RetornaIdEmpresa() {
        // Given
        String sucursal = "S1";
        Integer expectedEmpresa = 1;
        when(repository.obtenerEmpresaPorSucursal(sucursal)).thenReturn(expectedEmpresa);

        // When
        Integer resultado = service.obtenerEmpresaPorSucursal(sucursal);

        // Then
        assertThat(resultado).isEqualTo(expectedEmpresa);
        verify(repository).obtenerEmpresaPorSucursal(sucursal);
    }

    @Test
    @DisplayName("grabarCliente - alta exitosa debe retornar respuesta positiva")
    void grabarCliente_AltaExitosa_RetornaRespuestaPositiva() {
        // Given
        String sucursal = "S1";
        String codigoGenerado = "CLT001";
        
        when(repository.grabaCliente(any(ClienteRequest.class), eq(sucursal)))
                .thenReturn(codigoGenerado);
        when(repository.consultarCliente(eq(codigoGenerado), eq(1)))
                .thenReturn(Optional.of(clienteCompleto));

        // When
        ClienteResponse resultado = service.grabarCliente(clienteRequest, sucursal);

        // Then
        assertThat(resultado).isNotNull();
        assertThat(resultado.getSuccess()).isTrue();
        assertThat(resultado.getMessage()).contains("creado exitosamente");
        assertThat(resultado.getCliente()).isEqualTo(clienteCompleto);
        verify(repository).grabaCliente(any(ClienteRequest.class), eq(sucursal));
        verify(repository).consultarCliente(eq(codigoGenerado), eq(1));
    }

    @Test
    @DisplayName("grabarCliente - modificación exitosa debe retornar respuesta positiva")
    void grabarCliente_ModificacionExitosa_RetornaRespuestaPositiva() {
        // Given
        clienteRequest.setOperacion("M");
        String sucursal = "S1";
        String codigoCliente = "CLT001";
        
        when(repository.grabaCliente(any(ClienteRequest.class), eq(sucursal)))
                .thenReturn(codigoCliente);
        when(repository.consultarCliente(eq(codigoCliente), eq(1)))
                .thenReturn(Optional.of(clienteCompleto));

        // When
        ClienteResponse resultado = service.grabarCliente(clienteRequest, sucursal);

        // Then
        assertThat(resultado).isNotNull();
        assertThat(resultado.getSuccess()).isTrue();
        assertThat(resultado.getMessage()).contains("actualizado exitosamente");
        assertThat(resultado.getCliente()).isEqualTo(clienteCompleto);
        verify(repository).grabaCliente(any(ClienteRequest.class), eq(sucursal));
    }

    @Test
    @DisplayName("grabarCliente - error del repository debe retornar respuesta de error")
    void grabarCliente_ErrorRepository_RetornaRespuestaError() {
        // Given
        String sucursal = "S1";
        when(repository.grabaCliente(any(ClienteRequest.class), eq(sucursal)))
                .thenThrow(new RuntimeException("Error de base de datos"));

        // When
        ClienteResponse resultado = service.grabarCliente(clienteRequest, sucursal);

        // Then
        assertThat(resultado).isNotNull();
        assertThat(resultado.getSuccess()).isFalse();
        assertThat(resultado.getMessage()).contains("Error interno");
        verify(repository).grabaCliente(any(ClienteRequest.class), eq(sucursal));
    }

    @Test
    @DisplayName("eliminarCliente - debe procesar eliminación exitosamente")
    void eliminarCliente_ProcesaEliminacionExitosamente() {
        // Given
        doNothing().when(repository).bajaCliente(any(BajaClienteRequest.class));

        // When
        ClienteResponse resultado = service.eliminarCliente(bajaRequest);

        // Then
        assertThat(resultado).isNotNull();
        assertThat(resultado.getSuccess()).isTrue();
        assertThat(resultado.getMessage()).contains("eliminado exitosamente");
        verify(repository).bajaCliente(any(BajaClienteRequest.class));
    }

    @Test
    @DisplayName("eliminarCliente - error del repository debe retornar respuesta de error")
    void eliminarCliente_ErrorRepository_RetornaRespuestaError() {
        // Given
        doThrow(new RuntimeException("Error al eliminar"))
                .when(repository).bajaCliente(any(BajaClienteRequest.class));

        // When
        ClienteResponse resultado = service.eliminarCliente(bajaRequest);

        // Then
        assertThat(resultado).isNotNull();
        assertThat(resultado.getSuccess()).isFalse();
        assertThat(resultado.getMessage()).contains("Error interno");
        verify(repository).bajaCliente(any(BajaClienteRequest.class));
    }

    @Test
    @DisplayName("consultarCliente - debe retornar cliente cuando existe")
    void consultarCliente_ClienteExiste_RetornaCliente() {
        // Given
        String codigoCliente = "CLT001";
        Integer idEmpresa = 1;
        
        when(repository.consultarCliente(eq(codigoCliente), eq(idEmpresa)))
                .thenReturn(Optional.of(clienteCompleto));

        // When
        ClienteResponse resultado = service.consultarCliente(codigoCliente, idEmpresa);

        // Then
        assertThat(resultado).isNotNull();
        assertThat(resultado.getSuccess()).isTrue();
        assertThat(resultado.getMessage()).contains("consultado exitosamente");
        assertThat(resultado.getCliente()).isEqualTo(clienteCompleto);
        verify(repository).consultarCliente(eq(codigoCliente), eq(idEmpresa));
    }

    @Test
    @DisplayName("consultarCliente - debe retornar respuesta negativa cuando no existe")
    void consultarCliente_ClienteNoExiste_RetornaRespuestaNegativa() {
        // Given
        String codigoCliente = "CLT999";
        Integer idEmpresa = 1;
        
        when(repository.consultarCliente(eq(codigoCliente), eq(idEmpresa)))
                .thenReturn(Optional.empty());

        // When
        ClienteResponse resultado = service.consultarCliente(codigoCliente, idEmpresa);

        // Then
        assertThat(resultado).isNotNull();
        assertThat(resultado.getSuccess()).isFalse();
        assertThat(resultado.getMessage()).contains("no encontrado");
        assertThat(resultado.getCliente()).isNull();
        verify(repository).consultarCliente(eq(codigoCliente), eq(idEmpresa));
    }

    @Test
    @DisplayName("consultarCliente - error del repository debe retornar respuesta de error")
    void consultarCliente_ErrorRepository_RetornaRespuestaError() {
        // Given
        String codigoCliente = "CLT001";
        Integer idEmpresa = 1;
        
        when(repository.consultarCliente(eq(codigoCliente), eq(idEmpresa)))
                .thenThrow(new RuntimeException("Error de conexión"));

        // When
        ClienteResponse resultado = service.consultarCliente(codigoCliente, idEmpresa);

        // Then
        assertThat(resultado).isNotNull();
        assertThat(resultado.getSuccess()).isFalse();
        assertThat(resultado.getMessage()).contains("Error al consultar cliente");
        assertThat(resultado.getCliente()).isNull();
        verify(repository).consultarCliente(eq(codigoCliente), eq(idEmpresa));
    }

    @Test
    @DisplayName("existeCliente - debe retornar true cuando cliente existe")
    void existeCliente_ClienteExiste_RetornaTrue() {
        // Given
        String codigoCliente = "CLT001";
        
        // Mock JdbcClient para simular existencia del cliente
        JdbcClient.StatementSpec statementSpec = mock(JdbcClient.StatementSpec.class);
        JdbcClient.MappedQuerySpec<Long> mappedQuerySpec = mock(JdbcClient.MappedQuerySpec.class);
        
        when(jdbcClient.sql(anyString())).thenReturn(statementSpec);
        when(statementSpec.param(eq(codigoCliente))).thenReturn(statementSpec);
        when(statementSpec.query(Long.class)).thenReturn(mappedQuerySpec);
        when(mappedQuerySpec.single()).thenReturn(1L);

        // When
        boolean resultado = service.existeCliente(codigoCliente);

        // Then
        assertThat(resultado).isTrue();
        verify(jdbcClient).sql(anyString());
    }

    @Test
    @DisplayName("existeCliente - debe retornar false cuando cliente no existe")
    void existeCliente_ClienteNoExiste_RetornaFalse() {
        // Given
        String codigoCliente = "CLT999";
        
        // Mock JdbcClient para simular cliente no existente
        JdbcClient.StatementSpec statementSpec = mock(JdbcClient.StatementSpec.class);
        JdbcClient.MappedQuerySpec<Long> mappedQuerySpec = mock(JdbcClient.MappedQuerySpec.class);
        
        when(jdbcClient.sql(anyString())).thenReturn(statementSpec);
        when(statementSpec.param(eq(codigoCliente))).thenReturn(statementSpec);
        when(statementSpec.query(Long.class)).thenReturn(mappedQuerySpec);
        when(mappedQuerySpec.single()).thenReturn(0L);

        // When
        boolean resultado = service.existeCliente(codigoCliente);

        // Then
        assertThat(resultado).isFalse();
        verify(jdbcClient).sql(anyString());
    }

    @Test
    @DisplayName("obtenerDatosCredito - debe retornar datos de crédito cuando existen")
    void obtenerDatosCredito_DatosExisten_RetornaDatos() {
        // Given
        String idCliente = "CLT001";
        DatosCredito entity = new DatosCredito();
        entity.setCliente(idCliente);
        
        when(datosCreditoRepository.findById(eq(idCliente)))
                .thenReturn(Optional.of(entity));

        // When
        DatosCreditoDTO resultado = service.obtenerDatosCredito(idCliente);

        // Then
        assertThat(resultado).isNotNull();
        assertThat(resultado.getIdcliente()).isEqualTo(idCliente);
        verify(datosCreditoRepository).findById(eq(idCliente));
    }

    @Test
    @DisplayName("obtenerDatosCredito - debe retornar null cuando no existen datos")
    void obtenerDatosCredito_DatosNoExisten_RetornaNull() {
        // Given
        String idCliente = "CLT999";
        
        when(datosCreditoRepository.findById(eq(idCliente)))
                .thenReturn(Optional.empty());

        // When
        DatosCreditoDTO resultado = service.obtenerDatosCredito(idCliente);

        // Then
        assertThat(resultado).isNull();
        verify(datosCreditoRepository).findById(eq(idCliente));
    }

    @Test
    @DisplayName("guardarDatosCredito - debe guardar y retornar datos de crédito")
    void guardarDatosCredito_GuardaDatos() {
        // Given
        DatosCreditoDTO dto = new DatosCreditoDTO();
        dto.setIdcliente("CLT001");
        dto.setMontosol(new BigDecimal("15000.00"));
        dto.setFechasolicitud(LocalDate.now());
        
        DatosCredito savedEntity = new DatosCredito();
        savedEntity.setCliente("CLT001");
        
        when(datosCreditoRepository.save(any(DatosCredito.class)))
                .thenReturn(savedEntity);

        // When
        DatosCreditoDTO resultado = service.guardarDatosCredito(dto);

        // Then
        assertThat(resultado).isNotNull();
        assertThat(resultado.getIdcliente()).isEqualTo("CLT001");
        verify(datosCreditoRepository).save(any(DatosCredito.class));
    }

    @Test
    @DisplayName("obtenerDetalleEcommerce - debe retornar detalle cuando existe")
    void obtenerDetalleEcommerce_DetalleExiste_RetornaDetalle() {
        // Given
        String idCliente = "CLT001";
        ClienteDetalleEcommerce entity = new ClienteDetalleEcommerce();
        entity.setCliente(idCliente);
        entity.setMarketing(true);
        
        when(clienteDetalleEcommerceRepository.findById(eq(idCliente)))
                .thenReturn(Optional.of(entity));

        // When
        ClienteDetalleEcommerceDTO resultado = service.obtenerDetalleEcommerce(idCliente);

        // Then
        assertThat(resultado).isNotNull();
        assertThat(resultado.getIdcliente()).isEqualTo(idCliente);
        verify(clienteDetalleEcommerceRepository).findById(eq(idCliente));
    }

    @Test
    @DisplayName("obtenerDetalleEcommerce - debe retornar null cuando no existe")
    void obtenerDetalleEcommerce_DetalleNoExiste_RetornaNull() {
        // Given
        String idCliente = "CLT999";
        
        when(clienteDetalleEcommerceRepository.findById(eq(idCliente)))
                .thenReturn(Optional.empty());

        // When
        ClienteDetalleEcommerceDTO resultado = service.obtenerDetalleEcommerce(idCliente);

        // Then
        assertThat(resultado).isNull();
        verify(clienteDetalleEcommerceRepository).findById(eq(idCliente));
    }

    @Test
    @DisplayName("guardarDetalleEcommerce - debe guardar y retornar detalle")
    void guardarDetalleEcommerce_GuardaDetalle() {
        // Given
        ClienteDetalleEcommerceDTO dto = new ClienteDetalleEcommerceDTO();
        dto.setIdcliente("CLT001");
        dto.setMarketing(true);
        dto.setActivo(true);
        
        ClienteDetalleEcommerce savedEntity = new ClienteDetalleEcommerce();
        savedEntity.setCliente("CLT001");
        savedEntity.setMarketing(true);
        
        when(clienteDetalleEcommerceRepository.save(any(ClienteDetalleEcommerce.class)))
                .thenReturn(savedEntity);

        // When
        ClienteDetalleEcommerceDTO resultado = service.guardarDetalleEcommerce(dto);

        // Then
        assertThat(resultado).isNotNull();
        assertThat(resultado.getIdcliente()).isEqualTo("CLT001");
        verify(clienteDetalleEcommerceRepository).save(any(ClienteDetalleEcommerce.class));
    }

    @Test
    @DisplayName("listarClientes - debe retornar lista paginada")
    void listarClientes_RetornaListaPaginada() {
        // Given
        String filtros = "activo";
        Integer pagina = 1;
        Integer registrosPorPagina = 10;
        Integer idEmpresa = 1;
        
        // Mock JdbcClient para simular consulta SQL
        JdbcClient.StatementSpec statementSpec = mock(JdbcClient.StatementSpec.class);
        when(jdbcClient.sql(anyString())).thenReturn(statementSpec);
        when(statementSpec.param(any())).thenReturn(statementSpec);
        when(statementSpec.query(any(Class.class))).thenReturn(mock(JdbcClient.MappedQuerySpec.class));

        // When
        ClienteListResponse resultado = service.listarClientes(filtros, pagina, registrosPorPagina, idEmpresa);

        // Then
        assertThat(resultado).isNotNull();
        verify(jdbcClient).sql(anyString());
    }

    @Test
    @DisplayName("Validaciones de parámetros nulos")
    void validacionesParametrosNulos() {
        // Test obtenerEmpresaPorSucursal con parámetro nulo
        when(repository.obtenerEmpresaPorSucursal(null)).thenReturn(1);
        Integer resultado1 = service.obtenerEmpresaPorSucursal(null);
        assertThat(resultado1).isEqualTo(1);

        // Test consultarCliente con parámetros nulos maneja gracefully
        ClienteResponse resultado2 = service.consultarCliente(null, 1);
        assertThat(resultado2).isNotNull();
        assertThat(resultado2.getSuccess()).isFalse();
        
        ClienteResponse resultado3 = service.consultarCliente("CLT001", null);
        assertThat(resultado3).isNotNull();
        assertThat(resultado3.getSuccess()).isFalse();
    }
}