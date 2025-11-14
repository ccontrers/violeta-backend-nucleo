package com.lavioleta.desarrollo.violetaserver.dto.request;

import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;

import jakarta.validation.ConstraintViolation;
import jakarta.validation.Validation;
import jakarta.validation.Validator;
import jakarta.validation.ValidatorFactory;
import java.util.Set;

import static org.assertj.core.api.Assertions.assertThat;

/**
 * Tests de validación para BusquedaProveedoresRequest DTO.
 * Verifica validaciones Bean Validation y reglas de negocio.
 */
@DisplayName("DTO Validation: BusquedaProveedoresRequest")
class BusquedaProveedoresRequestValidationTest {

    private final ValidatorFactory factory = Validation.buildDefaultValidatorFactory();
    private final Validator validator = factory.getValidator();

    @Nested
    @DisplayName("Validaciones de Campo Requerido")
    class ValidacionesCampoRequeridoTests {

        @Test
        @DisplayName("Debería fallar cuando codcondicion es null")
        void validation_deberiaFallarCuandoCodcondicionEsNull() {
            // ARRANGE
            BusquedaProveedoresRequest request = BusquedaProveedoresRequest.builder()
                .codcondicion(null) // NULL no permitido
                .condicion("ACME")
                .build();

            // ACT
            Set<ConstraintViolation<BusquedaProveedoresRequest>> violations = validator.validate(request);

            // ASSERT
            assertThat(violations).hasSize(1);
            assertThat(violations)
                .extracting(ConstraintViolation::getMessage)
                .contains("Tipo de búsqueda es requerido");
        }

        @Test
        @DisplayName("Debería aceptar condicion null")
        void validation_deberiaAceptarCondicionNull() {
            // ARRANGE
            BusquedaProveedoresRequest request = BusquedaProveedoresRequest.builder()
                .codcondicion("RSO")
                .condicion(null) // NULL permitido según DTO actual
                .build();

            // ACT
            Set<ConstraintViolation<BusquedaProveedoresRequest>> violations = validator.validate(request);

            // ASSERT
            assertThat(violations).isEmpty(); // No hay validación @NotBlank en condicion
        }
    }

    @Nested
    @DisplayName("Validaciones de Código de Condición")
    class ValidacionesCodigoCondicionTests {

        @Test
        @DisplayName("Debería aceptar códigos válidos")
        void validation_deberiaAceptarCodigosValidos() {
            String[] codigosValidos = {"RSO", "RFC", "CLA", "REP"};

            for (String codigo : codigosValidos) {
                // ARRANGE
                BusquedaProveedoresRequest request = BusquedaProveedoresRequest.builder()
                    .codcondicion(codigo)
                    .condicion("ACME")
                    .filas("50")
                    .build();

                // ACT
                Set<ConstraintViolation<BusquedaProveedoresRequest>> violations = validator.validate(request);

                // ASSERT
                assertThat(violations)
                    .withFailMessage("Falló validación para código: '" + codigo + "'")
                    .isEmpty();
            }
        }

        @Test
        @DisplayName("Debería fallar con código inválido")
        void validation_deberiaFallarConCodigoInvalido() {
            // ARRANGE
            BusquedaProveedoresRequest request = BusquedaProveedoresRequest.builder()
                .codcondicion("INVALIDO")
                .condicion("ACME")
                .filas("50")
                .build();

            // ACT
            Set<ConstraintViolation<BusquedaProveedoresRequest>> violations = validator.validate(request);

            // ASSERT
            assertThat(violations).hasSize(1);
            assertThat(violations)
                .extracting(ConstraintViolation::getMessage)
                .contains("Tipo de búsqueda inválido. Valores permitidos: RSO, RFC, CLA, REP o vacío");
        }
    }

    @Nested
    @DisplayName("Validaciones de Longitud")
    class ValidacionesLongitudTests {

        @Test
        @DisplayName("Debería aceptar condición hasta 100 caracteres")
        void validation_deberiaAceptarCondicionValida() {
            // ARRANGE
            String condicionLarga = "A".repeat(100); // Exactamente 100 caracteres
            BusquedaProveedoresRequest request = BusquedaProveedoresRequest.builder()
                .codcondicion("RSO")
                .condicion(condicionLarga)
                .filas("50")
                .build();

            // ACT
            Set<ConstraintViolation<BusquedaProveedoresRequest>> violations = validator.validate(request);

            // ASSERT
            assertThat(violations).isEmpty();
        }

        @Test
        @DisplayName("Debería fallar con condición mayor a 100 caracteres")
        void validation_deberiaFallarConCondicionMuyLarga() {
            // ARRANGE
            String condicionMuyLarga = "A".repeat(101); // 101 caracteres, excede límite
            BusquedaProveedoresRequest request = BusquedaProveedoresRequest.builder()
                .codcondicion("RSO")
                .condicion(condicionMuyLarga)
                .filas("50")
                .build();

            // ACT
            Set<ConstraintViolation<BusquedaProveedoresRequest>> violations = validator.validate(request);

            // ASSERT
            assertThat(violations).hasSize(1);
            assertThat(violations)
                .extracting(ConstraintViolation::getMessage)
                .contains("La condición no puede exceder 100 caracteres");
        }
    }

    @Nested
    @DisplayName("Validaciones de Campo Filas")
    class ValidacionesFilasTests {

        @Test
        @DisplayName("Debería aceptar números válidos")
        void validation_deberiaAceptarNumerosValidos() {
            String[] filasValidas = {"1", "50", "999", "1000"};

            for (String filas : filasValidas) {
                // ARRANGE
                BusquedaProveedoresRequest request = BusquedaProveedoresRequest.builder()
                    .codcondicion("RSO")
                    .condicion("ACME")
                    .filas(filas)
                    .build();

                // ACT
                Set<ConstraintViolation<BusquedaProveedoresRequest>> violations = validator.validate(request);

                // ASSERT
                assertThat(violations)
                    .withFailMessage("Falló validación para filas: '" + filas + "'")
                    .isEmpty();
            }
        }

        @Test
        @DisplayName("Debería fallar con valores inválidos")
        void validation_deberiaFallarConValoresInvalidos() {
            String[] filasInvalidas = {"0", "abc", "-10", "50!", "01", ""};

            for (String filas : filasInvalidas) {
                // ARRANGE
                BusquedaProveedoresRequest request = BusquedaProveedoresRequest.builder()
                    .codcondicion("RSO")
                    .condicion("ACME")
                    .filas(filas)
                    .build();

                // ACT
                Set<ConstraintViolation<BusquedaProveedoresRequest>> violations = validator.validate(request);

                // ASSERT
                assertThat(violations)
                    .withFailMessage("Debería fallar para filas: '" + filas + "'")
                    .hasSizeGreaterThan(0);
                assertThat(violations)
                    .extracting(ConstraintViolation::getMessage)
                    .contains("El número de filas debe ser un entero positivo");
            }
        }
    }

    @Nested
    @DisplayName("Casos Combinados")
    class CasosCombinados {

        @Test
        @DisplayName("Debería validar request completamente válido")
        void validation_deberiaValidarRequestCompleto() {
            // ARRANGE
            BusquedaProveedoresRequest request = BusquedaProveedoresRequest.builder()
                .codcondicion("RSO")
                .condicion("ACME Corporation")
                .mostrarInactivos(true)
                .soloProveedorGastos(true)
                .soloProveedorMercancia(false)
                .filas("25")
                .build();

            // ACT
            Set<ConstraintViolation<BusquedaProveedoresRequest>> violations = validator.validate(request);

            // ASSERT
            assertThat(violations).isEmpty();
        }

        @Test
        @DisplayName("Debería acumular múltiples violaciones")
        void validation_deberiaAcumularMultiplesViolaciones() {
            // ARRANGE
            BusquedaProveedoresRequest request = BusquedaProveedoresRequest.builder()
                .codcondicion("INVALIDO")
                .condicion("A".repeat(101)) // Excede 100 caracteres
                .filas("abc") // No numérico
                .build();

            // ACT
            Set<ConstraintViolation<BusquedaProveedoresRequest>> violations = validator.validate(request);

            // ASSERT
            assertThat(violations).hasSize(3);
            assertThat(violations)
                .extracting(ConstraintViolation::getMessage)
                .containsExactlyInAnyOrder(
                    "Tipo de búsqueda inválido. Valores permitidos: RSO, RFC, CLA, REP o vacío",
                    "La condición no puede exceder 100 caracteres",
                    "El número de filas debe ser un entero positivo"
                );
        }

        @Test
        @DisplayName("Debería usar valores por defecto correctos")
        void validation_deberiaUsarValoresPorDefecto() {
            // ARRANGE
            BusquedaProveedoresRequest request = BusquedaProveedoresRequest.builder()
                .codcondicion("RSO")
                .condicion("ACME")
                .build(); // Solo campos requeridos, otros usan defaults

            // ACT
            Set<ConstraintViolation<BusquedaProveedoresRequest>> violations = validator.validate(request);

            // ASSERT
            assertThat(violations).isEmpty();
            assertThat(request.getMostrarInactivos()).isFalse();
            assertThat(request.getSoloProveedorGastos()).isFalse();
            assertThat(request.getSoloProveedorMercancia()).isFalse();
            assertThat(request.getFilas()).isEqualTo("50");
        }
    }
}