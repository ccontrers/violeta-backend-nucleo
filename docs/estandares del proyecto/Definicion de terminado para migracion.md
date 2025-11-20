Definición de terminado para cada API que se genere:

Validez técnica:

Endpoint documentado en OpenAPI/Swagger con ejemplos y descripciones.
Validaciones, manejo de errores y respuestas normalizadas implementados.
Autenticación/autorización configuradas en SecurityConfig, CORS actualizado si aplica.
Registro de auditoría y métricas en logs/actuators cuando corresponda.

Pruebas y QA

Pruebas unitarias e integración automáticas pasando en CI.
Revisión de código aprobada y despliegue probado en el entorno objetivo.


Pruebas automatizadas
	Backend
		Pruebas unitarias 
        Pruebas de integración
		Pruebas de Apis
	Frontend
		Pruebas unitarias
        Pruebas de integración
		Pruebas de interfaz (playwrite)
			- Validacion
			- Existencia de control

Pruebas en desarrollo.
	Se obvian las de caja blanca para el código generado manualmente, para el código generado por IA hay que generar buenas práctica.
	Pruebas de funcionalidad por equipo de sistemas.
	Crear definición de terminado organizacional (incluye checklist relevantes).
		Antes del inicio del proyecto definir una inicial.
	Revisar con el PO (evento de sprint review)
	Pruebas con el usuario.