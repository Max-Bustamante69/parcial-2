# ============================================================================
# ARCHIVO: Makefile
# ============================================================================
# DESCRIPCIÓN:
#     Script de construcción (build automation) para el proyecto
#     "Smart Backup Kernel-Space Utility".
#
#     Automatiza:
#     - Compilación de archivos C con flags recomendados
#     - Generación de directorios necesarios
#     - Limpieza de archivos generados
#     - Ejecución de pruebas
#
# PROYECTO: Sistemas Operativos - EAFIT (C2661-SI2004-5186)
# AUTORES:
#     - Maximiliano Bustamante G  (Optimizaciones de compilación)
#     - Valeria Hornung U         (Reglas de prueba y validación)
#
# FECHA: 2026
# ============================================================================

# ========== CONFIGURACIÓN DE COMPILADOR ==========
# CC: Compilador a usar (gcc o clang)
CC = gcc

# CFLAGS: Banderas de compilación
# -Wall:   Activar todos los warnings comunes
# -Wextra: Warnings adicionales para mejor calidad de código
# -g:      Incluir símbolos de debug (útil para gdb)
# -O2:     Optimización nivel 2 (buen balance rendimiento/tiempo compilación)
# -D_POSIX_C_SOURCE=200809L: Usar características POSIX 2008
CFLAGS = -Wall -Wextra -g -O2 -D_POSIX_C_SOURCE=200809L -Wpedantic

# LDFLAGS: Banderas del linker
# (Vacío por ahora, pero usamos librerías estándar compiladas en kernel)
LDFLAGS = 

# ========== ARCHIVOS OBJETIVO ==========
TARGET = smart-backup

# SOURCES: Archivos fuente a compilar
SOURCES = main.c backup_engine.c

# HEADERS: Archivos de cabecera
HEADERS = smart_copy.h

# OBJECTS: Archivos objeto generados (uno por cada .c)
OBJECTS = $(SOURCES:.c=.o)

# ========== DIRECTORIOS ESPECIALES ==========
TEST_DIR = test_files
BACKUP_DIR = backups
BIN_DIR = bin

# ========== REGLAS PRINCIPALES ==========

# Regla por defecto: construir el ejecutable
all: $(TARGET)
	@echo ""
	@echo "✓ Compilación completada exitosamente"
	@echo "  Ejecutable: $(TARGET)"
	@echo ""
	@echo "Uso: ./$(TARGET) --help"
	@echo ""

# Construir el ejecutable desde archivos objeto
$(TARGET): $(OBJECTS)
	@echo "[Linker] Generando $(TARGET)..."
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) $(LDFLAGS)

# Compilar archivos .c a .o
%.o: %.c $(HEADERS)
	@echo "[Compilando] $< → $@"
	$(CC) $(CFLAGS) -c $< -o $@

# ========== REGLAS DE TESTING ==========

# Regla: setup (preparar entorno de prueba)
.PHONY: setup
setup:
	@echo "[Setup] Creando directorios de prueba..."
	mkdir -p $(TEST_DIR)
	mkdir -p $(BACKUP_DIR)
	mkdir -p $(BIN_DIR)
	@echo "✓ Directorios creados"

# Regla: test (ejecutar pruebas)
.PHONY: test
test: all setup
	@echo ""
	@echo "========================================="
	@echo "INICIANDO SUITE DE PRUEBAS"
	@echo "========================================="
	@echo ""
	
	@echo "[Test 1] Generar archivo de prueba (5 MB)"
	./$(TARGET) -g 5 $(TEST_DIR)/test_5mb.bin
	@echo ""
	
	@echo "[Test 2] Copiar archivo individual"
	./$(TARGET) -b $(TEST_DIR)/test_5mb.bin $(BACKUP_DIR)/test_5mb_backup.bin
	@echo ""
	
	@echo "[Test 3] Crear estructura de directorios de prueba"
	@echo "Creando archivos de prueba..."
	mkdir -p $(TEST_DIR)/directorio_prueba/subdir1
	mkdir -p $(TEST_DIR)/directorio_prueba/subdir2
	echo "Archivo 1 en raíz" > $(TEST_DIR)/directorio_prueba/archivo1.txt
	echo "Archivo 2 en raíz" > $(TEST_DIR)/directorio_prueba/archivo2.txt
	echo "Archivo en subdir1" > $(TEST_DIR)/directorio_prueba/subdir1/archivo3.txt
	echo "Archivo en subdir2" > $(TEST_DIR)/directorio_prueba/subdir2/archivo4.txt
	@echo ""
	
	@echo "[Test 4] Copiar directorio completo"
	./$(TARGET) -b $(TEST_DIR)/directorio_prueba $(BACKUP_DIR)/directorio_backup
	@echo ""
	
	@echo "[Test 5] Comparativa de rendimiento (fread vs sys_smart_copy)"
	./$(TARGET) -c 10 $(TEST_DIR)/benchmark
	@echo ""
	
	@echo "========================================="
	@echo "SUITE DE PRUEBAS COMPLETADA"
	@echo "========================================="
	@echo ""
	@echo "Archivos de prueba en: $(TEST_DIR)/"
	@echo "Backups generados en:  $(BACKUP_DIR)/"
	@echo ""

# Regla: run (ejecutar programa con interfaz interactiva)
.PHONY: run
run: all
	@./$(TARGET) --help

# Regla: benchmark (solo ejecutar comparativa)
.PHONY: benchmark
benchmark: all setup
	@echo ""
	@echo "=== BENCHMARK: Comparativa fread vs sys_smart_copy ==="
	@echo ""
	./$(TARGET) -c 50 $(TEST_DIR)/bench_50mb.bin
	@echo ""

# ========== REGLAS DE LIMPIEZA ==========

# Regla: clean (limpiar archivos generados por compilación)
.PHONY: clean
clean:
	@echo "[Limpiando] Eliminando archivos objeto..."
	rm -f $(OBJECTS)
	@echo "[Limpiando] Eliminando ejecutable..."
	rm -f $(TARGET)
	@echo "✓ Limpieza completada"

# Regla: clean-all (limpieza profunda - incluyendo test files)
.PHONY: clean-all
clean-all: clean
	@echo "[Limpieza Total] Eliminando archivos de prueba..."
	rm -rf $(TEST_DIR)
	rm -rf $(BACKUP_DIR)
	rm -rf $(BIN_DIR)
	@echo "✓ Limpieza total completada"

# ========== REGLAS DE INFO ==========

# Regla: info (mostrar información del proyecto)
.PHONY: info
info:
	@echo ""
	@echo "╔════════════════════════════════════════════════╗"
	@echo "║ Smart Backup - Kernel-Space Utility (Simulado) ║"
	@echo "╚════════════════════════════════════════════════╝"
	@echo ""
	@echo "Proyecto: Parcial 2 - Sistemas Operativos (EAFIT)"
	@echo "Autores:  Maximiliano Bustamante G"
	@echo "          Valeria Hornung U"
	@echo "Curso:    C2661-SI2004-5186"
	@echo ""
	@echo "ARCHIVOS DEL PROYECTO:"
	@echo "  - main.c              Interfaz CLI"
	@echo "  - backup_engine.c     Motor de respaldo"
	@echo "  - smart_copy.h        Definiciones y tipos"
	@echo ""
	@echo "COMANDOS DISPONIBLES:"
	@echo "  make              Compilar el proyecto"
	@echo "  make run          Mostrar ayuda y uso"
	@echo "  make test         Ejecutar suite de pruebas"
	@echo "  make benchmark    Ejecutar comparativa de rendimiento"
	@echo "  make clean        Limpiar archivos de compilación"
	@echo "  make clean-all    Limpiar todo (incluyendo pruebas)"
	@echo "  make info         Mostrar esta información"
	@echo ""
	@echo "EJEMPLO DE USO:"
	@echo "  # Copiar archivo"
	@echo "  $ ./smart-backup -b archivo.txt backup.txt"
	@echo ""
	@echo "  # Copiar directorio"
	@echo "  $ ./smart-backup -b /ruta/origen /ruta/destino"
	@echo ""
	@echo "  # Benchmarking"
	@echo "  $ ./smart-backup -c 100 /tmp/test.bin"
	@echo ""

# ========== PHONY TARGETS ==========
# (.PHONY indica que estos targets no son nombres de archivos)
.PHONY: all test run benchmark clean clean-all info
