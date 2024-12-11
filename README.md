# Atividade 7 - OpenGL - Snake-Game
### Sobre o jogo
Foi desenvolvido o jogo Snake utilizando a API gráfica OpenGL. O jogo permite o controle da cobrinha pelas seguintes teclas:
- W, A, S, D: Movimentação.
- R: Reinicia o jogo.
- P: Pausa o jogo. Para retomar, basta pressionar qualquer tecla diferente de P.
- Z e X: Ajustam o arrasto da cobrinha, permitindo que o jogo fique mais lento ou mais rápido.

O framerate do jogo está limitado a, no máximo, 60 FPS.

### Sobre os arquivos / dependências 

- O jogo pode ser testado executanto o arquivo '**Snake.exe**'
- Segue um vídeo demonstrativo do jogo '**video do jogo funcionando.mp4**'

Importante:

- O jogo foi testado tanto em Linux quanto em Windows. No entanto, foram evitadas bibliotecas específicas de sistemas operacionais com o objetivo de melhorar a portabilidade entre diferentes plataformas.
# Linux
- Instalação:
``` bash
sudo apt-get update
sudo apt-get upgrade
sudo apt-get install freeglut3
sudo apt-get install freeglut3-dev
sudo apt-get install binutils-gold
sudo apt-get install g++ cmake
sudo apt-get install libglew-dev
sudo apt-get install g++
sudo apt-get install mesa-common-dev
sudo apt-get install build-essential
sudo apt-get install libglew1.5-dev libglm-dev
```
- Compilação / Execução
Para compilar  é importante sar a flag **-std=c++11** caso contrário não funcionará.
``` bash
g++ -std=c++11 main.cpp -lGL -lGLU -lGLEW -lglut -o snakeGame
./snakeGame
```

# Windows
- A versão compilada para Windows (em anexo) utiliza OpenGL, mais especificamente a biblioteca **freeglut** [freeglut-MinGW-3.0.0-1.mp] pois oferece uma ampla portabilidade. Os arquivos da API gráfica podem ser baixados através do seguinte link: [https://www.transmissionzero.co.uk/software/freeglut-devel/](https://www.transmissionzero.co.uk/software/freeglut-devel/) ou especificamente a versão utilizado em [https://www.transmissionzero.co.uk/files/software/development/GLUT/freeglut-MinGW.zip](https://www.transmissionzero.co.uk/files/software/development/GLUT/freeglut-MinGW.zip).
- O arqivo do projeto segue no **codeblocks**, para funcionar no Windows com o freeglut basta colocar os arquivos disponíveis no link / anexo[pasta freeglut]:

- **Lib do freeglut** em C:\Program Files (x86)\CodeBlocks\MinGW\lib

- **Bin do freeglut** em C:\Program Files (x86)\CodeBlocks\MinGW\bin

- **Includes do freeglut** em C:\Program Files (x86)\CodeBlocks\MinGW\include [PODE EXISTIR A PASTA GL JÁ, SUBSTITUA OS ARQUIVOS PRESENTES NELA]

- Importante: A versão de compilação é de 32 bits, vai funcionar para 64 bits também. 
---
---
---

# Informações relevantes
Há na main.cpp algumas linhas referentes a drivers gráficos, com elas é possível habilitas a placa de vídeo tanto da Nvidea quanto da AMD, em sitações em que há briga de renderização por efifiência energética, extraindo mais desempenho da placa gráfica. Um exemplo claro disso é em notbooks que tem optimus ou MUX Switch.

Porém isso só funciona no Windows.

```bash
extern "C"{
	__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001; ///habilitar placa nvidea
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;///habilitar placa AMD
}
```