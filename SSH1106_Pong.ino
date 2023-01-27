#include <GyverOLED.h>
#include "Joystick.h"
#include "pong.c"

const int GAME_OVER_SCORE = 10;
const int GAME_SPEED = 40;  //You can go up to 60

const int SCREEN_WIDTH = 128;
const int SCREEN_HEIGHT = 64;

unsigned long last_tick;

int cpu_score = 0;
int player_score = 0;
int state = 0;

GyverOLED<SSH1106_128x64> screen;

void tick(int fps){
  unsigned long fps_delay = 1000/fps;
  last_tick += fps_delay;
  unsigned long time_to_delay = last_tick - millis();
  unsigned long wait = max(0, time_to_delay);
  if ( wait > 0){
    delay(time_to_delay);
  }
  else{ 
    last_tick = millis();
  }
}

class Paddle {

 protected:
 void LimitMovement()
 {
   if(y < 0)
   {
     y = 0;
   }
   if( y+ height >= screen_height)
   {
     y = screen_height - height;
   }
 } 
 public:
 int x, y;
 int width, height;
 int speed;
 int screen_height;

  void Draw(){
    screen.rect(x, y, x+width, y+height, OLED_FILL);
  }

  void Update(char keyPressed)
  {
    if(keyPressed == 'U')
    {
      y -= speed;
    }
    if(keyPressed == 'D')
    {
      y += speed;
    }
    LimitMovement();
  }
};

class Ball{
  public:
  int x, y;
  int radius;
  int speed_x;
  int speed_y;

  void Draw(){
    screen.circle(x, y, radius, 1);
  }

  void Update(Paddle &player, Paddle &cpu) {
    x += speed_x;
    y += speed_y;

    if (y + radius >= SCREEN_HEIGHT - 1)
    {
        speed_y *= -1;
        y = SCREEN_HEIGHT - 1 - radius;
    }
    
    if( y - radius <= 1) {
      speed_y *= -1;
    }

    if (x + radius >= SCREEN_WIDTH - 1){
        cpu_score ++;
        ResetBall();
    }
    
    if( x - radius <= 1){
      player_score++;
      ResetBall();
    }

    if ((x - radius <= cpu.x + cpu.width) && (y + radius > cpu.y) && (y-radius < cpu.y + cpu.height))
    {
      speed_x *= -1;
    }

     if ((x + radius >= player.x) && (y + radius > player.y) && (y-radius < player.y + player.height))
    {
      speed_x *= -1;
    }
  }

  void ResetBall()
  {
    x = SCREEN_WIDTH/2;
    y = SCREEN_HEIGHT/2;
  }
};

class CpuPaddle :public Paddle{
  public:
  void Update(Ball &ball)
  {
    if (ball.y > y  && ball.speed_x < 0 && ball.x < SCREEN_WIDTH/2)
    {
      y += speed;
    }
    if (ball.y < y + height && ball.speed_x < 0 && ball.x < SCREEN_WIDTH/2)
    {
      y -= speed;
    }
    LimitMovement();
  }
};

void printScore(int x, int y, int score){
  screen.setCursorXY(x, y);
  screen.setScale(1);
  screen.print(score);
}

void gameOverScreen() {
  screen.clear();
  screen.rect(0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, OLED_STROKE);
  screen.setCursorXY(35, 10);
  screen.print("GAME OVER!");
  if(player_score >= cpu_score)
  {
    screen.setCursorXY(28, 28);
    screen.print("PLAYER WINS!");
  }else
  {
    screen.setCursorXY(38, 28);
    screen.print("CPU WINS!");
  }
  screen.update();
}

void startScreen() {
  screen.clear();

  screen.drawBitmap(0, 0, pong, 128, 64, BITMAP_INVERT, BUF_ADD);
  screen.update();
}

Ball ball;
Paddle player;
CpuPaddle cpu;
Joystick joystick;

void setup(void) {
  Serial.begin(9600);
  initDisplay();
  Wire.setClock(800000L);
  last_tick = millis();

  ball.x = SCREEN_WIDTH/2;
  ball.y = SCREEN_HEIGHT/2;
  ball.radius = 3;
  ball.speed_x = 2;
  ball.speed_y = 2;

  cpu.width = 2;
  cpu.height = 16;
  cpu.x = 3;
  cpu.y = (SCREEN_HEIGHT-1)/2 - cpu.height/2;
  cpu.speed = 2;
  cpu.screen_height = SCREEN_HEIGHT;

  player.width = 2;
  player.height = 16;
  player.x = (SCREEN_WIDTH-1) - player.width - 3;
  player.y = (SCREEN_HEIGHT-1)/2 - player.height/2;
  player.speed = 2;
  player.screen_height = SCREEN_HEIGHT;
  startScreen();
}

void loop() {

  joystick.ReadYaxis();
  Serial.println(joystick.keyPressed);

  if (state == 2) {
    gameOverScreen();
    if(joystick.keyPressed == 'U' || joystick.keyPressed == 'D')
    {
      state = 0;
      player_score = 0;
      cpu_score = 0;
    }

  }else if (state == 1)
  {
    screen.clear();
    screen.fastLineV(SCREEN_WIDTH/2, 0, SCREEN_HEIGHT, 1);
    ball.Update(player, cpu);
    cpu.Update(ball);
    player.Update(joystick.keyPressed);
    ball.Draw();
    player.Draw();
    cpu.Draw();
    printScore(SCREEN_WIDTH/4,2,cpu_score);
    printScore(3*SCREEN_WIDTH/4,2,player_score);
    screen.update();
    if(player_score == GAME_OVER_SCORE || cpu_score == GAME_OVER_SCORE)
    {
      state++;
    }
    
  }else if (state == 0)
  {
    Serial.println("State 0");
    if(joystick.keyPressed == 'U' || joystick.keyPressed == 'D')
    {
      state ++;
    }
  }
  tick(GAME_SPEED);
}

void initDisplay(){
  screen.init();  
  screen.clear();   
  screen.update(); 
}