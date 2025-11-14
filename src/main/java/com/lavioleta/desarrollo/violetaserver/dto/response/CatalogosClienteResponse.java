package com.lavioleta.desarrollo.violetaserver.dto.response;

import lombok.Data;
import lombok.Builder;
import java.util.List;

/**
 * DTOs para catálogos de apoyo del formulario de clientes
 */
public class CatalogosClienteResponse {
    
    @Data
    @Builder
    public static class EmpresasResponse {
        private Boolean success;
        private String message;
        private List<Empresa> empresas;
        
        @Data
        @Builder
        public static class Empresa {
            private Integer idempresa;
            private String clave;
            private String nombre;
            private String sucprincipal;
            private Boolean essuper;
        }
    }
    
    @Data
    @Builder
    public static class GirosNegocioResponse {
        private Boolean success;
        private String message;
        private List<GiroNegocio> giros;
        
        @Data
        @Builder
        public static class GiroNegocio {
            private String giro;
            private String nombre;
            private Boolean activo;
        }
    }
    
    @Data
    @Builder
    public static class CanalesClienteResponse {
        private Boolean success;
        private String message;
        private List<CanalCliente> canales;
        
        @Data
        @Builder
        public static class CanalCliente {
            private String canal;
            private String nombre;
            private Boolean activo;
        }
    }
    
    @Data
    @Builder
    public static class RegimenesFiscalesResponse {
        private Boolean success;
        private String message;
        private List<RegimenFiscal> regimenes;
        
        @Data
        @Builder
        public static class RegimenFiscal {
            private String clave;
            private String descripcion;
            private Boolean activo;
        }
    }
    
    @Data
    @Builder
    public static class SociedadesMercantilesResponse {
        private Boolean success;
        private String message;
        private List<SociedadMercantil> sociedades;
        
        @Data
        @Builder
        public static class SociedadMercantil {
            private String clave;
            private String descripcion;
            private Boolean activo;
        }
    }
    
    // FormasPagoResponse eliminado; limpieza de anotaciones duplicadas
    public static class UsosCFDIResponse {
        private Boolean success;
        private String message;
        private List<UsoCFDI> usos;
        
        @Data
        @Builder
        public static class UsoCFDI {
            private String clave;
            private String descripcion;
            private Boolean activo;
        }
    }
    
    @Data
    @Builder
    public static class ColoniasResponse {
        private Boolean success;
        private String message;
        private List<Colonia> colonias;
        
        @Data
        @Builder
        public static class Colonia {
            private String colonia;
            private String nombre;
            private String cp;
            private String municipio;
            private String estado;
        }
    }
    
    @Data
    @Builder
    public static class VendedoresResponse {
        private Boolean success;
        private String message;
        private List<Vendedor> vendedores;
        
        @Data
        @Builder
        public static class Vendedor {
            private String empleado;
            private String nombre;
            private Boolean activo;
        }
    }
    
    @Data
    @Builder
    public static class CobradoresResponse {
        private Boolean success;
        private String message;
        private List<Cobrador> cobradores;
        
        @Data
        @Builder
        public static class Cobrador {
            private String empleado;
            private String nombre;
            private Boolean activo;
        }
    }
    
    /**
     * Respuesta que incluye todos los catálogos necesarios
     */
    @Data
    @Builder
    public static class CatalogosCompletos {
        private Boolean success;
        private String message;
        private EmpresasResponse.Empresa empresaActual;
        private List<EmpresasResponse.Empresa> empresas;
        private List<GirosNegocioResponse.GiroNegocio> giros;
        private List<CanalesClienteResponse.CanalCliente> canales;
        private List<RegimenesFiscalesResponse.RegimenFiscal> regimenes;
        private List<SociedadesMercantilesResponse.SociedadMercantil> sociedades;
        private List<UsosCFDIResponse.UsoCFDI> usosCFDI;
        private List<VendedoresResponse.Vendedor> vendedores;
        private List<CobradoresResponse.Cobrador> cobradores;
    }
}
