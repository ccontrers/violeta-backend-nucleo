# Módulos tipo Catálogos# Módulos tipo — Catálogos (patrón)



Guía para migrar módulos de catálogos CRUD desde el sistema legado C++ Builder a Spring Boot + React, basada en la migración exitosa del **Catálogo de Clientes**.## Estructura recomendada

- DTOs (request/response)

## Introducción- Repository (consultas SQL)

- Service / ServiceImpl (lógica + transacciones)

Los **módulos de catálogo** en el sistema legacy son formularios CRUD (Create, Read, Update, Delete) que permiten gestionar maestros de datos. Se caracterizan por:- Controller (endpoints REST)