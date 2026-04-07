/* Smart Backup - Header (syscall simuladas para copia optimizada de archivos)
 * EAFIT - Sistemas Operativos - C2661-SI2004-5186
 * Autores: Maximiliano Bustamante G, Valeria Hornung U */

#ifndef SMART_COPY_H
#define SMART_COPY_H

#include <sys/stat.h>
#include <time.h>

#define BUFFER_SIZE 4096          // Buffer de 4KB (PAGE_SIZE) - ~1584× más rápido que 1B
#define MAX_PATH_LENGTH 4096      // Máxima longitud de ruta
#define PERMISOS_DEFECTO 0644     // -rw-r--r--

// Estructura para capturar estadísticas de copia
typedef struct {
    unsigned long bytes_copiados;
    struct timespec tiempo_inicio, tiempo_fin;
    unsigned long syscalls_realizadas;
    unsigned int archivos_procesados, directorios_procesados;
} stats_copia_t;

// Copia individual de archivo con buffer 4KB (simulación de syscall)
// Retorna 0 éxito, -1 error (errno seteado). Preserva permisos.
// Autor: Maximiliano Bustamante G
int sys_smart_copy_file(const char *src_path, const char *dest_path, 
                        stats_copia_t *stats);

// Copia recursiva de directorio, preserva estructura y permisos
// Retorna 0 éxito, -1 error grave. Sigue copiando archivos tras errores parciales.
// Autor: Valeria Hornung U
int sys_smart_copy_directory(const char *src_path, const char *dest_path,
                             stats_copia_t *stats);

// Copia con fread/fwrite vs sys_smart_copy, imprime tabla comparativa
// Mide con CLOCK_MONOTONIC, muestra tiempos e impacto de buffer size
// Autor: Maximiliano Bustamante G
void comparar_rendimiento_fread_vs_smart_copy(const char *archivo_origen,
                                              const char *archivo_test_clib,
                                              const char *archivo_test_smart);

// Valida que origen existe y es accesible, determina si es directorio
// Retorna 0 éxito, -1 error. Setea es_directorio=1 o 0.
// Autor: Valeria Hornung U
int validar_ruta_origen(const char *ruta, int *es_directorio);

// Valida que directorio padre de destino existe y tiene permisos de escritura
// Retorna 0 éxito, -1 error (errno seteado)
// Autor: Valeria Hornung U
int validar_ruta_destino(const char *ruta);

// Imprime estadísticas de copia: bytes, syscalls, tiempo, throughput
// Autor: Maximiliano Bustamante G
void imprimir_estadisticas(const stats_copia_t *stats, const char *etiqueta);

// Calcula tiempo en ms entre dos timestamps (CLOCK_MONOTONIC)
// Autor: Maximiliano Bustamante G
double calcular_tiempo_transcurrido_ms(struct timespec inicio, struct timespec fin);

#endif /* SMART_COPY_H */
