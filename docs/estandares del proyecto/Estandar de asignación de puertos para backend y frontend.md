# **Estándar de Asignación de Puertos para Microservicios**

## **1. Objetivo**

Definir un esquema consistente de puertos para frontends, backends y bases de datos en los entornos de desarrollo y producción,
 evitando colisiones y facilitando la organización interna.

---

## **2. Asignación de Puertos**

### **2.1 Desarrollo**

* **Backends:** 6820–6839
* **Frontends:** 3120–3139
* **Bases de datos:** 3300–3319

### **2.2 Producción**

* **Backends:** 7820–7839
* **Frontends:** 8120–8139
* **Bases de datos:** 3320–3339

---

## **3. Lineamientos**

1. Cada servicio debe utilizar únicamente un puerto dentro del rango asignado a su tipo y entorno.
2. Los rangos no deben reutilizarse entre servicios dentro del mismo entorno.
3. Cualquier nuevo backend, frontend o base de datos debe seleccionar el siguiente puerto disponible dentro de su rango correspondiente.
4. Está prohibido usar puertos fuera de estos rangos para servicios internos sin autorización técnica.
