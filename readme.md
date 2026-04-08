# Smart Backup - Kernel-Space Utility (Simulado)

## 📋 Información General

**Proyecto:** Parcial 2 - Sistemas Operativos  
**Universidad:** EAFIT  
**Curso:** Sistemas Operativos - C2661-SI2004-5186  
**Fecha:** 7 de Abril 2026

### 👥 Autores y Contribuciones

| Autor                        | Contribución                                                                                                                         |
| ---------------------------- | ------------------------------------------------------------------------------------------------------------------------------------ |
| **Maximiliano Bustamante G** | Diseño del motor de respaldo, optimizaciones de buffer, implementación de syscalls directo, comparativa de rendimiento, interfaz CLI |
| **Valeria Hornung U**        | Validación robusta de rutas, manejo completo de errores con errno, recursión segura en directorios, captura de estadísticas          |

---

## 🎯 Descripción del Proyecto

**Smart Backup** es un sistema de respaldo de archivos que simula el comportamiento de una función a nivel de kernel (kernel-space) pero se ejecuta en espacio de usuario (user-space).

### Objetivos Académicos

El proyecto demonstra:

1. **Uso de System Calls POSIX:**
   - `open()`, `read()`, `write()`, `close()` para operaciones de bajo nivel
   - `stat()`, `lstat()` para obtener metadatos de archivos
   - `mkdir()`, `opendir()`, `readdir()`, `closedir()` para directorios

2. **Optimización de I/O:**
   - Importancia del tamaño de buffer (4096 bytes = PAGE_SIZE)
   - Reducción de context switches kernel ↔ user mode
   - Comparativa real entre `fread`/`fwrite` vs syscalls directos

3. **Manejo Robusto de Errores:**
   - Validación de permisos POSIX
   - Uso adecuado de `errno`
   - Recuperación segura de recursos

4. **Captura de Estadísticas:**
   - Medición precisa de tiempo con `CLOCK_MONOTONIC`
   - Conteo de syscalls realizadas
   - Cálculo de throughput (MB/s)

---

## 📁 Estructura del Proyecto

```
parcial-2/
│
├── smart_copy.h              [HEADER] Definiciones de interfaz
│                             Autores: Ambos
│
├── backup_engine.c           [MOTOR] Implementación central
│                             Autor: Maximiliano (lógica)
│                             Autor: Valeria (validación)
│
├── main.c                    [CLI] Interfaz de usuario
│                             Autor: Maximiliano (CLI+parseo)
│                             Autor: Valeria (validación entrada)
│
├── Makefile                  [BUILD] Script de construcción
│                             Autores: Ambos
│
├── README.md                 [DOCS] Este archivo
│                             Autores: Ambos
│
└── apoyoTematico/            [REFERENCIA] Código base de referencia
    ├── lib_io.c              Ejemplo: fputc
    ├── syscall_io.c          Ejemplo: write 1 byte
    └── syscall_buffered.c    Ejemplo: write con buffer
```

### Archivos Principales

#### 1. **smart_copy.h** (Header/Definiciones)

- Constantes: `BUFFER_SIZE`, `MAX_PATH_LENGTH`, etc.
- Estructura: `stats_copia_t` (statisticas de copia)
- Declaraciones de funciones públicas
- Documentación extensiva en formato Doxygen

**Autor principal: Maximiliano (estructura) + Valeria (validación)**

#### 2. **backup_engine.c** (Motor de Respaldo)

Implementa el "corazón" del sistema:

**Funciones implementadas por Maximiliano:**

- `sys_smart_copy_file()` - Copia optimizada de archivo individual
- `comparar_rendimiento_fread_vs_smart_copy()` - Benchmarking
- `imprimir_estadisticas()` - Presentación de resultados
- `calcular_tiempo_transcurrido_ms()` - Timing preciso

**Funciones implementadas por Valeria:**

- `sys_smart_copy_directory()` - Copia recursiva de directorios
- `validar_ruta_origen()` - Validación de origen
- `validar_ruta_destino()` - Validación de destino

#### 3. **main.c** (Interfaz CLI)

Interfaz de línea de comandos con soporte para:

**Implementado por Maximiliano:**

- Parseo de argumentos
- Generación de archivos de prueba
- Interfaz colorizada
- Lógica principal

**Implementado por Valeria:**

- Validación de entrada
- Manejo de errores
- Recuperación de recursos

#### 4. **Makefile**

Script de automatización:

- `make` - Compilar
- `make test` - Pruebas completas
- `make benchmark` - Solo comparativa
- `make clean` - Limpiar archivos
- `make info` - Información del proyecto

---

## 🚀 Cómo Compilar

### Prerrequisitos

- **Compilador:** GCC 4.9+, Clang 3.5+, o compatible POSIX
- **Sistema operativo:** Linux, macOS, o BSD
- **Herramientas:** `make` (GNU Make 3.8+)

### Pasos de Compilación

```bash
# 1. Navegar al directorio del proyecto
cd parcial-2

# 2. Compilar el proyecto
make

# Output esperado:
# [Compilando] main.c → main.o
# [Compilando] backup_engine.c → backup_engine.o
# [Linker] Generando smart-backup...
# ✓ Compilación completada exitosamente
```

### Verificación

```bash
# Verificar que el ejecutable se creó
ls -lh smart-backup

# Debería mostrar algo como:
# -rwxr-xr-x 1 usuario grupo 50K ene 15 10:30 smart-backup
```

---

## 📖 Cómo Usar el Programa

### Sintaxis General

```bash
./smart-backup [COMANDO] [OPCIONES]
```

### Comando 1: Copiar Archivo o Directorio

```bash
./smart-backup -b ORIGEN DESTINO
# O
./smart-backup --backup ORIGEN DESTINO
```

**Ejemplos:**

```bash
# Copiar archivo individual
./smart-backup -b documento.pdf backup-documento.pdf

# Copiar directorio recursivamente
./smart-backup -b /home/usuario/documentos /backup/documentos
```

**Salida esperada:**

```
╔════════════════════════════════════════════════════════════════╗
║     SMART BACKUP - Kernel-Space Utility (Simulado)           ║
║                                                              ║
║  Proyecto: Parcial 2 - Sistemas Operativos (EAFIT)          ║
║  Autores:  Maximiliano Bustamante G, Valeria Hornung U      ║
│  Curso:    Sistemas Operativos - C2661-SI2004-5186          ║
║  Versión:  1.0.0                                            ║
╚════════════════════════════════════════════════════════════════╝

[1/3] Validando ruta de origen...
  ✓ Origen es un archivo
[2/3] Validando ruta de destino...
  ✓ Ruta de destino es accesible
[3/3] Iniciando copia...
  → Copiando archivo: documento.pdf → backup-documento.pdf

✓ COPIA COMPLETADA EXITOSAMENTE

========================================
ESTADÍSTICAS: Copia con sys_smart_copy
========================================
Bytes copiados:       5242880 bytes (5.00 MB)
Syscalls realizadas:  1282
Archivos procesados:  1
Directorios procesados: 0
Tiempo total:         45.321 ms (0.045321 seg)
Throughput:           110.23 MB/s
========================================
```

### Comando 2: Comparativa de Rendimiento

```bash
./smart-backup -c TAMAÑO(MB) ARCHIVO_SALIDA
# O
./smart-backup --compare TAMAÑO(MB) ARCHIVO_SALIDA
```

**Ejemplos:**

```bash
# Crear archivo de 10MB y comparar velocidades
./smart-backup -c 10 /tmp/bench.bin

# Crear archivo de 50MB y comparar
./smart-backup -c 50 /tmp/bench_50mb.bin
```

### Comando 3: Generar Archivo de Prueba

```bash
./smart-backup -g TAMAÑO(MB) ARCHIVO_SALIDA
# O
./smart-backup --generate TAMAÑO(MB) ARCHIVO_SALIDA
```

**Ejemplo:**

```bash
./smart-backup -g 5 /tmp/archivo-prueba.bin
```

### Comando 4: Ayuda

```bash
./smart-backup -h
./smart-backup --help
```

---

## 🧪 Ejecución de Pruebas

### Suite Completa de Pruebas

```bash
make test
```

Esto ejecuta automáticamente:

1. Generación de archivo de 5MB
2. Copia de archivo individual
3. Creación de estructura de directorios
4. Copia recursiva de directorio
5. Comparativa de rendimiento (10MB)

### Solo Benchmarking

```bash
make benchmark
```

Ejecuta comparativa con archivo de 50MB.

### Prueba Manual

```bash
# 1. Crear un archivo de prueba
./smart-backup -g 1 /tmp/test.bin

# 2. Copiarlo
./smart-backup -b /tmp/test.bin /tmp/backup.bin

# 3. Verificar que la copia es idéntica
cmp /tmp/test.bin /tmp/backup.bin && echo "✓ Archivos idénticos"

# 4. Benchmarking
./smart-backup -c 25 /tmp/bench.bin
```

---

## 🔍 Análisis Técnico Detallado

### 1. Tamaño de Buffer

#### El Problema

Si copiáramos archivo de 1MB byte a byte:

```c
char c;
for (long i = 0; i < 1000000; i++) {
    read(fd_src, &c, 1);    // ← SYSCALL 1
    write(fd_dst, &c, 1);   // ← SYSCALL 2
}
// Total: 2 MILLONES de syscalls
// Cada syscall = context switch kernel ↔ user (MUY LENTO)
```

#### La Solución: Buffer de 4KB

```c
char buffer[4096];  // Buffer de 4KB (PAGE_SIZE)
ssize_t bytes_read;
while ((bytes_read = read(fd_src, buffer, 4096)) > 0) {
    write(fd_dst, buffer, bytes_read);
}
// Total: ~244 syscalls para 1MB
// Mejora: ~8200× más rápido
```

### 2. System Calls Utilizadas

| Syscall      | Propósito                          | Implementado |
| ------------ | ---------------------------------- | ------------ |
| `open()`     | Abrir archivo (lectura/escritura)  | ✓            |
| `close()`    | Cerrar descriptor                  | ✓            |
| `read()`     | Leer desde descriptor              | ✓            |
| `write()`    | Escribir en descriptor             | ✓            |
| `stat()`     | Obtener metadatos sin seguir links | ✓            |
| `lstat()`    | Obtener metadatos (lstat)          | ✓            |
| `mkdir()`    | Crear directorio                   | ✓            |
| `opendir()`  | Abrir directorio                   | ✓            |
| `readdir()`  | Leer entrada de directorio         | ✓            |
| `closedir()` | Cerrar directorio                  | ✓            |
| `access()`   | Verificar permisos                 | ✓            |

### 3. Manejo de Errores

Código implementa manejo robusto de errores POSIX:

```c
// ✓ Valida archivo origen existe
if (stat(src_path, &st) == -1) {
    return -1;  // errno seteado por stat()
}

// ✓ Rechaza directorios
if (S_ISDIR(st.st_mode)) {
    errno = EISDIR;
    return -1;
}

// ✓ Maneja falta de espacio en disco
if (bytes_writtn != bytes_leidos) {
    // errno = ENOSPC o similar
    resultado = -1;
}

// ✓ Siempre cierra archivos (incluso en error)
close(fd_src);
close(fd_dest);
```

---

## 💡 Conceptos de Sistemas Operativos

### Context Switch

Cuando un programa llama a una syscall:

```
┌─────────────────────────────────────────────────────┐
│ USER MODE                                           │
│ Tu programa C                                       │
│                                                     │
│  write(fd, buffer, 4096);  ← Invoca syscall       │
└─────┬───────────────────────────────────────────────┘
      │
      ▼ CONTEXT SWITCH (caro en CPU cycles)
      │
┌─────┴───────────────────────────────────────────────┐
│ KERNEL MODE                                         │
│ Sistema Operativo                                   │
│                                                     │
│  - Verificar permisos                             │
│  - Escribir en sistema de archivos                 │
│  - Actualizar inodos                               │
│  - Interactuar con hardware (disco)                │
└─────┬───────────────────────────────────────────────┘
      │
      ▼ CONTEXT SWITCH (volver a user mode)
      │
┌─────┴───────────────────────────────────────────────┐
│ USER MODE                                           │
│ Tu programa continue...                             │
└─────────────────────────────────────────────────────┘
```

---


