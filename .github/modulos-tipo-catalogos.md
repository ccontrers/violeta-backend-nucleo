# Módulos tipo Catálogos# Módulos tipo — Catálogos (patrón)



Guía para migrar módulos de catálogos CRUD desde el sistema legado C++ Builder a Spring Boot + React, basada en la migración exitosa del **Catálogo de Clientes**.## Estructura recomendada

- DTOs (request/response)

## Introducción- Repository (consultas SQL)

- Service / ServiceImpl (lógica + transacciones)

Los **módulos de catálogo** en el sistema legacy son formularios CRUD (Create, Read, Update, Delete) que permiten gestionar maestros de datos. Se caracterizan por:- Controller (endpoints REST)



- **Formularios multi-pestaña** con datos organizados en secciones temáticas## Tablas típicas (ejemplo Catálogo Clientes)

- **Múltiples tablas relacionadas** (principal + empresas + detalles)- `clientes` (principal)

- **Validaciones complejas** de integridad referencial y reglas de negocio- `clientesemp` (datos por empresa)

- **Catálogos dependientes** (combos que se cargan dinámicamente)- `clientesdetalleecommerce`

- **Operaciones transaccionales** que afectan múltiples tablas- `datoscredito`

- `direccionesentregaclientes`

La migración del **Catálogo de Clientes** (6 tablas + 15 catálogos dependientes) es el ejemplo de referencia exitoso que debe replicarse para otros catálogos como Artículos, Proveedores, Vendedores, etc.- `telefonosclientes`

- Catálogos dependientes: `canalesclientes`, `gironegocio`, `cregimenfiscal`, etc.

---

## Patrones críticos

## Interpretación del Código Legado- JOINs en consulta base para evitar múltiples llamadas.

- Datos por empresa: upsert en `clientesemp` con defaults.

### Backend Legacy (ClassServidorCatalogos.cpp)- Validar NOT NULL antes de commit.



#### **Identificación de Patrones CRUD**## Checklist al implementar un catálogo

```cpp- DTOs tempranos

// Patrón típico en ClassServidorCatalogos.cpp- Repositorio con QueryBuilder

class ClassServidorCatalogos {- Tests unitarios del repositorio y service

    // IDs de operación por entidad- Validaciones en DTOs y Controller

    static const int ID_GRA_CLIENTE = 1001;  // Grabar (Alta/Modificación)

    static const int ID_BAJ_CLIENTE = 1002;  // Baja (Eliminación)## Documentación relacionada

    static const int ID_CON_CLIENTE = 1003;  // Consultar individual- `docs/api-busqueda-articulos.md`

    static const int ID_LIS_CLIENTE = 1004;  // Listar con filtros

    ## Nota de refactor (Sept 2025)

    // Método principal de procesamientoSe eliminó del frontend el campo "Forma de Pago por Defecto" y el catálogo asociado (`/catalogos/formas-pago`).

    void ProcesarSolicitud(ArregloTransacciones& solicitud);

    Motivos:

    // Métodos específicos por operación- Evitar confusión con los campos CFDI reales `metododef` / `metodosup` (método de pago SAT PUE/PPD).

    void GrabarCliente(ClienteData& datos);- `metododef` en BD ya cumple la función requerida para CFDI y se mantendrá.

    void BajaCliente(string codigoCliente);- Reducción de complejidad y reconciliaciones innecesarias en el formulario de clientes.

    void ConsultarCliente(string codigoCliente);

    void ListarClientes(FiltrosLista& filtros);Impacto:

};- Endpoint `/api/v1/catalogos/formas-pago` removido.

```- Eliminadas clases DTO `FormasPagoResponse` y el campo `formasPago` en `CatalogosCompletos`.

- Persisten sólo `metododef` y `metodosup` para emisión CFDI.

#### **Elementos Clave a Migrar**

- **Switch de operaciones** según código de transacción (A/M/B/C)Si en el futuro se requiere nuevamente un catálogo de formas de pago, reintroducir alias sobre la tabla `cformapago` y exponerlo con un endpoint separado sin mezclarlo con método de pago.
- **Transacciones multi-tabla** coordinadas
- **Validaciones de integridad** referencial (FK constraints)
- **Manejo de códigos automáticos** con secuencias personalizadas

### Frontend Legacy (FormCatalogo*.cpp/.dfm)

#### **Estructura UI Típica**
```cpp
// Patrón UI en FormCatalogoClientes.dfm
class TFormCatalogoClientes : public TForm {
    // Control principal de navegación
    TFrameNavegacion *BarraClientes;        // Botones CRUD
    
    // Pestañas organizacionales
    TPageControl *PageControlClientes;
    TTabSheet *PagClientesGeneral;          // Datos básicos
    TTabSheet *PagClientesCredito;          // Datos específicos
    TTabSheet *PagClientesContacto;         // Información adicional
    
    // Controles de datos básicos
    VTLabeledEdit *EditCodigoCliente;       // Clave (PK)
    VTRadioGroup *RadioGroupTipoEmpresa;    // F=Física/M=Moral
    VTLabeledEdit *EditNombreCliente;       // Campos principales
    
    // Combos de catálogos dependientes
    VTComboBox *ComboBoxGiroNegocio;        // Cargado dinámicamente
    VTComboBox *ComboBoxRegimenFiscal;      // Filtrado por empresa
    VTComboBox *ComboBoxCanal;              // Con validaciones
};
```

#### **Lógica CRUD Legacy**
```cpp
void TFormCatalogoClientes::BarraClientesButtonGrabarClick(TObject *Sender) {
    // Validaciones previas
    if (!ValidarFormulario()) return;
    
    // Construir estructura de datos
    ClienteData cliente;
    cliente.operacion = (modoEdicion) ? "M" : "A";
    cliente.codigo = EditCodigoCliente->Text;
    cliente.nombre = EditNombreCliente->Text;
    // ... llenar campos
    
    // Enviar al servidor con ID_GRA_CLIENTE
    servidor->GrabarCliente(cliente);
}

void TFormCatalogoClientes::CargarCatalogos() {
    // Llenar combos dependientes
    servidor->ObtenerGiros(ComboBoxGiroNegocio);
    servidor->ObtenerRegimenes(ComboBoxRegimenFiscal);
    servidor->ObtenerCanales(ComboBoxCanal);
}
```

---

## Patrón Común de Migración de Catálogos

### 1. Análisis del Código Legacy

#### **Identificar Operaciones CRUD**
```cpp
// En ClassServidorCatalogos.cpp buscar:
- Constantes ID_GRA_*, ID_BAJ_*, ID_CON_*, ID_LIS_*
- Métodos Grabar*(), Baja*(), Consultar*(), Listar*()
- Validaciones de integridad y reglas de negocio
- Transacciones multi-tabla coordinadas
```

#### **Mapear Estructura de Datos**
```cpp
// Identificar tablas involucradas:
- Tabla principal (ej: clientes) 
- Tablas empresa (ej: clientesemp) - CRÍTICAS
- Tablas relacionadas (ej: datoscredito, direcciones)
- Catálogos dependientes (ej: giros, regimenes)
```

#### **Analizar Validaciones**
```cpp
// Validaciones comunes en catálogos:
- Códigos únicos por empresa
- Referencias FK válidas 
- Reglas de negocio específicas (RFC válido, etc.)
- Estados de activo/inactivo
```

### 2. Diseño de DTOs Multi-Entidad

#### **Request DTO - Estructura Jerárquica**
```java
@Data
@Builder
public class CatalogoRequest {
    // Operación CRUD
    @NotBlank(message = "Operación es requerida")
    @Pattern(regexp = "^[AMB]$", message = "Operación debe ser A, M o B")
    private String operacion;
    
    // Clave principal (se genera automático en Alta)
    @Size(max = 10, message = "Código muy largo")
    private String codigo;
    
    // ID empresa para datos específicos por empresa
    @NotNull(message = "ID empresa es requerido")
    private Integer idEmpresa;
    
    // === DATOS PRINCIPALES (tabla principal) ===
    @NotBlank(message = "Nombre es requerido")
    @Size(max = 60, message = "Nombre muy largo")
    private String nombre;
    
    @Size(max = 100, message = "Descripción muy larga")
    private String descripcion;
    
    @Builder.Default
    private Boolean activo = true;
    
    // === DATOS ESPECÍFICOS POR EMPRESA ===
    @Valid
    private List<DatoEmpresaRequest> datosEmpresas;
    
    // === DATOS RELACIONADOS (tablas satélite) ===
    @Valid
    private List<DetalleRequest> detalles;
}
```

#### **Response DTO - Estructura Anidada**
```java
@Data
@Builder
public class CatalogoResponse {
    // Status operacional
    private boolean success;
    private String message;
    private String codigoGenerado; // Para operaciones de alta
    
    // Datos completos del registro
    private DatosCompletos datos;
    
    // Catálogos dependientes para UI
    private List<CatalogoDto> giros;
    private List<CatalogoDto> regimenes;
    private List<CatalogoDto> canales;
    
    @Data
    @Builder
    public static class DatosCompletos {
        // Datos principales
        private String codigo;
        private String nombre;
        private String descripcion;
        private Boolean activo;
        
        // Datos por empresa (array)
        private List<DatoEmpresa> datosEmpresas;
        
        // Datos relacionados
        private List<Detalle> detalles;
    }
}
```

### 3. Repository - Patrón Multi-Tabla

#### **Operaciones CRUD Transaccionales**
```java
@Repository
public class CatalogoRepository {
    
    @Transactional
    public CatalogoResponse grabar(CatalogoRequest request) {
        String operacion = request.getOperacion();
        
        return switch (operacion) {
            case "A" -> insertarRegistro(request);
            case "M" -> actualizarRegistro(request);
            default -> throw new IllegalArgumentException("Operación inválida: " + operacion);
        };
    }
    
    private CatalogoResponse insertarRegistro(CatalogoRequest request) {
        try {
            // 1. Generar código automático
            String codigoGenerado = generarCodigoAutomatico();
            
            // 2. Insertar tabla principal
            insertarTablaPrincipal(codigoGenerado, request);
            
            // 3. Insertar datos por empresa (CRÍTICO)
            insertarDatosEmpresa(codigoGenerado, request.getDatosEmpresas());
            
            // 4. Insertar tablas relacionadas
            if (request.getDetalles() != null) {
                insertarDetalles(codigoGenerado, request.getDetalles());
            }
            
            return CatalogoResponse.builder()
                .success(true)
                .message("Registro creado exitosamente")
                .codigoGenerado(codigoGenerado)
                .build();
                
        } catch (DataIntegrityViolationException e) {
            throw new RuntimeException("Error de integridad: " + e.getMessage());
        }
    }
}
```

#### **Consultas Multi-JOIN**
```java
public CatalogoResponse consultar(String codigo) {
    // Query completa con todos los JOINs necesarios
    String sql = """
        SELECT p.codigo, p.nombre, p.descripcion, p.activo,
               e.idempresa, e.tipoprecio, e.canal, e.giro,
               g.nombre as nombreGiro, c.nombre as nombreCanal
        FROM tabla_principal p
        INNER JOIN tabla_empresa e ON p.codigo = e.codigo
        LEFT JOIN giros g ON e.giro = g.giro
        LEFT JOIN canales c ON e.canal = c.canal
        WHERE p.codigo = ?
        ORDER BY e.idempresa
        """;
        
    List<RegistroCompleto> registros = jdbcClient.sql(sql)
        .param(codigo)
        .query(this::mapToRegistroCompleto)
        .list();
        
    if (registros.isEmpty()) {
        throw new EntityNotFoundException("Registro no encontrado: " + codigo);
    }
    
    return construirRespuestaCompleta(registros);
}
```

### 4. Service - Validaciones de Negocio

#### **Patrón Service Transaccional**
```java
@Service
@Transactional
public class CatalogoServiceImpl implements CatalogoService {
    
    @Override
    public CatalogoResponse grabar(CatalogoRequest request, String sucursal) {
        try {
            // 1. Obtener empresa por sucursal
            Integer idEmpresa = obtenerEmpresaPorSucursal(sucursal);
            request.setIdEmpresa(idEmpresa);
            
            // 2. Validaciones específicas de catálogo
            validarReglasNegocio(request);
            
            // 3. Ejecutar operación CRUD
            CatalogoResponse response = repository.grabar(request);
            
            // 4. Enriquecer respuesta con catálogos dependientes
            if (response.getSuccess()) {
                response = enriquecerConCatalogos(response);
            }
            
            return response;
            
        } catch (Exception e) {
            log.error("Error en operación CRUD: {}", e.getMessage(), e);
            return CatalogoResponse.builder()
                .success(false)
                .message("Error interno: " + e.getMessage())
                .build();
        }
    }
    
    private void validarReglasNegocio(CatalogoRequest request) {
        // Validaciones específicas del catálogo
        if ("A".equals(request.getOperacion())) {
            validarDuplicados(request);
        }
        
        // Validar referencias FK
        validarReferencias(request);
        
        // Validar reglas específicas
        validarReglasEspecificas(request);
    }
}
```

### 5. Controller - Endpoints RESTful

#### **Endpoints CRUD Estándar**
```java
@RestController
@RequestMapping("/api/v1/catalogos/{entidad}")
@CrossOrigin(origins = "*")
public class CatalogoController {
    
    // CREATE/UPDATE - POST principal
    @PostMapping
    public ResponseEntity<CatalogoResponse> grabar(
            @PathVariable String entidad,
            @Valid @RequestBody CatalogoRequest request,
            @RequestHeader(value = "X-Sucursal", defaultValue = "1") String sucursal) {
        
        var response = catalogoService.grabar(entidad, request, sucursal);
        HttpStatus status = response.getSuccess() ? HttpStatus.OK : HttpStatus.BAD_REQUEST;
        return ResponseEntity.status(status).body(response);
    }
    
    // READ - GET individual  
    @GetMapping("/{codigo}")
    public ResponseEntity<CatalogoResponse> consultar(
            @PathVariable String entidad,
            @PathVariable String codigo) {
            
        var response = catalogoService.consultar(entidad, codigo);
        return ResponseEntity.ok(response);
    }
    
    // DELETE - DELETE con validaciones
    @DeleteMapping("/{codigo}")
    public ResponseEntity<CatalogoResponse> eliminar(
            @PathVariable String entidad,
            @PathVariable String codigo,
            @RequestBody BajaRequest bajaRequest) {
            
        var response = catalogoService.eliminar(entidad, codigo, bajaRequest);
        HttpStatus status = response.getSuccess() ? HttpStatus.OK : HttpStatus.BAD_REQUEST;
        return ResponseEntity.status(status).body(response);
    }
    
    // LIST - GET con filtros y paginación
    @GetMapping
    public ResponseEntity<ListResponse> listar(
            @PathVariable String entidad,
            @RequestParam(defaultValue = "0") int page,
            @RequestParam(defaultValue = "20") int size,
            @RequestParam(required = false) String filtro) {
            
        var response = catalogoService.listar(entidad, page, size, filtro);
        return ResponseEntity.ok(response);
    }
}
```

---

## Ejemplo Aplicado: Catálogo de Clientes

### Backend - Implementación Completa

#### **Estructura Multi-Tabla (6 tablas)**
```java
// Mapeo desde FormCatalogoClientes.cpp
public class CatalogoClientesRepository {
    
    // Tabla principal: clientes
    private void insertarCliente(String codigo, ClienteRequest request) {
        String sql = """
            INSERT INTO clientes (cliente, nombre, appat, apmat, titulo, 
                                rsocial, nomnegocio, rfc, tipoempre, activo)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
            """;
        jdbcClient.sql(sql)
            .param(codigo)
            .param(request.getNombre())
            // ... más parámetros
            .update();
    }
    
    // Tabla empresa (CRÍTICA): clientesemp  
    private void insertarClienteEmpresa(String codigo, List<DatoEmpresaRequest> datos) {
        for (DatoEmpresaRequest dato : datos) {
            String sql = """
                INSERT INTO clientesemp (cliente, idempresa, tipoprecio, 
                                       canal, giro, regimenfiscal, activo)
                VALUES (?, ?, ?, ?, ?, ?, ?)
                """;
            jdbcClient.sql(sql)
                .param(codigo)
                .param(dato.getIdEmpresa())
                // ... más parámetros
                .update();
        }
    }
    
    // Tablas relacionadas: datoscredito, direcciones, telefonos
    private void insertarDatosCredito(String codigo, DatosCreditoRequest credito) {
        // Solo si se proporcionan datos de crédito
        if (credito != null) {
            String sql = """
                INSERT INTO datoscredito (cliente, limitecredito, plazocredito, 
                                        creditoautorizado, excederlimite)
                VALUES (?, ?, ?, ?, ?)
                """;
            // ... implementación
        }
    }
}
```

#### **Validaciones Específicas**
```java
private void validarClienteRequest(ClienteRequest request) {
    // RFC válido según tipo de empresa
    String tipoEmpresa = request.getTipoempre();
    String rfc = request.getRfc();
    
    if ("0".equals(tipoEmpresa)) { // Persona física
        if (!validarRFCPersonaFisica(rfc)) {
            throw new IllegalArgumentException("RFC de persona física inválido");
        }
    } else if ("1".equals(tipoEmpresa)) { // Persona moral
        if (!validarRFCPersonaMoral(rfc)) {
            throw new IllegalArgumentException("RFC de persona moral inválido");
        }
    }
    
    // Validar unicidad de RFC por empresa
    if (existeRFCEnEmpresa(rfc, request.getIdEmpresa(), request.getCliente())) {
        throw new IllegalArgumentException("RFC ya existe en la empresa");
    }
}
```

### Frontend - Implementación React

#### **Hook Especializado Multi-Estado**
```typescript
const useCatalogoClientes = () => {
  const [estado, setEstado] = useState<EstadoCatalogo>({
    loading: false,
    cliente: null,
    catalogos: {
      giros: [],
      regimenes: [],
      canales: [],
      tiposPrecios: []
    },
    error: null,
    modoEdicion: false
  });
  
  const [formData, setFormData] = useState<ClienteFormData>({
    // Datos básicos
    codigo: '',
    nombre: '',
    rfc: '',
    tipoEmpresa: 'F', // F=Física, M=Moral
    activo: true,
    
    // Datos por empresa
    datosEmpresas: [{
      idEmpresa: 1,
      tipoPrecio: '',
      canal: '',
      giro: '',
      regimenFiscal: ''
    }],
    
    // Datos relacionados
    datosCredito: null,
    direcciones: [],
    telefonos: []
  });

  const grabar = useCallback(async () => {
    setEstado(prev => ({ ...prev, loading: true, error: null }));
    
    try {
      const request: ClienteRequest = {
        operacion: formData.codigo ? 'M' : 'A',
        ...formData
      };
      
      const response = await catalogoService.grabarCliente(request);
      
      if (response.success) {
        setEstado(prev => ({
          ...prev,
          loading: false,
          cliente: response.datos,
          modoEdicion: false
        }));
        
        // Actualizar código generado en alta
        if (!formData.codigo && response.codigoGenerado) {
          setFormData(prev => ({ 
            ...prev, 
            codigo: response.codigoGenerado 
          }));
        }
        
        toast.success('Cliente guardado exitosamente');
      } else {
        throw new Error(response.message);
      }
    } catch (error) {
      setEstado(prev => ({
        ...prev,
        loading: false,
        error: error.message
      }));
      toast.error(`Error: ${error.message}`);
    }
  }, [formData]);

  return {
    ...estado,
    formData,
    setFormData,
    grabar,
    consultar: async (codigo: string) => { /* implementar */ },
    eliminar: async (codigo: string, motivo: string) => { /* implementar */ },
    nuevo: () => { /* limpiar formulario */ }
  };
};
```

#### **Componente Multi-Pestaña**
```typescript
const CatalogoClientes: React.FC = () => {
  const {
    loading,
    cliente,
    catalogos,
    error,
    formData,
    setFormData,
    grabar,
    consultar,
    eliminar,
    nuevo
  } = useCatalogoClientes();

  const [tabActiva, setTabActiva] = useState<TabType>('general');
  
  return (
    <div className="catalogo-clientes">
      {/* Barra de navegación CRUD */}
      <Card className="mb-4">
        <CardHeader>
          <div className="flex justify-between items-center">
            <CardTitle>Catálogo de Clientes</CardTitle>
            <div className="flex gap-2">
              <Button onClick={nuevo} variant="outline">
                <Plus className="w-4 h-4 mr-2" />
                Nuevo
              </Button>
              <Button onClick={grabar} disabled={loading}>
                <Save className="w-4 h-4 mr-2" />
                {loading ? 'Guardando...' : 'Guardar'}
              </Button>
            </div>
          </div>
        </CardHeader>
      </Card>

      {/* Pestañas principales */}
      <Tabs value={tabActiva} onValueChange={(value) => setTabActiva(value as TabType)}>
        <TabsList className="grid w-full grid-cols-4">
          <TabsTrigger value="general">Datos Generales</TabsTrigger>
          <TabsTrigger value="empresa">Datos de Empresa</TabsTrigger>
          <TabsTrigger value="credito">Crédito</TabsTrigger>
          <TabsTrigger value="contacto">Contacto</TabsTrigger>
        </TabsList>

        {/* Pestaña Datos Generales */}
        <TabsContent value="general">
          <Card>
            <CardContent className="space-y-4 pt-6">
              
              {/* Código (readonly en modificación) */}
              <div className="grid grid-cols-2 gap-4">
                <div>
                  <Label>Código Cliente</Label>
                  <Input
                    value={formData.codigo}
                    onChange={(e) => setFormData(prev => ({ 
                      ...prev, 
                      codigo: e.target.value 
                    }))}
                    readOnly={!!cliente} // Solo editable en alta
                    className={!!cliente ? 'bg-gray-100' : ''}
                  />
                </div>
                
                <div>
                  <Label>Tipo de Empresa</Label>
                  <Select
                    value={formData.tipoEmpresa}
                    onValueChange={(value) => setFormData(prev => ({ 
                      ...prev, 
                      tipoEmpresa: value 
                    }))}
                  >
                    <SelectTrigger>
                      <SelectValue />
                    </SelectTrigger>
                    <SelectContent>
                      <SelectItem value="F">Persona Física</SelectItem>
                      <SelectItem value="M">Persona Moral</SelectItem>
                    </SelectContent>
                  </Select>
                </div>
              </div>

              {/* Campos dinámicos según tipo de empresa */}
              {formData.tipoEmpresa === 'F' ? (
                <PersonaFisicaFields 
                  data={formData} 
                  onChange={setFormData} 
                />
              ) : (
                <PersonaMoralFields 
                  data={formData} 
                  onChange={setFormData} 
                />
              )}
            </CardContent>
          </Card>
        </TabsContent>

        {/* Pestaña Datos de Empresa */}
        <TabsContent value="empresa">
          <Card>
            <CardContent className="space-y-4 pt-6">
              <DatosEmpresaSection
                datosEmpresas={formData.datosEmpresas}
                catalogos={catalogos}
                onChange={(nuevosdatos) => 
                  setFormData(prev => ({ 
                    ...prev, 
                    datosEmpresas: nuevosdatos 
                  }))
                }
              />
            </CardContent>
          </Card>
        </TabsContent>

        {/* Más pestañas... */}
      </Tabs>
    </div>
  );
};
```

---

## Buenas Prácticas Específicas de Catálogos

### 1. Manejo de Códigos Automáticos

#### **Generación Segura de Códigos**
```java
@Transactional
public String generarCodigoAutomatico(String entidad) {
    // Obtener siguiente número de secuencia
    String sql = """
        SELECT COALESCE(MAX(CAST(SUBSTRING(codigo, 2) AS UNSIGNED)), 0) + 1 as siguiente
        FROM """ + entidad + """ 
        WHERE codigo REGEXP '^[A-Z][0-9]+$'
        """;
    
    Integer siguiente = jdbcClient.sql(sql)
        .query(Integer.class)
        .single();
    
    // Formato: C00001, A00001, etc.
    String prefijo = entidad.substring(0, 1).toUpperCase();
    return String.format("%s%05d", prefijo, siguiente);
}
```

### 2. Validaciones de Integridad

#### **Patrón de Validaciones Transaccionales**
```java
@Transactional(readOnly = true)
public void validarEliminacion(String codigo) {
    // Verificar referencias en otras tablas
    List<String> tablasReferencia = List.of(
        "facturas", "movimientos", "pedidos"
    );
    
    for (String tabla : tablasReferencia) {
        String sql = "SELECT COUNT(*) FROM " + tabla + " WHERE cliente = ?";
        Integer count = jdbcClient.sql(sql).param(codigo).query(Integer.class).single();
        
        if (count > 0) {
            throw new DataIntegrityViolationException(
                String.format("No se puede eliminar: existen %d registros en %s", count, tabla)
            );
        }
    }
}
```

### 3. Catálogos Dependientes Dinámicos

#### **Carga Inteligente de Combos**
```java
public List<CatalogoDto> obtenerCatalogo(String tipo, Integer idEmpresa) {
    String sql = switch (tipo) {
        case "giros" -> "SELECT giro as codigo, nombre FROM giros WHERE activo = 1";
        case "canales" -> """
            SELECT canal as codigo, nombre FROM canales 
            WHERE activo = 1 AND (idempresa = ? OR idempresa IS NULL)
            ORDER BY idempresa DESC, nombre
            """;
        case "regimenes" -> """
            SELECT regimen as codigo, descripcion as nombre 
            FROM regimenes_fiscales WHERE activo = 1
            """;
        default -> throw new IllegalArgumentException("Catálogo no válido: " + tipo);
    };
    
    var query = jdbcClient.sql(sql);
    if (sql.contains("idempresa = ?")) {
        query = query.param(idEmpresa);
    }
    
    return query.query((rs, rowNum) -> 
        CatalogoDto.builder()
            .codigo(rs.getString("codigo"))
            .nombre(rs.getString("nombre"))
            .build()
    ).list();
}
```

### 4. Frontend - Estado Complejo

#### **Manejo de Estado Multi-Nivel**
```typescript
interface EstadoCatalogo<T> {
  // Estados operacionales
  loading: boolean;
  saving: boolean;
  deleting: boolean;
  
  // Datos principales
  registro: T | null;
  esNuevo: boolean;
  modoConsulta: boolean;
  
  // Catálogos dependientes
  catalogos: {
    [key: string]: CatalogoOption[];
  };
  
  // Manejo de errores
  errores: {
    [campo: string]: string;
  };
  
  // UI específica
  tabActiva: string;
  mostrarConfirmacion: boolean;
}

// Hook reutilizable para cualquier catálogo
const useCatalogo = <T>(entidad: string) => {
  const [estado, setEstado] = useState<EstadoCatalogo<T>>({
    loading: false,
    saving: false,
    deleting: false,
    registro: null,
    esNuevo: true,
    modoConsulta: false,
    catalogos: {},
    errores: {},
    tabActiva: 'general',
    mostrarConfirmacion: false
  });
  
  // Métodos CRUD genéricos
  const operaciones = {
    async cargar(codigo: string) { /* implementar */ },
    async guardar(datos: T) { /* implementar */ },
    async eliminar(codigo: string, motivo: string) { /* implementar */ },
    nuevo() { /* resetear estado */ }
  };
  
  return { estado, ...operaciones };
};
```

---

## Referencias Cruzadas

### Documentación General
- **[Backend General](backend-general.md)** - Guardrails tecnológicos y patrones transaccionales
- **[Frontend General](frontend-general.md)** - Arquitectura React y componentes reutilizables
- **[Backend Testing](backend-testing.md)** - Testing de endpoints CRUD

### Documentación Específica de Catálogos
- **[📋 CATALOGO_CLIENTES.md](../docs/CATALOGO_CLIENTES.md)** - Migración completa con todos los detalles
- **[🔍 Ejemplo de Referencia](../src/main/java/com/lavioleta/desarrollo/violetaserver/controller/CatalogoClientesController.java)** - Implementación completa

### Código Legacy de Referencia
- **[cpp-original.md](cpp-original.md)** - Interpretación del código C++ legacy
- **[dfm-original.md](dfm-original.md)** - Análisis de formularios .dfm
- **[📁 cpp/FormCatalogo*.cpp](../cpp/)** - Código fuente original

### Testing y Validación
- **[🧪 Tests Unitarios](../src/test/java/com/lavioleta/desarrollo/violetaserver/controller/CatalogoClientesControllerWebTest.java)** - Ejemplos de testing
- **[⚙️ Tests E2E](../frontend/tests/e2e/)** - Validación completa de flujos

---

## Próximos Módulos de Catálogo a Migrar

### Prioridad 1 - Catálogos Maestros
- **Catálogo Artículos** - Multi-empresa + precios por sucursal + existencias
- **Catálogo Proveedores** - Similar a clientes con datos fiscales

### Prioridad 2 - Catálogos Operativos  
- **Catálogo Vendedores** - Por sucursal + comisiones + zonas
- **Catálogo Empleados** - Datos laborales + permisos + sucursales

### Prioridad 3 - Catálogos de Soporte
- **Catálogo Marcas** - Simple con validación de duplicados
- **Catálogo Clasificaciones** - Jerárquico con niveles

### Aplicar Este Patrón Para
1. **Identificar** FormCatalogo*.cpp/.dfm correspondiente y ClassServidorCatalogos.cpp
2. **Mapear** operaciones CRUD (IDs ID_GRA_*, ID_BAJ_*, etc.)
3. **Analizar** estructura multi-tabla y validaciones de integridad
4. **Implementar** backend siguiendo patrón transaccional (Request → Service → Repository → Response)
5. **Migrar** frontend replicando pestañas y controles de formulario legacy
6. **Validar** con tests unitarios e integración cubriendo todas las operaciones CRUD

El **Catálogo de Clientes migrado exitosamente** es la plantilla base perfecta para replicar en todos los demás catálogos del sistema, garantizando consistencia arquitectónica y funcional.