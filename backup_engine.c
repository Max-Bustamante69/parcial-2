/* Motor de Respaldo - Implementación de syscalls simuladas
 * EAFIT - Sistemas Operativos - C2661-SI2004-5186
 * Autores: Maximiliano Bustamante G, Valeria Hornung U */

#include "smart_copy.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <time.h>

// Portabilidad Windows/Unix
#ifdef _WIN32
    #include <io.h>
    #include <direct.h>
    #define mkdir_portable(path, mode) _mkdir(path)
    #define lstat stat
#else
    #define mkdir_portable(path, mode) mkdir(path, mode)
#endif

// === FUNCIÓN PRINCIPAL: Copia optimizada de archivo ===
// Autor: Maximiliano Bustamante G
// Copia binaria con buffer 4KB, preserva permisos, captura estadísticas
// Retorna: 0 éxito, -1 error
int sys_smart_copy_file(const char *src_path, const char *dest_path, stats_copia_t *stats) {
    int fd_src = -1;      /* Descriptor de archivo origen */
    int fd_dest = -1;     /* Descriptor de archivo destino */
    ssize_t bytes_leidos; /* Retorno de read() */
    ssize_t bytes_writtn; /* Retorno de write() */
    char buffer[BUFFER_SIZE]; /* Buffer de 4096 bytes: ¡la magia! */
    struct stat st;
    int resultado = 0;

    /* ====== VALIDACIÓN DE PARÁMETROS ====== */
    if (!src_path || !dest_path) {
        errno = EINVAL;
        return -1;
    }

    /* ====== ESTADÍSTICA 1: Marcar tiempo de inicio ====== */
    if (stats) {
        clock_gettime(CLOCK_MONOTONIC, &stats->tiempo_inicio);
    }

    /* ====== SYSCALL: stat() - Obtener información del archivo origen ====== 
     * 
     * ¿POR QUÉ stat()?
     * - Valida que el archivo existe
     * - Obtiene permisos (st_mode) para preservarlos
     * - Obtiene tamaño (st_size) para estadísticas
     * - ¡IMPORTANTE!: En un kernel real, esto es una syscall con overhead
     */
    if (stat(src_path, &st) == -1) {
        /* errno ya fue seteado por stat() */
        return -1;
    }

    /* ====== VALIDACIÓN: Rechazar directorios ====== */
    if (S_ISDIR(st.st_mode)) {
        errno = EISDIR;  /* Is a directory */
        return -1;
    }

    /* ====== SYSCALL: open() - LECTURA ====== 
     * 
     * PARÁMETROS:
     * - O_RDONLY: Abre solo para lectura (flags)
     * 
     * RETORNO:
     * - fd_src >= 0: Descriptor de archivo válido
     * - -1: Error (errno seteado)
     * 
     * OVERHEAD:
     * - Cambio de User Mode a Kernel Mode (Context Switch)
     * - Validación de permisos en kernel
     * - Búsqueda en tabla de inodos
     */
    fd_src = open(src_path, O_RDONLY);
    if (fd_src < 0) {
        /* errno ya fue seteado por open() */
        return -1;
    }

    /* ====== SYSCALL: open() - ESCRITURA ====== 
     * 
     * PARÁMETROS:
     * - O_WRONLY: Solo escritura
     * - O_CREAT: Crear si no existe
     * - O_TRUNC: Truncar (sobrescribir) si existe
     * - st.st_mode: Preservar permisos del archivo original
     * 
     * EJEMPLO:
     * Si original es: -rw-r--r-- (0644)
     * Entonces dest será: -rw-r--r-- (0644)
     */
    fd_dest = open(dest_path, O_WRONLY | O_CREAT | O_TRUNC, st.st_mode);
    if (fd_dest < 0) {
        close(fd_src); /* Limpiar recurso antes de salir */
        return -1;
    }

    /* ====== LOOP PRINCIPAL: Transferencia de datos ====== 
     * 
     * MECÁNICA:
     * 1. read() intenta leer hasta BUFFER_SIZE bytes del fd_src
     * 2. Si bytes_leidos > 0, escribimos ese bloque en fd_dest
     * 3. Si bytes_leidos == 0, hemos llegado a EOF
     * 4. Si bytes_leidos < 0, error durante lectura
     * 
     * IMPACTO DE RENDIMIENTO:
     * Archivo de 10 MB:
     * - Sin buffer (1B por syscall): ~10,000,000 syscalls (LENTÍSIMO)
     * - Con buffer 4KB: ~2,441 syscalls (razonable)
     * - Con stdio fread (8KB buffer): ~1,220 syscalls
     */
    while ((bytes_leidos = read(fd_src, buffer, BUFFER_SIZE)) > 0) {
        
        /* ====== SYSCALL: write() - Escribir bloque ====== */
        bytes_writtn = write(fd_dest, buffer, bytes_leidos);
        
        if (bytes_writtn != bytes_leidos) {
            /* Error: No se escribió todo lo que debía */
            /* errno será ENOSPC (sin espacio), EBADF (descriptor inválido), etc. */
            resultado = -1;
            break;
        }
        
        /* ====== ESTADÍSTICA: Contar bytes y syscalls ====== */
        if (stats) {
            stats->bytes_copiados += bytes_writtn;
            stats->syscalls_realizadas += 2; /* 1 read + 1 write */
        }
    }

    /* ====== VERIFICACIÓN FINAL DE LA LECTURA ====== */
    if (bytes_leidos < 0) {
        /* Error durante read(), errno ya está seteado */
        resultado = -1;
    }

    /* ====== LIMPIEZA: Cerrar descriptores ====== 
     * 
     * IMPORTANTE: SIEMPRE cerrar archivos abiertos, incluso si hay error.
     * (Válido incluso si fd ya es -1)
     */
    close(fd_src);
    close(fd_dest);

    /* ====== ESTADÍSTICA FINAL ====== */
    if (stats) {
        clock_gettime(CLOCK_MONOTONIC, &stats->tiempo_fin);
        stats->archivos_procesados++;
    }

    return resultado;
}

// === Copia recursiva de directorio ===
// Autor: Valeria Hornung U
int sys_smart_copy_directory(const char *src_path, const char *dest_path,
                             stats_copia_t *stats) {
    struct stat st, next_st;
    DIR *dir;
    struct dirent *entry;
    char next_src[MAX_PATH_LENGTH], next_dest[MAX_PATH_LENGTH];
    int len_src, len_dest;

    if (!src_path || !dest_path) {
        errno = EINVAL;
        return -1;
    }

    // Validar que origen es directorio
    if (stat(src_path, &st) == -1 || !S_ISDIR(st.st_mode)) {
        if (!S_ISDIR(st.st_mode)) errno = ENOTDIR;
        return -1;
    }

    // Crear directorio destino (ignorar si ya existe)
    if (mkdir_portable(dest_path, st.st_mode) == -1) {
        if (errno != EEXIST) return -1;
    } else if (stats) {
        stats->directorios_procesados++;
    }

    // Abrir y procesar directorio
    dir = opendir(src_path);
    if (!dir) return -1;

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        // Construir rutas completas
        len_src = snprintf(next_src, MAX_PATH_LENGTH, "%s/%s", src_path, entry->d_name);
        len_dest = snprintf(next_dest, MAX_PATH_LENGTH, "%s/%s", dest_path, entry->d_name);

        if (len_src >= MAX_PATH_LENGTH || len_dest >= MAX_PATH_LENGTH) {
            fprintf(stderr, "[ERROR] Ruta demasiado larga: %s\n", entry->d_name);
            continue;
        }

        // lstat: obtener info sin seguir symlinks (previene ciclos)
        if (lstat(next_src, &next_st) == -1) {
            perror("lstat");
            continue;
        }

        if (S_ISDIR(next_st.st_mode)) {
            sys_smart_copy_directory(next_src, next_dest, stats);
        } else if (S_ISREG(next_st.st_mode)) {
            if (sys_smart_copy_file(next_src, next_dest, stats) == 0) {
                printf("[OK] Archivo copiado: %s -> %s\n", next_src, next_dest);
            } else {
                fprintf(stderr, "[ERROR] No se pudo copiar %s: %s\n", next_src, strerror(errno));
            }
        } else {
            printf("[INFO] Tipo especial ignorado: %s\n", next_src);
        }
    }

    closedir(dir);
    return 0;
}

// Validar ruta origen (Valeria Hornung U)
int validar_ruta_origen(const char *ruta, int *es_directorio) {
    struct stat st;
    if (!ruta || !es_directorio) { errno = EINVAL; return -1; }
    if (strlen(ruta) >= MAX_PATH_LENGTH) { errno = ENAMETOOLONG; return -1; }
    if (stat(ruta, &st) == -1) return -1;
    *es_directorio = S_ISDIR(st.st_mode) ? 1 : 0;
    return 0;
}

// Validar ruta destino: verificar que directorio padre sea escribible
int validar_ruta_destino(const char *ruta) {
    struct stat st;
    if (!ruta) { errno = EINVAL; return -1; }
    if (strlen(ruta) >= MAX_PATH_LENGTH) { errno = ENAMETOOLONG; return -1; }
    
    char *parent = strdup(ruta);
    if (!parent) { errno = ENOMEM; return -1; }
    
    char *last_slash = strrchr(parent, '/');
    if (last_slash && last_slash != parent) *last_slash = '\0';
    else if (!last_slash) strcpy(parent, ".");
    else parent[1] = '\0';
    
    if (stat(parent, &st) == -1 || !S_ISDIR(st.st_mode)) {
        if (S_ISDIR(st.st_mode) == 0) errno = ENOTDIR;
        free(parent); return -1;
    }
    
    if (access(parent, W_OK) == -1) { free(parent); return -1; }
    free(parent);
    return 0;
}

// Calcular tiempo transcurrido en ms (Maximiliano Bustamante G)
double calcular_tiempo_transcurrido_ms(struct timespec inicio, struct timespec fin) {
    return (fin.tv_sec - inicio.tv_sec) * 1000.0 + (fin.tv_nsec - inicio.tv_nsec) / 1000000.0;
}

// Imprimir estadísticas de copia (Maximiliano Bustamante G)
void imprimir_estadisticas(const stats_copia_t *stats, const char *etiqueta) {
    if (!stats || !etiqueta) return;
    double tiempo_ms = calcular_tiempo_transcurrido_ms(stats->tiempo_inicio, stats->tiempo_fin);
    double tiempo_seg = tiempo_ms / 1000.0;
    double throughput_mbs = (stats->bytes_copiados / (1024.0 * 1024.0)) / tiempo_seg;
    printf("\n========== ESTADÍSTICAS: %s ==========\n", etiqueta);
    printf("Bytes copiados:  %ld (%.2f MB)\nSyscalls: %ld\nArchivos: %d | Dirs: %d\n", 
           stats->bytes_copiados, stats->bytes_copiados / (1024.0 * 1024.0),
           stats->syscalls_realizadas, stats->archivos_procesados, stats->directorios_procesados);
    printf("Tiempo: %.3f ms (%.6f s) | Throughput: %.2f MB/s\n========================================\n", 
           tiempo_ms, tiempo_seg, throughput_mbs);
}

/* ========================================================================== */
/* FUNCIÓN DE COMPARATIVA: sys_smart_copy vs fread/fwrite                    */
/* ========================================================================== */

// Comparar rendimiento fread vs sys_smart_copy (Maximiliano Bustamante G)
// Copia archivo 2x: con fread y con sys_smart_copy, compara tiempos
void comparar_rendimiento_fread_vs_smart_copy(const char *archivo_origen,
                                              const char *archivo_test_clib,
                                              const char *archivo_test_smart) {
    FILE *src_clib = NULL, *dst_clib = NULL;
    struct stat st;
    struct timespec inicio_clib, fin_clib, inicio_smart, fin_smart;
    char buffer_clib[8192];
    size_t bytes_leidos;
    double tiempo_clib_ms, tiempo_smart_ms;

    if (stat(archivo_origen, &st) == -1) { perror("stat"); return; }
    printf("\n[BENCHMARK] %s - %.2f MB\n", archivo_origen, st.st_size / (1024.0 * 1024.0));

    // Método 1: fread/fwrite (8KB buffer)
    src_clib = fopen(archivo_origen, "rb");
    dst_clib = fopen(archivo_test_clib, "wb");
    if (!src_clib || !dst_clib) { perror("fopen"); return; }
    clock_gettime(CLOCK_MONOTONIC, &inicio_clib);
    while ((bytes_leidos = fread(buffer_clib, 1, sizeof(buffer_clib), src_clib)) > 0)
        fwrite(buffer_clib, 1, bytes_leidos, dst_clib);
    clock_gettime(CLOCK_MONOTONIC, &fin_clib);
    fclose(src_clib);
    fclose(dst_clib);
    tiempo_clib_ms = calcular_tiempo_transcurrido_ms(inicio_clib, fin_clib);

    // Método 2: sys_smart_copy (4KB buffer)
    clock_gettime(CLOCK_MONOTONIC, &inicio_smart);
    sys_smart_copy_file(archivo_origen, archivo_test_smart, NULL);
    clock_gettime(CLOCK_MONOTONIC, &fin_smart);
    tiempo_smart_ms = calcular_tiempo_transcurrido_ms(inicio_smart, fin_smart);

    // Mostrar resultados
    printf("fread/fwrite (8KB):  %.3f ms\nsys_smart_copy (4KB): %.3f ms\n", 
           tiempo_clib_ms, tiempo_smart_ms);
    double diff = ((tiempo_smart_ms - tiempo_clib_ms) / tiempo_clib_ms) * 100;
    printf("Diferencia: %6.1f%%\n\n", diff);
}
