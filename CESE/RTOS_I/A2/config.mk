# Compile options
VERBOSE=n
OPT=g
USE_NANO=y
SEMIHOST=n
USE_FPU=y

# Libraries
USE_LPCOPEN=y
USE_SAPI=y
USE_FREERTOS=n
FREERTOS_HEAP_TYPE=5
LOAD_INRAM=n

#con esta macro, se puede globalizar la defincion y cambiar la cantidad maxima
#de tareas planificadas para SEOS
DEFINES+= SCHEDULER_MAX_TASKS = 10

