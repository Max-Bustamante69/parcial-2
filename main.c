/*
 * Smart Backup - Kernel-Space Utility (User-Space Simulation)
 * EAFIT - Sistemas Operativos (C2661-SI2004-5186) Parcial 2
 * Autores: Maximiliano Bustamante G, Valeria Hornung U
 */

#include "smart_copy.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#define VERSION "1.0.0"
#define PROGRAMA "smart-backup"
#define COLOR_RESET "\x1b[0m"
#define COLOR_RED "\x1b[31m"
#define COLOR_GREEN "\x1b[32m"
#define COLOR_YELLOW "\x1b[33m"
#define COLOR_BLUE "\x1b[34m"
#define COLOR_CYAN "\x1b[36m"

// Banner introductorio (Maximiliano Bustamante G)
void imprimir_banner(void) {
    printf("\n%sSMART BACKUP - Kernel-Space Utility%s\n", COLOR_CYAN, COLOR_RESET);
    printf("EAFIT Parcial 2 - Sistemas Operativos\n");
    printf("Maximiliano Bustamante G, Valeria Hornung U | v%s\n\n", VERSION);
}

// Mostrar información de uso (Valeria Hornung U)
void imprimir_ayuda(const char *nombre_prog) {
    printf("%s=== MANUAL ===%s\n", COLOR_BLUE, COLOR_RESET);
    printf("USO: %s [COMANDO] [OPCIONES]\n\n", nombre_prog);
    printf("COMANDOS:\n");
    printf("  -b,--backup ORIGEN DESTINO   Copia archivo/directorio\n");
    printf("  -c,--compare MB DIR           Benchmarks fread vs sys_smart_copy\n");
    printf("  -g,--generate MB ARCHIVO     Genera archivo de prueba\n");
    printf("  -h,--help                     Muestra esta ayuda\n");
    printf("  --version                     Muestra versión\n\n");
    printf("EJEMPLOS:\n");
    printf("  %s -b documento.pdf backup.pdf\n", nombre_prog);
    printf("  %s -c 100 /tmp/bench.bin\n", nombre_prog);
    printf("  %s -g 5 /tmp/test.bin\n\n", nombre_prog);
}

// Generar archivo de prueba (Maximiliano Bustamante G)
int generar_archivo_prueba(const char *ruta_archivo, int mb_totales) {
    FILE *f = fopen(ruta_archivo, "wb");
    if (!f) { perror("fopen"); return -1; }
    
    char buffer[8192];
    for (int i = 0; i < (int)sizeof(buffer); i++) buffer[i] = (char)(i % 256);
    
    long bytes_totales = (long)mb_totales * 1024 * 1024;
    long bytes_escritos = 0;
    
    printf("%s[Generando]%s %d MB en %s\n", COLOR_YELLOW, COLOR_RESET, mb_totales, ruta_archivo);
    
    while (bytes_escritos < bytes_totales) {
        int bytes_write = sizeof(buffer);
        if (bytes_escritos + bytes_write > bytes_totales)
            bytes_write = bytes_totales - bytes_escritos;
        
        if (fwrite(buffer, 1, bytes_write, f) != (size_t)bytes_write) {
            perror("fwrite"); fclose(f); return -1;
        }
        bytes_escritos += bytes_write;
        
        if (bytes_escritos % (bytes_totales / 10) == 0 || bytes_escritos == bytes_totales)
            printf("  %3ld%% (%ld MB)\n", (bytes_escritos * 100) / bytes_totales, bytes_escritos / (1024*1024));
    }
    
    fclose(f);
    printf("%s✓ Archivo generado%s\n\n", COLOR_GREEN, COLOR_RESET);
    return 0;
}

/* ========================================================================== */
/* FUNCIÓN PRINCIPAL: main()                                                 */
/* ========================================================================== */

/**
 * FUNCIÓN: main
 * PROPÓSITO: Punto de entrada del programa. Procesa argumentos CLI.
 * AUTOR: Maximiliano Bustamante G y Valeria Hornung U
 * 
 * DESCRIPCIÓN DETALLADA:
 *     El programa utiliza un modelo de comandos simple:
 *     
 *     $ smart-backup [COMANDO] [ARGUMENTOS]
 *     
 *     Comandos soportados:
 *     - backup (-b): Copia archivo/directorio
 *     - compare (-c): Benchmarking
 *     - generate (-g): Crear archivo de prueba
 *     - help (-h): Mostrar ayuda
 *     - version: Mostrar versión
 * 
 * USO TÍPICO:
 *     # Copiar archivo con estadísticas
 *     $ ./smart-backup -b documento.pdf backup.pdf
 *     
 *     # Comparar velocidades con archivo de 10MB
 *     $ ./smart-backup -c 10 /tmp/bench.bin
 */
int main(int argc, char *argv[]) {
    int resultado = EXIT_SUCCESS;
    int es_directorio;
    stats_copia_t estadisticas = {0};

    /* ====== MOSTRAR BANNER ====== */
    imprimir_banner();

    /* ====== PROCESAR ARGUMENTOS ====== */
    if (argc < 2) {
        printf("%s[ERROR]%s Argumentos insuficientes\n\n", COLOR_RED, COLOR_RESET);
        imprimir_ayuda(argv[0]);
        return EXIT_FAILURE;
    }

    /* ====== COMANDO: --version ====== */
    if (strcmp(argv[1], "--version") == 0) {
        printf("Smart Backup v%s\n", VERSION);
        printf("Proyecto: Parcial 2 - Sistemas Operativos (EAFIT)\n");
        printf("Autores: Maximiliano Bustamante G, Valeria Hornung U\n");
        return EXIT_SUCCESS;
    }

    /* ====== COMANDO: -h / --help ====== */
    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        imprimir_ayuda(argv[0]);
        return EXIT_SUCCESS;
    }

    /* ====== COMANDO: -b / --backup ====== 
     * 
     * Sintaxis: smart-backup -b ORIGEN DESTINO
     * 
     * Procesa:
     * 1. Valida que origen existe
     * 2. Valida que destino es accesible
     * 3. Copia archivo o directorio
     * 4. Imprime estadísticas
     */
    if (strcmp(argv[1], "-b") == 0 || strcmp(argv[1], "--backup") == 0) {
        
        if (argc != 4) {
            fprintf(stderr, "%s[ERROR]%s Sintaxis incorrecta para -b\n\n", 
                    COLOR_RED, COLOR_RESET);
            fprintf(stderr, "USO: %s -b ORIGEN DESTINO\n\n", argv[0]);
            return EXIT_FAILURE;
        }

        const char *origen = argv[2];
        const char *destino = argv[3];

        /* ====== Paso 1: Validar origen ====== */
        printf("%s[1/3]%s Validando ruta de origen...\n", COLOR_BLUE, COLOR_RESET);
        
        if (validar_ruta_origen(origen, &es_directorio) == -1) {
            fprintf(stderr, "%s[ERROR]%s No se puede acceder a '%s': %s\n", 
                    COLOR_RED, COLOR_RESET, origen, strerror(errno));
            return EXIT_FAILURE;
        }

        if (es_directorio) {
            printf("  ✓ Origen es un directorio\n");
        } else {
            printf("  ✓ Origen es un archivo\n");
        }

        /* ====== Paso 2: Validar destino ====== */
        printf("%s[2/3]%s Validando ruta de destino...\n", COLOR_BLUE, COLOR_RESET);
        
        if (validar_ruta_destino(destino) == -1) {
            fprintf(stderr, "%s[ERROR]%s Destino inválido '%s': %s\n", 
                    COLOR_RED, COLOR_RESET, destino, strerror(errno));
            return EXIT_FAILURE;
        }
        printf("  ✓ Ruta de destino es accesible\n");

        /* ====== Paso 3: Ejecutar copia ====== */
        printf("%s[3/3]%s Iniciando copia...\n", COLOR_BLUE, COLOR_RESET);
        
        if (es_directorio) {
            printf("  → Copiando directorio: %s → %s\n", origen, destino);
            resultado = sys_smart_copy_directory(origen, destino, &estadisticas);
        } else {
            printf("  → Copiando archivo: %s → %s\n", origen, destino);
            resultado = sys_smart_copy_file(origen, destino, &estadisticas);
        }

        if (resultado == -1) {
            fprintf(stderr, "%s[ERROR]%s Fallo en la copia: %s\n", 
                    COLOR_RED, COLOR_RESET, strerror(errno));
            return EXIT_FAILURE;
        }

        /* ====== Mostrar estadísticas ====== */
        printf("\n%s✓ COPIA COMPLETADA EXITOSAMENTE%s\n", COLOR_GREEN, COLOR_RESET);
        imprimir_estadisticas(&estadisticas, "Copia con sys_smart_copy");

        return EXIT_SUCCESS;
    }

    /* ====== COMANDO: -c / --compare ====== 
     * 
     * Sintaxis: smart-backup -c TAMAÑO(MB) ARCHIVO_SALIDA
     * 
     * Procesa:
     * 1. Genera archivo de prueba
     * 2. Copia con fread/fwrite
     * 3. Copia con sys_smart_copy
     * 4. Compara rendimiento
     */
    if (strcmp(argv[1], "-c") == 0 || strcmp(argv[1], "--compare") == 0) {
        
        if (argc != 4) {
            fprintf(stderr, "%s[ERROR]%s Sintaxis incorrecta para -c\n\n", 
                    COLOR_RED, COLOR_RESET);
            fprintf(stderr, "USO: %s -c TAMAÑO(MB) ARCHIVO_SALIDA\n\n", argv[0]);
            fprintf(stderr, "Ejemplo: %s -c 10 /tmp/bench.bin\n\n", argv[0]);
            return EXIT_FAILURE;
        }

        int tamaño_mb = atoi(argv[2]);
        const char *archivo_base = argv[3];

        if (tamaño_mb <= 0 || tamaño_mb > 1024) {
            fprintf(stderr, "%s[ERROR]%s Tamaño debe estar entre 1 y 1024 MB\n", 
                    COLOR_RED, COLOR_RESET);
            return EXIT_FAILURE;
        }

        /* Construir nombres de archivos derivados */
        char archivo_clib[512];
        char archivo_smart[512];
        snprintf(archivo_clib, sizeof(archivo_clib), "%s.clib", archivo_base);
        snprintf(archivo_smart, sizeof(archivo_smart), "%s.smart", archivo_base);

        /* Generar archivo de prueba base */
        if (generar_archivo_prueba(archivo_base, tamaño_mb) == -1) {
            return EXIT_FAILURE;
        }

        /* Comparar rendimiento */
        comparar_rendimiento_fread_vs_smart_copy(archivo_base, 
                                                 archivo_clib, 
                                                 archivo_smart);

        return EXIT_SUCCESS;
    }

    /* ====== COMANDO: -g / --generate ====== 
     * 
     * Sintaxis: smart-backup -g TAMAÑO(MB) ARCHIVO_SALIDA
     * 
     * Genera un archivo de prueba sin hacer comparativas.
     */
    if (strcmp(argv[1], "-g") == 0 || strcmp(argv[1], "--generate") == 0) {
        
        if (argc != 4) {
            fprintf(stderr, "%s[ERROR]%s Sintaxis incorrecta para -g\n\n", 
                    COLOR_RED, COLOR_RESET);
            fprintf(stderr, "USO: %s -g TAMAÑO(MB) ARCHIVO_SALIDA\n\n", argv[0]);
            return EXIT_FAILURE;
        }

        int tamaño_mb = atoi(argv[2]);
        const char *archivo_salida = argv[3];

        if (tamaño_mb <= 0 || tamaño_mb > 1024) {
            fprintf(stderr, "%s[ERROR]%s Tamaño debe estar entre 1 y 1024 MB\n", 
                    COLOR_RED, COLOR_RESET);
            return EXIT_FAILURE;
        }

        if (generar_archivo_prueba(archivo_salida, tamaño_mb) == -1) {
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    }

    /* ====== COMANDO NO RECONOCIDO ====== */
    fprintf(stderr, "%s[ERROR]%s Comando no reconocido: '%s'\n\n", 
            COLOR_RED, COLOR_RESET, argv[1]);
    imprimir_ayuda(argv[0]);
    return EXIT_FAILURE;
}

/* ========================================================================== */
/* FIN DE ARCHIVO: main.c                                                     */
/* ========================================================================== */
