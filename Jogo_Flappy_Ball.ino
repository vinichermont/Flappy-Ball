// INCLUIR BIBLIOTECAS
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <MusicBuzzer.h>

// DEFINIR PINOS
#define TFT_CS 38
#define TFT_RST 42
#define TFT_DC 40
#define BTN_PIN 2
#define BUZZER 10

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// DEFINIR PALETA DE CORES
#define ST7735_RED 0x001F
#define ST7735_ORANGE 0x00AA
#define ST7735_GREEN 0x07E0
#define ST7735_CYAN 0xFFE0
#define ST7735_MAGENTA 0xF81F
#define ST7735_YELLOW 0x07FF
#define ST7735_WHITE 0xFFFF
#define ST7735_LIGHTRED 0xFBEF
#define ST7735_BLACK 0x0000

// DEFINIR VALORES INICIAIS
// Tela
int screenW = 60;
int screenH = 140;

// Pássaro
float birdY = screenH / 4;
float oldBirdY = screenH / 4;
float velocity = 0;

// Cano
int pipeX = 160;
int oldPipeX = 160;
int gapY = random(10, 45);
int oldGapY = gapY;

int gapSize = 30;
int gapY2 = random(0,20);
int oldGapY2 = gapY2;

// Score
int score = 0;
bool gameOver = false;
bool hardMode = false;
bool scoredThisPipe = false;

// Menu
bool menu = true;
bool startGame = false;

// Efeito Sonoro
int scoredState = 0;
unsigned long scoredTimer = 0;
bool scoredPlaying = false;

void setup() {
  Serial.begin(9600);
  pinMode(BUZZER, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);

  music.init(BUZZER);

  tft.initR(INITR_MINI160x80);
  tft.setRotation(1); // ajusta orientação
  tft.fillScreen(ST7735_LIGHTRED);

  showMenu();
}


void loop() {
  // VERIFICAÇÕES
  // Modo Difícil
  if(score > 1 && pipeX == 0){
    hardMode = true;
  }

  // Tempo de Efeito Sonoro
  updateScoredSound();

  // Controles
  if (btnPressed()) {
      if(score>1){
        velocity = -3.7;
      }else{
        velocity = -2.5;
      }
  }

  // FÍSICA
  // Movimentação
  if (score>1){ 	// Hard Mode
    velocity += 0.70;
    birdY += velocity;
    pipeX -= 5;
  } else { 		// Easy Mode
    velocity += 0.30;
    birdY += velocity;
    pipeX -= 2;
  }

  if (pipeX < 0 && score <3 || pipeX < -350 && hardMode == true) {
    pipeX = 160;
    gapY = random(0, 25); // 45
    gapY2 = random(-20,20);
    scoredThisPipe = false;
  }

  // Colisão Easy Mode
  if (!hardMode && (birdY < gapY + 5 || birdY > gapY + gapSize - 5) && pipeX < 75 && pipeX > 65) {
    // && pipeX entre 6 e 19 (msm posição que o passaro)
    gameOver = true;
  }

  // Colisão Hard Mode
  if(hardMode==true){
    int i= 1;
    for(int n=15,m=5; n >= -225; n = n - 60){
      float ram = pow(-1,i);
      if ((birdY < gapY + ram*gapY2 + 5 || birdY > gapY + gapSize + ram*gapY2 - 5) && pipeX < n && pipeX > m ) {
        //ativo no modo dificil
        gameOver = true;
      } 
      m = m - 60; 
      i++;
     }
  }

  // Colisão do Limite de Tela
  if (birdY > (screenH+6) / 2 || birdY < 6) {
    gameOver = true;
  }

  // CONTAR PONTUAÇÃO
  // Pontuação Easy Mode
  if(!hardMode && !gameOver && !scoredThisPipe && pipeX <= 70 && pipeX >= 65){
    score++;
    scoredThisPipe = true;
    startScoredSound();
  }

  // Pontuação Hard Mode
  if(hardMode==true){
    for(int n=70; n >= -235; n = n - 60){
      if(pipeX == n && !gameOver){
        score++;
        scoredThisPipe = true;
        startScoredSound();
      }
    }
  }

  // GAME OVER
  if (gameOver) {
    tft.fillScreen(ST7735_BLACK);
    tft.setCursor(screenW/2, screenH/4);
    tft.setTextColor(ST7735_RED);
    tft.setTextSize(2);
    tft.print("GAME OVER");
    gameOver_Song();
    startGame = false;
    tft.fillScreen(ST7735_LIGHTRED);
    showMenu();
  }

  drawGame();
  delay(50);
}

// RETORNAR BOTÃO OU TECLADO
bool btnPressed(){
  if (Serial.available() > 0) {
    char tecla = Serial.read();
    
    if (tecla != 'p') {
      return true;
    }
  // Botão (pressionado = LOW)
  } else if (digitalRead(BTN_PIN) == LOW){
      return true;
  } else {
      return false;
  }
}

// TOCAR MÚSICA DE FIM DE JOGO
void gameOver_Song(){
  // Melodia da música de Game Over
  tone(BUZZER, 440, 300); // A4
  delay(350);
  tone(BUZZER, 392, 300); // G4
  delay(350);
  tone(BUZZER, 330, 300); // E4
  delay(350);
  tone(BUZZER, 262, 600); // C4
  delay(700);
}

// CALCULAR TEMPO PRA TOCAR EFEITO SONORO
void startScoredSound() {
  // Preparando Cálculo para Tocar Efeito Sonoro e não Travar o Jogo
  scoredPlaying = true;
  scoredState = 0;
  scoredTimer = millis();
}
void updateScoredSound() {
// Cálculo para Tocar Efeito Sonoro e não Travar o Jogo
  if (!scoredPlaying) return;

  switch (scoredState) {
    case 0:
      tone(BUZZER, 988); // B5
      scoredTimer = millis();
      scoredState = 1;
      break;

    case 1:
      if (millis() - scoredTimer >= 90) {
        tone(BUZZER, 1319); // E6
        scoredTimer = millis();
        scoredState = 2;
      }
      break;

    case 2:
      if (millis() - scoredTimer >= 120) {
        noTone(BUZZER);
        scoredPlaying = false;
      }
      break;
  }
}

// MOSTRAR MENU
void showMenu(){
  tft.setCursor(15, 30);
  tft.setTextColor(ST7735_WHITE);
  tft.setTextSize(2);
  tft.println("FLAPPY BALL");
  tft.setTextSize(1);
  tft.setCursor(40, 60);
  tft.println("Aperte o Botao");
  

  if(btnPressed()){
    startGame = true;
  }

  if(startGame){
    startGame = false;
    menu = false;
    resetGame();
  } else {
    showMenu();
  }
}

// ATUALIZAR QUADROS
void drawGame() {

  // 1. Apaga o Pássaro Antigo (Desenha de azul na posição velha)
  if ((int)oldBirdY != (int)birdY) {
    tft.fillCircle(70, (int)oldBirdY, 3, ST7735_LIGHTRED);
  }

  // 2. Apaga o Cano Antigo
  if (oldPipeX != pipeX) {
    tft.fillRect(oldPipeX, 0, 10, oldGapY, ST7735_LIGHTRED); // Apaga cano de cima
    tft.fillRect(oldPipeX, oldGapY + gapSize, 10, screenH, ST7735_LIGHTRED); // Apaga cano de baixo

    if (hardMode == true){
      // Laço que roda 5 vezes para APAGAR os próximos canos
      for (int i = 1; i <= 5; i++) {
        float ram= pow(-1,i);
        int posicaoX = oldPipeX + (i * 60); // Multiplica a distância
        tft.fillRect(posicaoX, 0, 10, oldGapY + ram*oldGapY2, ST7735_LIGHTRED);
        tft.fillRect(posicaoX, oldGapY + gapSize + ram*oldGapY2, 10, screenH, ST7735_LIGHTRED);
      }
    }
  }

  // 3. Desenha o Pássaro Novo
  tft.fillCircle(70, (int)birdY, 3, ST7735_YELLOW);

  // 4. Desenha o Cano Novo
  tft.fillRect(pipeX, 0, 10, gapY, ST7735_GREEN);
  tft.fillRect(pipeX, gapY + gapSize, 10, screenH, ST7735_GREEN);

  if (hardMode == true){
    // Laço que roda 5 vezes para DESENHAR os próximos canos
    for (int i = 1; i <= 5; i++) {
      float ram= pow(-1,i);
      int posicaoX = pipeX + (i * 60); // Multiplica a distância
      tft.fillRect(posicaoX, 0, 10, gapY + ram*gapY2 , ST7735_GREEN);
      tft.fillRect(posicaoX, gapY + gapSize + ram*gapY2, 10, screenH, ST7735_GREEN);
    }
  }

  // 5. Atualiza a Pontuação (Usando o truque da cor de fundo)
  tft.setCursor(2, 2);
  tft.setTextColor(ST7735_WHITE, ST7735_LIGHTRED); // Cor do texto e fundo
  tft.setTextSize(1);
  tft.print(score);
  // Imprime um espaço em branco extra caso a pontuação mude de casas
  tft.print(" ");

  // 6. Atualiza as variáveis Antigas para o Próximo Quadro
  oldBirdY = birdY;
  oldPipeX = pipeX;
  oldGapY = gapY;
  oldGapY2 = gapY2;
}

// REINICIAR O JOGO
void resetGame() {
  birdY = screenH / 4;
  oldBirdY = screenH / 4; // Reinicia a velha
  velocity = 0;

  pipeX = 160;
  oldPipeX = 160; // Reinicia a velha

  gapY = random(10, 45);
  oldGapY = gapY; // Reinicia a velha

  gapY2 = random(-20,20);
  oldGapY2 = gapY2;

  score = 0;
  gameOver = false;
  hardMode = false;
  scoredThisPipe = false;

  // Limpa a tela uma única vez ao reiniciar a partida
  tft.fillScreen(ST7735_LIGHTRED);
}