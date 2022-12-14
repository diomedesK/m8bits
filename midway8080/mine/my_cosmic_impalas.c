#include <stdio.h>
#include <string.h>

#define LO_CHAR 0x20
#define HI_CHAR 0x5e

#define SCR_X 28
#define SCR_Y 32
#define MAX_X (28*8) - 27
#define MIN_X 0

#define MAX_ENEMIES 28
#define PLAYER_STEP 2
#define ALIEN_STEP_X 4

void main();

void start(){
  __asm
    LD	SP, #0x2400;
  DI;
  __endasm;

  main();
}

char __hld[30];

typedef unsigned char byte;
typedef signed char sbyte;
typedef unsigned short word;
typedef enum { false, true } boolean;


volatile __sfr __at (0x0) input0;
volatile __sfr __at (0x1) input1;
volatile __sfr __at (0x2) input2;
__sfr __at (0x2) bitshift_offset;
volatile __sfr __at (0x3) bitshift_read;
__sfr __at (0x4) bitshift_value;
__sfr __at (0x6) watchdog_strobe;

byte __at (0x2400) vidmem[SCR_X * 8 ][SCR_Y]; // 256x224x1 video memory

byte g_frame;
byte g_enemycount;
byte g_enemyindex;
byte g_lives;


struct {
  byte X;
  byte Y;
} Player;

struct {
  byte X;
  byte Y;
} Bullet;

struct {
  byte X;
  byte Y;
  boolean launched:1;
} Bomb;

typedef struct {
  byte X;
  byte Y;
  const byte* sprite;
} Enemy;

typedef struct {
  byte right:1;
  byte down:1;
} MarchMode;

MarchMode g_currentMode, g_nextMode;

Enemy enemies[MAX_ENEMIES];

#define FIRE1 (input1 & 0x10)
#define LEFT1 (input1 & 0x20)
#define RIGHT1 (input1 & 0x40)
#define COIN1 (input1 & 0x1)
#define START1 (input1 & 0x4)
#define START2 (input1 & 0x2)

const byte font8x8[HI_CHAR - LO_CHAR + 1][8] = {
  { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 }, { 0x00,0x00,0x00,0x79,0x79,0x00,0x00,0x00 }, { 0x00,0x70,0x70,0x00,0x00,0x70,0x70,0x00 }, { 0x14,0x7f,0x7f,0x14,0x14,0x7f,0x7f,0x14 }, { 0x00,0x12,0x3a,0x6b,0x6b,0x2e,0x24,0x00 }, { 0x00,0x63,0x66,0x0c,0x18,0x33,0x63,0x00 }, { 0x00,0x26,0x7f,0x59,0x59,0x77,0x27,0x05 }, { 0x00,0x00,0x00,0x10,0x30,0x60,0x40,0x00 }, { 0x00,0x00,0x1c,0x3e,0x63,0x41,0x00,0x00 }, { 0x00,0x00,0x41,0x63,0x3e,0x1c,0x00,0x00 }, { 0x08,0x2a,0x3e,0x1c,0x1c,0x3e,0x2a,0x08 }, { 0x00,0x08,0x08,0x3e,0x3e,0x08,0x08,0x00 }, { 0x00,0x00,0x00,0x03,0x03,0x00,0x00,0x00 }, { 0x00,0x08,0x08,0x08,0x08,0x08,0x08,0x00 }, { 0x00,0x00,0x00,0x03,0x03,0x00,0x00,0x00 }, { 0x00,0x01,0x03,0x06,0x0c,0x18,0x30,0x20 }, { 0x00,0x3e,0x7f,0x49,0x51,0x7f,0x3e,0x00 }, { 0x00,0x01,0x11,0x7f,0x7f,0x01,0x01,0x00 }, { 0x00,0x23,0x67,0x45,0x49,0x79,0x31,0x00 }, { 0x00,0x22,0x63,0x49,0x49,0x7f,0x36,0x00 }, { 0x00,0x0c,0x0c,0x14,0x34,0x7f,0x7f,0x04 }, { 0x00,0x72,0x73,0x51,0x51,0x5f,0x4e,0x00 }, { 0x00,0x3e,0x7f,0x49,0x49,0x6f,0x26,0x00 }, { 0x00,0x60,0x60,0x4f,0x5f,0x70,0x60,0x00 }, { 0x00,0x36,0x7f,0x49,0x49,0x7f,0x36,0x00 }, { 0x00,0x32,0x7b,0x49,0x49,0x7f,0x3e,0x00 }, { 0x00,0x00,0x00,0x12,0x12,0x00,0x00,0x00 }, { 0x00,0x00,0x00,0x13,0x13,0x00,0x00,0x00 }, { 0x00,0x08,0x1c,0x36,0x63,0x41,0x41,0x00 }, { 0x00,0x14,0x14,0x14,0x14,0x14,0x14,0x00 }, { 0x00,0x41,0x41,0x63,0x36,0x1c,0x08,0x00 }, { 0x00,0x20,0x60,0x45,0x4d,0x78,0x30,0x00 }, { 0x00,0x3e,0x7f,0x41,0x59,0x79,0x3a,0x00 }, { 0x00,0x1f,0x3f,0x68,0x68,0x3f,0x1f,0x00 }, { 0x00,0x7f,0x7f,0x49,0x49,0x7f,0x36,0x00 }, { 0x00,0x3e,0x7f,0x41,0x41,0x63,0x22,0x00 }, { 0x00,0x7f,0x7f,0x41,0x63,0x3e,0x1c,0x00 }, { 0x00,0x7f,0x7f,0x49,0x49,0x41,0x41,0x00 }, { 0x00,0x7f,0x7f,0x48,0x48,0x40,0x40,0x00 }, { 0x00,0x3e,0x7f,0x41,0x49,0x6f,0x2e,0x00 }, { 0x00,0x7f,0x7f,0x08,0x08,0x7f,0x7f,0x00 }, { 0x00,0x00,0x41,0x7f,0x7f,0x41,0x00,0x00 }, { 0x00,0x02,0x03,0x41,0x7f,0x7e,0x40,0x00 }, { 0x00,0x7f,0x7f,0x1c,0x36,0x63,0x41,0x00 }, { 0x00,0x7f,0x7f,0x01,0x01,0x01,0x01,0x00 }, { 0x00,0x7f,0x7f,0x30,0x18,0x30,0x7f,0x7f }, { 0x00,0x7f,0x7f,0x38,0x1c,0x7f,0x7f,0x00 }, { 0x00,0x3e,0x7f,0x41,0x41,0x7f,0x3e,0x00 }, { 0x00,0x7f,0x7f,0x48,0x48,0x78,0x30,0x00 }, { 0x00,0x3c,0x7e,0x42,0x43,0x7f,0x3d,0x00 }, { 0x00,0x7f,0x7f,0x4c,0x4e,0x7b,0x31,0x00 }, { 0x00,0x32,0x7b,0x49,0x49,0x6f,0x26,0x00 }, { 0x00,0x40,0x40,0x7f,0x7f,0x40,0x40,0x00 }, { 0x00,0x7e,0x7f,0x01,0x01,0x7f,0x7e,0x00 }, { 0x00,0x7c,0x7e,0x03,0x03,0x7e,0x7c,0x00 }, { 0x00,0x7f,0x7f,0x06,0x0c,0x06,0x7f,0x7f }, { 0x00,0x63,0x77,0x1c,0x1c,0x77,0x63,0x00 }, { 0x00,0x70,0x78,0x0f,0x0f,0x78,0x70,0x00 }, { 0x00,0x43,0x47,0x4d,0x59,0x71,0x61,0x00 }, { 0x00,0x00,0x7f,0x7f,0x41,0x41,0x00,0x00 }, { 0x00,0x20,0x30,0x18,0x0c,0x06,0x03,0x01 }, { 0x00,0x00,0x41,0x41,0x7f,0x7f,0x00,0x00 }, { 0x00,0x08,0x18,0x3f,0x3f,0x18,0x08,0x00 }
};

/* Sprites have their width (in bytes) and height (in bits) defined in the
   first position of the array
   */
const byte player_bitmap[] =
{2,27,/*{w:16,h:27,bpp:1,xform:"rotate(-90deg)"}*/0x0,0x0,0x0,0x0,0x0f,0x00,0x3e,0x00,0xf4,0x07,0xec,0x00,0x76,0x00,0x2b,0x00,0x33,0x00,0x75,0x00,0xf5,0x00,0xeb,0x31,0xbf,0xef,0x3f,0xcf,0xbf,0xef,0xeb,0x31,0xf5,0x00,0x75,0x00,0x33,0x00,0x2b,0x00,0x76,0x00,0xec,0x00,0xf4,0x07,0x3e,0x00,0x0f,0x00,0x00,0x00,0x0,0x0};
const byte bomb_bitmap[] =
{1,5,/*{w:8,h:5,bpp:1,xform:"rotate(-90deg)"}*/0x88,0x55,0x77,0x55,0x88};
const byte bullet_bitmap[] =
{2,2,/*{w:16,h:2,bpp:1,xform:"rotate(-90deg)"}*/0x88,0x88,0x44,0x44};
const byte enemy1_bitmap[] =
{2,16,/*{w:16,h:17,bpp:1,xform:"rotate(-90deg)"}*/0x00,0x00,0x00,0x0c,0x04,0x1e,0x46,0x3f,0xb8,0x7f,0xb0,0x7f,0xba,0x7f,0xfd,0x3f,0xfc,0x07,0xfc,0x07,0xfd,0x3f,0xba,0x7f,0xb0,0x7f,0xb8,0x7f,0x46,0x3f,0x04,0x1e,0x00,0x0c};
const byte enemy2_bitmap[] =
{2,16,/*{w:16,h:16,bpp:1,xform:"rotate(-90deg)"}*/0x26,0x00,0x59,0x10,0x10,0x30,0x33,0x18,0xe6,0x61,0xc4,0x56,0x03,0x03,0xdc,0x03,0xdc,0x03,0x03,0x03,0xc4,0x56,0xe6,0x61,0x33,0x18,0x10,0x30,0x59,0x10,0x26,0x00};
const byte enemy3_bitmap[] =
{2,16,/*{w:16,h:16,bpp:1,xform:"rotate(-90deg)"}*/0x80,0x1f,0xc0,0x03,0xf8,0x3f,0x70,0x00,0xf0,0x01,0xfc,0x07,0xe8,0x01,0xf8,0x03,0xf8,0x03,0xe8,0x01,0xf8,0x07,0xf0,0x01,0x70,0x00,0xf8,0x3f,0xc0,0x03,0x80,0x1f};
const byte enemy4_bitmap[] =
{2,16,/*{w:16,h:16,bpp:1,xform:"rotate(-90deg)"}*/0x06,0x00,0x0c,0x00,0x28,0x00,0x70,0x1f,0x84,0x3f,0xde,0x37,0xbb,0x3f,0xf0,0x3f,0xf0,0x3f,0xbb,0x3f,0xde,0x37,0x84,0x3f,0x70,0x1f,0x28,0x00,0x0c,0x00,0x06,0x00};

const byte* const enemies_bitmap[4] = {
  enemy1_bitmap,
  enemy2_bitmap,
  enemy3_bitmap,
  enemy4_bitmap,
};

void clrscr(){
  memset(vidmem, 0, sizeof(vidmem));
}

void xorpixel(byte x, byte y){
  byte *dest = &vidmem[x][y >> 3];
  *dest = 0x1 << (y&7);
}

void drawvline(byte x, byte y1, byte y2) {
  /*CC*/
  byte yb1 = y1/8;
  byte yb2 = y2/8;
  byte* dest = &vidmem[x][yb1];
  signed char nbytes = yb2 - yb1;
  *dest++ ^= 0xff << (y1&7);
  if (nbytes > 0) {
    while (--nbytes > 0) {
      *dest++ ^= 0xff;
    }
    *dest ^= 0xff >> (~y2&7);
  } else {
    *--dest ^= 0xff << ((y2+1)&7);
  }
}


void drawchar(char ch, byte x, byte y){
  byte i; 
  const byte *sprite = &font8x8[ ch - LO_CHAR ][0];
  byte *dest = &vidmem[x*8][y];

  for( i = 0; i < 8; i++ ){
    *dest = *sprite++;
    dest += 32; //because screen is a 28*32 grid duh
  }
}

void drawstr( char* str, byte x, byte y){
  for( ; *str != 0 ; str++ ){
    drawchar(*str, x++, y);
  }
}


void drawsprite( const byte *src, byte x, byte y ){
  byte i, j;
  /* Always consider that the screen is rotated, the so called 'width'
     here is in fact it's height, and the opposite is also true
     */

  byte *dest = &vidmem[x][y];
  byte width = *src++;
  byte height = *src++; 

  for (i=0; i< height; i++){
    for (j=0; j<width; j++) {
      *dest++ = *src++;
    }
    dest += 32 - width;	//32 - width because it's the difference needed to advance to the next byte
  }

}

void clearsprite( const byte *src, byte x, byte y){
  /*  Clear sprite will remove anything that is found on the target area */
  byte i, j;
  byte *dest = &vidmem[x][y];
  byte width = *src++;
  byte height = *src++;

  for( i = 0; i < height; i++){
    for( j = 0; j < width; j++){
      *dest++ = 0; 
    }
    dest += 32 - width;
  }
}

void erasesprite( const byte *src, byte x, byte y){
  /* Erase sprite will only remove the bytes that are part of the sprite from
     the target area, by using and AND of the target with a NOT operation on
     the source bitmap */
  byte i, j;
  byte *dest = &vidmem[x][y];
  byte width = *src++;
  byte height = *src++;

  for( i = 0; i < height; i++){
    for( j = 0; j < width; j++){
      *dest++ &= ~(*src++);
    }
    dest += 32 - width;
  }
}

byte xorsprite( const byte *src, byte x, byte y){
  byte i, j;
  byte *dest = &vidmem[x][y];
  byte width = *src++;
  byte height = *src++;

  byte result = 0;

  for( i = 0; i < height; i++){
    for( j = 0; j < width; j++){
      result |= (*dest++ ^= *src++);
    }
    dest += 32 - width;
  }

  return result;
}

void drawbunker(byte x, byte y, byte y2, byte h, byte w) {
  /*CC*/
  byte i;
  for (i=0; i<h; i++) {
    drawvline(x+i, y+i, y+y2+i*2);
    drawvline(x+h*2+w-i-1, y+i, y+y2+i*2);
  }
  for (i=0; i<w; i++) {
    drawvline(x+h+i, y+h, y+y2+h*2);
  }
}



void drawlives(){
  byte x, i;
  x = 27;
  drawchar(0, 27, 31);
  drawchar(0, 26, 31);
  drawchar(0, 25, 31);
  
  for( i = 0; i < g_lives; i++){
    drawchar('*', x--, 31);
  } 
}


void drawground(){
  byte i;
  for( i = 0; i < 28; i++){
    drawchar(30, i, 0);
  }
}

void drawshields(){
  byte i;
  i;
}

void firebullet(){
  Bullet.X = Player.X + 13;
  Bullet.Y = 3;
  xorsprite(bullet_bitmap, Bullet.X, Bullet.Y );  
}

void destroyplayer(){
  byte i;
  
  g_lives--;
  drawlives();
  
  for( i = 0; i < 50; i++){
    drawsprite(player_bitmap, Player.X, Player.Y);
    clearsprite(player_bitmap, Player.X, Player.Y);
    watchdog_strobe = 0;
  }

  Player.X = 100;
  
  if(g_lives) xorsprite(player_bitmap, Player.X, Player.Y);
 
}

void dropbomb(){

  Enemy *e = &enemies[g_enemyindex];
  Bomb.X = e->X + 13;
  Bomb.Y = e->Y - 2;
  
   
  Bomb.launched = true; 
  xorsprite(bomb_bitmap, Bomb.X, Bomb.Y);
  
}



void movebomb(){
  
  byte leftover = xorsprite(bomb_bitmap, Bomb.X, Bomb.Y); 
  if (Bomb.Y < 2) {
    Bomb.launched = false;
  } else if (leftover) {
    erasesprite(bomb_bitmap, Bomb.X, Bomb.Y); 
    if (Bomb.Y < 3) {
      destroyplayer();
    }
    Bomb.launched = false;
  } else {
    Bomb.Y--;
    xorsprite(bomb_bitmap, Bomb.X, Bomb.Y);
  }

}

void checkhit();

void movebullet(){

  byte leftover = xorsprite(bullet_bitmap, Bullet.X, Bullet.Y );  
  if( leftover || Bullet.Y > 26 ){ //26 = MAX HEIGHT FOR BULLET

    clearsprite(bullet_bitmap, Bullet.X, Bullet.Y);
    checkhit();
    Bullet.Y = 0;
  } else {
    xorsprite(bullet_bitmap, Bullet.X, ++Bullet.Y );  
  }

} 



byte g_score;


void drawscore(){
  static char _scorestr[28];
  sprintf(_scorestr, "SCORE %02d", g_score );
  drawstr(_scorestr, 0, 31);
}

void nextmode(){
  memcpy( &g_currentMode, &g_nextMode, sizeof(g_nextMode) );
}

void updateenemies(){
  /* One enemy is updated per round, for performance reasons */
  Enemy *e; 
  boolean debug = true;
  debug = false;

  
  
  if(g_enemyindex >= g_enemycount ){
    g_enemyindex = 0;
    nextmode();
    return;
  }
  
  e = &enemies[g_enemyindex];
  
  clearsprite(e->sprite, e->X, e->Y);

  if ( g_currentMode.down ){
    if( e->Y < 4) g_lives = 0;

    e->Y--;
    g_nextMode.down = 0;
    

  } else {

    if ( g_currentMode.right ){
      e->X+= ALIEN_STEP_X;
      if( e->X + ALIEN_STEP_X >= 200 ){
        g_nextMode.right = 0;
        g_nextMode.down = 1;
      } 
      
    } else {
      e->X-= ALIEN_STEP_X;
      if( e->X <= 2 ){
        g_nextMode.right = 1;
        g_nextMode.down = 1;              
      } 
      
    }
  }

  drawsprite(e->sprite, e->X, e->Y);
  g_enemyindex++;
 

}


void checkhit(){
  Enemy* e;
  byte i, width, height;
  boolean debug = false;
  //debug = true;
  
  e = &enemies[0];
  for( i = 0; i < g_enemycount; i++){
    #define enemy_yb 	e->Y - width
    #define enemy_yt 	e->Y
    #define enemy_xl	e->X
    #define enemy_xr	e->X + height
    width = e->sprite[0];
    height = e->sprite[1];

    
    if (Bullet.Y <= enemy_yt &&
        Bullet.Y >= enemy_yb &&
      	Bullet.X >= enemy_xl && 
        Bullet.X <= enemy_xr )
    { 
      if(debug){
      	sprintf(__hld, "A %03d",  g_enemycount - i);
    	drawstr(__hld, 0, 30);
        sprintf(__hld, "B %03d",  (enemies-e+MAX_ENEMIES-1));
    	drawstr(__hld, 0, 29);
        
        sprintf(__hld, "HIT %03d GC %03d",  i, g_enemycount );
    	drawstr(__hld, 0, 28);  
      }

      clearsprite(e->sprite, e->X, e->Y);
      clearsprite(bullet_bitmap, Bullet.X, Bullet.Y);
      memmove(e, (e+1), sizeof(Enemy) * (g_enemycount - i - 1) );
      
      if ( i < g_enemyindex) g_enemyindex--;
      
      g_enemycount--; 
      Bullet.Y = 0;
      g_score++;
      drawscore();
    }
    
   
    *e++;
  }
}

void moveplayer(){
  if( LEFT1 && Player.X > MIN_X ){
    Player.X += -PLAYER_STEP;
  } else if ( RIGHT1 && Player.X < MAX_X ){
    Player.X += PLAYER_STEP;
  } 

  if ( FIRE1 && Bullet.Y == 0 ){
    firebullet();
  }
  if( Bullet.Y ){
    movebullet();
  }
  drawsprite(player_bitmap, Player.X, Player.Y);

}


void setconfig(){
  Bullet.Y = 0;
  Player.Y = 1;
  Player.X = 100;
  g_enemycount = MAX_ENEMIES ;
  g_enemyindex = 0;  
  g_currentMode.right = 0; 
  g_score = 0;
  g_frame = 0;
  g_lives = 3;
  
  Bomb.launched = false;
}

void setenemies(){
  byte i;
  byte x = 20;
  byte y = 25;
  byte bm = 0;
  byte spacing = 10;

  for( i = 0; i < MAX_ENEMIES; i++){
    Enemy* e = &enemies[i];
    e->X = x;
    e->Y = y;
    e->sprite = enemies_bitmap[bm];

    x+= e->sprite[1] + spacing;

    if ( x > 180 ){
      x = 20;
      y -= e->sprite[0] + 1;
    } 

    bm = ++bm % 4; //4 == number of sprites
  }
  

}


void gameover(){
  byte i;
  for (i=0; i<50; i++) {
    drawstr(" *************** ", 5, 15);
    drawstr("***           ***", 5, 16);
    drawstr("**  GAME OVER  **", 5, 17);
    drawstr("***           ***", 5, 18);
    drawstr(" *************** ", 5, 19);
    watchdog_strobe = 0;
  }

}

void rungame(){
  
  clrscr();
  setconfig();
  setenemies();
  drawscore();
  drawground();
  drawshields();
  drawlives();
  
  drawbunker(30, 50, 15, 15, 20);
  drawbunker(140, 50, 15, 15, 20);
  

  for( ; g_enemycount && g_lives; ){
    moveplayer();
    updateenemies();

    if( g_frame & 1 ){

      if (!Bomb.launched){
        dropbomb();
      } else {
        movebomb();
      }   
    }

    g_frame++;
    watchdog_strobe = 0;
  }  
  
  if ( !g_lives) gameover();
  
  
}



void main(){

  /*It's so creative the way it's implemented the cleanse of the screen.
    All sprites have kind of an empty border around them, so as they move
    these empty borders just erase all the leftovers the sprites would have
    left behind. Simple but effective. */  
  for(; ; ){
    rungame();
  }
}









