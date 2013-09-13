Compila��o:
	- Correr os Makefiles
 	- Necessario alterar a variavel CC dos Makefiles
	- Necessario re-gerar a biblioteca GraphicInterface (ficheiro .a), e colocar
	  o ficheiro gerado em AppSRS/src/GraphicInterface

Pasta AppSRS:
	- GraphicInterface:
		- Headers da biblioteca
		- Ficheiro .a (que deve ser substituido)
	- MAC_API: 
		- Headers e sources da API
	- so_tcplib:
		- C�digo de auxilio para a cria��o de um servidor
	- OBU.c:
		- OBU, com interface gr�fica, apenas recebe atrav�s da MAC_API
		  e apresenta no ecr� os dados recebidos, com formato: 1 byte
		  com o valor 1 ou 2, que indica se foi emitido de um RSU ou de
		  outro OBU respectivamente, seguido de x bytes correspondentes
		  � string de velocidade, ex. "13 km/h\0"
	- RSU.c:
		- RSU, com servidor TCP que recebe os dados dos sensores SRS e
		  envia para RF atrav�s da MAC_API

Dependencias na execu��o:
	- Ficheiro da fonte: lutRS24.bdf
	- Ficheiro do background: back.bmp (possu� zonas encarnadas correspondentes
	  � zona n�o visivel no ecr� do ISEL com a resolu��o disponivel no linux
	  do TX27)

Pasta Demo:
	- Ambiente de execu��o
	- Ficheiros bin�rio (RSU e OBU) est�o compilados para a arquitectura do TX27

Outros aspectos importantes:
	- GraphicInterface/src/vga/vga.c, possu� a manipula��o do frame buffer
	- No tx27 o framebuffer tinha a configura��o: 640x480, 16bpp
	- O OBU dever� ser executado sobre um sistema da UA
	- O RSU dever� ser executado na m�quina virtual do ISEL, flexivel a outras
	  sugest�es