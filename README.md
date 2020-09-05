# TP1-SO
TP1 IPCs Sistemas Operativos - 2020

### Compilacion

Para poder compilar los archivos utilizar el comando:

    make all

### Ejecucion:

Para la ejecucion de los programas asegurarse de contar con archivos de extension .cnf que recibirá el programa "master" como parámetros. En caso de no proveerle ningun archivo el programa no producirá ninguna salida terminando su ejecucion. El programa "master" se encargará de procesar estos archivos. Utilice el comando:

    ./master archivo1.cnf ... archivoN.cnf

o tambien, en caso de tener un directorio con estos archivos puede:

    ./master nombreDirectorio/*

El programa envia por salida estandar la cantidad de tareas que tiene que resolver, y por ende, la cantidad de salidas que generará su ejecución. La salida 
es utilizada por el proceso "view".

Hay dos maneras de correr el programa

1. Pipear la salida del programa "master" al programa "view"  con el siguiente comando:

    ./master nombreDirectorio/* | ./view

2. Correr los procesos en dos terminales:

Terminal 1 
    ./master nombreDirectorio/*
    cantidadArchivos

Terminal 2

    ./vista cantidadArchivos

Por ejemplo supongamos un directorio con 50 archivos .cnf llamado "satFiles"

Luego debera:          Terminal 1                                               Terminal 2
                        ./master satFiles/*                                     ./vista 50
                        50

Para eliminar los archivos ejecutables basta con correr el comando:

    make clean

### Checkeo de memory leaks y testeo con Valgrind:

Para realizar los checkeos con pvs-studio y cppcheck correr el comando:

    make check

Si se desea correr algun caso de testeo con valgrind, se debe especificar dentro del makefile en la variable TF, el directorio donde se encuentran los archivos .cnf que se desean analizar y luego correr el comando:

    make test