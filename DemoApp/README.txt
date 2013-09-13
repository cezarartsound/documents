Compilação:
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
		- Código de auxilio para a criação de um servidor
	- OBU.c:
		- OBU, com interface gráfica, apenas recebe através da MAC_API
		  e apresenta no ecrã os dados recebidos, com formato: 1 byte
		  com o valor 1 ou 2, que indica se foi emitido de um RSU ou de
		  outro OBU respectivamente, seguido de x bytes correspondentes
		  à string de velocidade, ex. "13 km/h\0"
	- RSU.c:
		- RSU, com servidor TCP que recebe os dados dos sensores SRS e
		  envia para RF através da MAC_API

Dependencias na execução:
	- Ficheiro da fonte: lutRS24.bdf
	- Ficheiro do background: back.bmp (possuí zonas encarnadas correspondentes
	  à zona não visivel no ecrã do ISEL com a resolução disponivel no linux
	  do TX27)

Pasta Demo:
	- Ambiente de execução
	- Ficheiros binário (RSU e OBU) estão compilados para a arquitectura do TX27

Outros aspectos importantes:
	- GraphicInterface/src/vga/vga.c, possuí a manipulação do frame buffer
	- No tx27 o framebuffer tinha a configuração: 640x480, 16bpp
	- O OBU deverá ser executado sobre um sistema da UA
	- O RSU deverá ser executado na máquina virtual do ISEL, flexivel a outras
	  sugestões