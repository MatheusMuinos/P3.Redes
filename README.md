# P3.Redes — Resumen de programas y uso

Este repositorio contiene varios ejemplos y pruebas con sockets UDP (clientes y servidores). A continuación se listan los ficheros principales, su funcionalidad y comandos básicos de compilación/ejecución.

## Lista de ficheros y descripción breve

- 1a_emisor.c  
  Emisor UDP simple: lee una línea de stdin y la envía a destino.  
  Uso: emisor <puerto_local> <ip_destino> <puerto_destino>

- 1b_receptor.c  
  Receptor UDP simple: escucha en un puerto y muestra IP:puerto y contenido recibido.  
  Uso: receptor <puerto>

- 1c_emisor.c  
  Variante para comprobaciones: además de enviar stdin envía un datagrama grande (2000 B) para probar truncamiento.  
  Uso: emisor (igual que 1a) — incluye envío adicional grande.

- 1c_receptor.c  
  Receptor para la comprobación de truncamiento: usa recvmsg() con buffer pequeño y detecta MSG_TRUNC.  
  Uso: receptor <puerto>  
  Documento: 1c.md (explica la comprobación y resultados).

- 1d_emisor.c  
  Emisor que envía un array binario de floats en un único datagrama. Formato: [uint32_t count][float0][float1]... (count en network order).  
  Uso: emisor <puerto_local> <ip_destino> <puerto_destino> <num_floats>

- 1d_receptor.c  
  Receptor que extrae `count` del datagrama y muestra los floats recibidos; detecta truncamiento parcial.  
  Uso: receptor <puerto>  
  Documento: 1d.md (describe modificaciones y observaciones).

- 2_servidorUDP.c  
  Servidor UDP de mayúsculas: escucha datagramas, convierte su contenido a MAYÚSCULAS y responde al remitente.  
  Uso: servidorUDP <puerto>

- 2_clienteUDP.c  
  Cliente que lee un archivo de entrada línea por línea, envía cada línea al servidor, recibe la línea en mayúsculas y escribe en un archivo de salida cuyo nombre es el de entrada con sufijo `_MAYUS`.  
  Uso: clienteUDP <archivo_entrada> <ip_servidor> <puerto_servidor>  
  Nota: el cliente incluye `sleep()` entre envíos para pruebas de concurrencia y un timeout de recepción.

- 1c.md, 1d.md, 3.md  
  Documentación con resultados de pruebas y observaciones (truncamiento UDP, formato floats, pruebas con varios clientes).

## Compilación (ejemplos)

En WSL / Git Bash / MSYS2:
```
gcc -o emisor 1a_emisor.c
gcc -o receptor 1b_receptor.c
gcc -o servidorUDP 2_servidorUDP.c
gcc -o clienteUDP 2_clienteUDP.c
gcc -o emisor_1c 1c_emisor.c
gcc -o receptor_1c 1c_receptor.c
gcc -o emisor_1d 1d_emisor.c
gcc -o receptor_1d 1d_receptor.c
```

## Ejecución rápida (pruebas típicas)

- Servidor de mayúsculas:
  ./servidorUDP 12345

- Cliente que convierte un archivo:
  ./clienteUDP entrada.txt 127.0.0.1 12345  
  Resultado: archivo `entrada_MAYUS...` creado en la misma carpeta.

- Prueba truncamiento (1c):
  1) Ejecutar receptor_1c: ./receptor_1c 12345  
  2) Ejecutar emisor_1c: ./emisor_1c <puerto_local> 127.0.0.1 12345  
  Ver salida y 1c.md para detalles (MSG_TRUNC, bytes perdidos, imposibilidad de recuperar restante).

- Prueba arrays de float (1d):
  1) receptor_1d: ./receptor_1d 12345  
  2) emisor_1d: ./emisor_1d <puerto_local> 127.0.0.1 12345 <num_floats>  
  Ver 1d.md para formato y observaciones.

## Pruebas de concurrencia (varios clientes)

- Ejecutar servidor en Terminal A.  
- Ejecutar cliente1 en Terminal B (archivo con varias líneas). El cliente tiene un `sleep()` tras cada línea para permitir lanzar cliente2.  
- Mientras cliente1 duerme, lanzar cliente2 en Terminal C.  
- Observar en servidor mensajes intercalados con diferentes puertos origen; ambos clientes deben recibir respuestas.  
- Más detalles en 3.md.

## Notas importantes / recomendaciones

- UDP es datagramático y no fiable: datagramas grandes que no caben en el buffer son truncados y los bytes sobrantes se descartan. `recvmsg()` puede indicar truncamiento con `MSG_TRUNC`.  
- Tamaño máximo seguro de payload UDP ≈ 65507 bytes (sin considerar fragmentación IP). Evitar datagramas muy grandes o implementar fragmentación a nivel de aplicación.  
- Los ejemplos usan IPv4 (AF_INET). Ajustar si se requiere IPv6.  
- Si pruebas en la misma máquina use 127.0.0.1; en red real, revisa cortafuegos/NAT.  
- Ajustar buffers, timeouts y sleep según la prueba que quieras realizar.

Si quieres, genero automáticamente versiones README separadas por cada carpeta o añado instrucciones para Windows sin WSL (PowerShell + MinGW).