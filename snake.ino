#include <Wire.h> // Biblioteca para comunicação I2C
#include <Adafruit_GFX.h> // Biblioteca gráfica base
#include <Adafruit_SSD1306.h> // Driver do display OLED

#define SCREEN_WIDTH 128 // Largura do display
#define SCREEN_HEIGHT 64 // Altura do display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); // Inicializa o display

// Pinos dos botões
const int BTN_CIMA = 4, BTN_BAIXO = 5, BTN_ESQ = 6, BTN_DIR = 7, BUZZER = 3;

// Configurações do Jogo
const int TAM_BLOCO = 4; // Tamanho de cada segmento da cobra e da comida
int gameState = 0; // 0: Menu, 1: Jogando, 2: Game Over
int score = 0; // Pontuação

// Variáveis da Cobra
int cobraX[100], cobraY[100]; // Arrays para armazenar as coordenadas do corpo (máximo 100 segmentos)
int tamanhoCobra = 3; // Tamanho inicial da cobra
int dirX = 1, dirY = 0; // Direção inicial (movendo para a direita)

// Variáveis da Comida
int comidaX, comidaY;

// Controle de tempo
unsigned long tempoAnterior = 0;
int velocidade = 150; // Intervalo de atualização (ms) - quanto menor, mais rápido

void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Inicia o OLED no endereço 0x3C
  pinMode(BTN_CIMA, INPUT_PULLUP); pinMode(BTN_BAIXO, INPUT_PULLUP); // Configura botões
  pinMode(BTN_ESQ, INPUT_PULLUP); pinMode(BTN_DIR, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT); // Configura o som
  randomSeed(analogRead(0)); // Semente aleatória
}

// Função para colocar a comida em uma posição aleatória alinhada à grade
void gerarComida() {
  comidaX = random(12, 30) * TAM_BLOCO; // Gera X entre a margem do score e o fim da tela
  comidaY = random(0, 15) * TAM_BLOCO; // Gera Y dentro da altura da tela
}

// Reseta as variáveis para um novo jogo
void iniciarJogo() {
  score = 0;
  tamanhoCobra = 3;
  dirX = 1; dirY = 0;
  for(int i = 0; i < tamanhoCobra; i++) { // Posiciona o corpo inicial
    cobraX[i] = 60 - (i * TAM_BLOCO);
    cobraY[i] = 32;
  }
  gerarComida();
  gameState = 1;
}

void loop() {
  unsigned long tempoAtual = millis();

  if (gameState == 0) { // --- TELA INICIAL ---
    display.clearDisplay();
    display.setTextSize(2); display.setTextColor(WHITE);
    display.setCursor(35, 15); display.print("SNAKE");
    display.setTextSize(1); display.setCursor(32, 45);
    if ((tempoAtual / 500) % 2 == 0) display.print("PRESS START"); // Texto piscante
    display.display();
    if (digitalRead(BTN_DIR) == LOW || digitalRead(BTN_CIMA) == LOW) iniciarJogo(); // Inicia o jogo
  }

  else if (gameState == 1) { // --- JOGO ATIVO ---
    // Leitura dos botões (impede que a cobra volte diretamente sobre o próprio corpo)
    if (digitalRead(BTN_CIMA) == LOW && dirY == 0) { dirX = 0; dirY = -1; }
    if (digitalRead(BTN_BAIXO) == LOW && dirY == 0) { dirX = 0; dirY = 1; }
    if (digitalRead(BTN_ESQ) == LOW && dirX == 0) { dirX = -1; dirY = 0; }
    if (digitalRead(BTN_DIR) == LOW && dirX == 0) { dirX = 1; dirY = 0; }

    if (tempoAtual - tempoAnterior >= velocidade) {
      tempoAnterior = tempoAtual;

      // Move o corpo da cobra de trás para frente
      for (int i = tamanhoCobra - 1; i > 0; i--) {
        cobraX[i] = cobraX[i - 1];
        cobraY[i] = cobraY[i - 1];
      }

      // Move a cabeça
      cobraX[0] += dirX * TAM_BLOCO;
      cobraY[0] += dirY * TAM_BLOCO;

      // Colisão com as paredes
      if (cobraX[0] < 40 || cobraX[0] >= 128 || cobraY[0] < 0 || cobraY[0] >= 64) gameState = 2;

      // Colisão com o próprio corpo
      for (int i = 1; i < tamanhoCobra; i++) {
        if (cobraX[0] == cobraX[i] && cobraY[0] == cobraY[i]) gameState = 2;
      }

      // Colisão com a comida
      if (cobraX[0] == comidaX && cobraY[0] == comidaY) {
        score += 10;
        if (tamanhoCobra < 100) tamanhoCobra++; // Aumenta o tamanho
        tone(BUZZER, 1500, 20); // Som de comer
        gerarComida(); // Nova comida
        if (velocidade > 40) velocidade -= 1; // Aumenta a velocidade gradualmente
      }
    }

    // DESENHO
    display.clearDisplay();
    
    // Desenha borda da área de jogo (separando do score)
    display.drawRect(39, 0, 89, 64, WHITE);

    // Desenha a Cobra
    for (int i = 0; i < tamanhoCobra; i++) {
      display.fillRect(cobraX[i], cobraY[i], TAM_BLOCO - 1, TAM_BLOCO - 1, WHITE);
    }

    // Desenha a Comida
    display.fillCircle(comidaX + 2, comidaY + 2, 2, WHITE);

    // Score na lateral esquerda
    display.setTextSize(1);
    display.setCursor(2, 20); display.print("SCORE");
    display.setCursor(2, 32); display.print(score);

    display.display();
  }

  else if (gameState == 2) { // --- GAME OVER ---
    display.clearDisplay();
    display.setTextSize(2); display.setCursor(10, 15); display.print("GAME OVER");
    display.setTextSize(1); display.setCursor(40, 45); display.print("PTS: "); display.print(score);
    display.display();
    tone(BUZZER, 200, 500); // Som de derrota
    delay(2000);
    gameState = 0; // Volta ao menu
  }
}