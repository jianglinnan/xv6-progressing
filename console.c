// Console input and output.
// Input is from the keyboard or serial port.
// Output is written to the screen and serial port.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "traps.h"
#include "spinlock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"

// 引入自定义的全局变量
#include "defs_struct.h"
#include "global_var.h"

#define SHELL  0
#define EDITOR 1
#define CAT    2

static void consputc(int);

static int panicked = 0;

static struct {
  struct spinlock lock;
  int locking;
} cons;

static void
printint(int xx, int base, int sign)
{
  static char digits[] = "0123456789abcdef";
  char buf[16];
  int i;
  uint x;

  if(sign && (sign = xx < 0))
    x = -xx;
  else
    x = xx;

  i = 0;
  do{
    buf[i++] = digits[x % base];
  }while((x /= base) != 0);

  if(sign)
    buf[i++] = '-';

  while(--i >= 0)
    consputc(buf[i]);
}
//PAGEBREAK: 50

// Print to the console. only understands %d, %x, %p, %s.
void
cprintf(char *fmt, ...)
{
  int i, c, locking;
  uint *argp;
  char *s;

  locking = cons.locking;
  if(locking)
    acquire(&cons.lock);

  if (fmt == 0)
    panic("null fmt");

  argp = (uint*)(void*)(&fmt + 1);
  for(i = 0; (c = fmt[i] & 0xff) != 0; i++){
    if(c != '%'){
      consputc(c);
      continue;
    }
    c = fmt[++i] & 0xff;
    if(c == 0)
      break;
    switch(c){
    case 'd':
      printint(*argp++, 10, 1);
      break;
    case 'x':
    case 'p':
      printint(*argp++, 16, 0);
      break;
    case 's':
      if((s = (char*)*argp++) == 0)
        s = "(null)";
      for(; *s; s++)
        consputc(*s);
      break;
    case '%':
      consputc('%');
      break;
    default:
      // Print unknown % sequence to draw attention.
      consputc('%');
      consputc(c);
      break;
    }
  }

  if(locking)
    release(&cons.lock);
}

void
panic(char *s)
{
  int i;
  uint pcs[10];
  
  cli();
  cons.locking = 0;
  cprintf("cpu%d: panic: ", cpu->id);
  cprintf(s);
  cprintf("\n");
  getcallerpcs(&s, pcs);
  for(i=0; i<10; i++)
    cprintf(" %p", pcs[i]);
  panicked = 1; // freeze other CPU
  for(;;)
    ;
}

//PAGEBREAK: 50
#define BACKSPACE 0x100
#define CRTPORT 0x3d4
static ushort *crt = (ushort*)P2V(0xb8000);  // CGA memory

//used to put in console
//io interface
static void
cgaputc(int c)
{
  int pos;
  
  // Cursor position: col + 80*row.
  outb(CRTPORT, 14);
  pos = inb(CRTPORT+1) << 8;
  outb(CRTPORT, 15);
  pos |= inb(CRTPORT+1);
  //enter, change line
  if(c == '\n')
    pos += 80 - pos%80;
  //backspace, clear a char and change positon
  else if(c == BACKSPACE){
    if(pos > 0) --pos;
  } else
    //pos++, change position
    crt[pos++] = (c&0xff) | 0x0700;  // black on white
  
  if((pos/80) >= 24){  // Scroll up.
    memmove(crt, crt+80, sizeof(crt[0])*23*80);
    pos -= 80;
    memset(crt+pos, 0, sizeof(crt[0])*(24*80 - pos));
  }
  
  outb(CRTPORT, 14);
  outb(CRTPORT+1, pos>>8);
  outb(CRTPORT, 15);
  outb(CRTPORT+1, pos);
  crt[pos] = ' ' | 0x0700;
}

void
consputc(int c)
{
  if(panicked){
    cli();
    for(;;)
      ;
  }

  if(c == BACKSPACE){
    uartputc('\b'); uartputc(' '); uartputc('\b');
  } else
    uartputc(c);
  cgaputc(c);
}

#define INPUT_BUF 128
struct{
  struct spinlock lock;
  char buf[INPUT_BUF];
  uint r;  // Read index
  uint w;  // Write index
  uint e;  // Edit index
} input;
int tab_loc = -1;
int constant_tab = -1;
int last_tab_loc = -1;

int last_blank_loc = 0;
int blanks[32];
int blank_len = 0;
int firstInput = 1;

void initBlank(){
  if(firstInput == 1){
    firstInput = 0;
    blank_len = 1;
    memset(&blanks,0,32);
    blanks[0] = input.w;
  }
}


void checkPrefix(){
  int i = 0;
  char buffer[INPUT_BUF];
  memset(&buffer,0,INPUT_BUF);
  for(i = tab_loc; i < input.e; i++){
    buffer[i - tab_loc] = input.buf[i % INPUT_BUF];
  }
  buffer[input.e - tab_loc] = '\0';

  //if we have found the unique prefix
  int flag = 0;
  constant_tab++;
  int bufLen = strlen(buffer); 
  for(i = constant_tab; i < ec.len; i++){
    if(strncmp(buffer,ec.commands[i],bufLen) == 0){
      flag = 1;
      constant_tab = i;
      break;
    }
  }
  if(flag == 0){
    if(constant_tab == 0)
        return;  
    else{
        constant_tab = -1;
        checkPrefix();
        return;
    }
  }
  int cmdLen = strlen(ec.commands[constant_tab]);
  for(i = bufLen; i < cmdLen; i++){
    input.buf[input.e++ % INPUT_BUF] = ec.commands[constant_tab][i];
    consputc(ec.commands[constant_tab][i]);
  }
}

#define C(x)  ((x)-'@')  // Control-x

void
consoleintr(int (*getc)(void))
{
  int c;
  int i = 0;
  acquire(&input.lock);
  while((c = getc()) >= 0){
    int length = 0;
    switch(c){
    case C('P'):  // Process listing.
      procdump();
      break;
    case C('U'):  // Kill line.
      while(input.e != input.w &&
            input.buf[(input.e-1) % INPUT_BUF] != '\n'){
        input.e--;
        consputc(BACKSPACE);
      }
      break;
    case C('H'): case '\x7f':  // Backspace
      constant_tab = -1;
      if(input.e != input.w){
        if(input.e == blanks[blank_len-1]){
          blanks[blank_len - 1] = 0;
          blank_len--;
          //cprintf("***%d***",tab_loc);
          tab_loc = blanks[blank_len-1];
          //cprintf("***%d***",tab_loc);
        }
        input.e--;
        consputc(BACKSPACE);
      }
      break;
    //ESC
    case 27:
      if(program_status == SHELL || program_status == CAT){
        firstInput = 1;
        input.w = input.e;
        input.buf[input.e++ % INPUT_BUF] = 'E';
        input.buf[input.e++ % INPUT_BUF] = 'S';
        input.buf[input.e++ % INPUT_BUF] = 'C';
        input.buf[input.e++ % INPUT_BUF] = '\n';   
        input.w = input.e;
        tab_loc = input.e;
        wakeup(&input.r);        
      }
      else if(program_status == EDITOR){
        firstInput = 1;
        input.w = input.e;
        input.buf[input.e++ % INPUT_BUF] = 'e';
        input.buf[input.e++ % INPUT_BUF] = 'x';
        input.buf[input.e++ % INPUT_BUF] = 'i';
        input.buf[input.e++ % INPUT_BUF] = 't';
        input.buf[input.e++ % INPUT_BUF] = '\n';   
        input.w = input.e;
        tab_loc = input.e;
        wakeup(&input.r); 
      }
      break;
    case 19:
      if(program_status == EDITOR){
          firstInput = 1;
        input.w = input.e;
        input.buf[input.e++ % INPUT_BUF] = 's';
        input.buf[input.e++ % INPUT_BUF] = 'a';
        input.buf[input.e++ % INPUT_BUF] = 'v';
        input.buf[input.e++ % INPUT_BUF] = 'e';
        input.buf[input.e++ % INPUT_BUF] = '\n';   
        input.w = input.e;
        tab_loc = input.e;
        wakeup(&input.r);
      }
      break;
    case '\t':
      if(program_status == SHELL){
        initBlank();
        if(tab_loc == -1){
          tab_loc = input.w; 
          last_tab_loc = 0;      
        }
        if(constant_tab != -1){
          while(input.e != last_tab_loc){
            input.e--;
            consputc(BACKSPACE);
          }
        }
        else
          last_tab_loc = input.e;
        checkPrefix();
      }
      else if(program_status == CAT || program_status == EDITOR){
        for(i = 0; i < 4; i++) {
          input.buf[input.e++ % INPUT_BUF] = ' ';
          consputc(' ');
        }
      }
      break;
    case 0xE2:
      if(program_status == SHELL){
        constant_tab = -1;
        while(input.e != input.w){
          input.e--;
          consputc(BACKSPACE);
        }
        if(first == 1){
          length = strlen(hs.history[hs.current]);
          for(i = 0; i < length; i++){
            input.buf[input.e++ % INPUT_BUF] = hs.history[hs.current][i];
            consputc(hs.history[hs.current][i]);
          }
          first = 0;
          break;
        }
        if(hs.len >= H_ITEMS){
          int end = (hs.start + 1) % H_ITEMS;
          hs.current = (hs.current != end)?(hs.current - 1 + H_ITEMS) % H_ITEMS:hs.current; 
        }
        else
          hs.current = (hs.current > 0)?(hs.current - 1):hs.current;
        length = strlen(hs.history[hs.current]);
        for(i = 0; i < length; i++){
          input.buf[input.e++ % INPUT_BUF] = hs.history[hs.current][i];
          consputc(hs.history[hs.current][i]);
        }
      }
      break;
    case 0xE3:
      if(program_status == SHELL){
        constant_tab = -1;
        while(input.e != input.w){
          input.e--;
          consputc(BACKSPACE);
        }
        if(hs.current == hs.start){
          first = 1;
          break;
        }
        if(hs.len >= H_ITEMS)
          hs.current = (hs.current != hs.start)?(hs.current + 1) % H_ITEMS:hs.current;
        else
          hs.current = (hs.current < hs.start)?(hs.current + 1):hs.current;
        length = strlen(hs.history[hs.current]);
        for(i = 0; i < length; i++){
          input.buf[input.e++ % INPUT_BUF] = hs.history[hs.current][i];
          consputc(hs.history[hs.current][i]);
        }
      }
      break;
    case 0xE4:
      break;
    case 0xE5:
      break;
    default:
      if(c != 0 && input.e-input.r < INPUT_BUF){
        initBlank();
        constant_tab = -1;
        c = (c == '\r') ? '\n' : c;
        //get input
        input.buf[input.e++ % INPUT_BUF] = c;
        consputc(c);
        if(c == ' '){
          blank_len++;
          blanks[blank_len-1] = input.e;
          tab_loc = input.e;
          //cprintf("***%d**",blank_len);
        }
        if(c == '\n' || c == C('D') || input.e == input.r+INPUT_BUF){
          firstInput = 1;
          input.w = input.e;
          tab_loc = input.e;
          wakeup(&input.r);
        }
      }
      break;
    }
  }
  release(&input.lock);
}


int
consoleread(struct inode *ip, char *dst, int n)
{
  uint target;
  int c;

  iunlock(ip);
  target = n;
  acquire(&input.lock);
  while(n > 0){
    while(input.r == input.w){
      if(proc->killed){
        release(&input.lock);
        ilock(ip);
        return -1;
      }
      sleep(&input.r, &input.lock);
    }
    c = input.buf[input.r++ % INPUT_BUF];
    if(c == C('D')){  // EOF
      if(n < target){
        // Save ^D for next time, to make sure
        // caller gets a 0-byte result.
        input.r--;
      }
      break;
    }
    *dst++ = c;
    --n;
    if(c == '\n')
      break;
  }
  release(&input.lock);
  ilock(ip);

  return target - n;
}

int
consolewrite(struct inode *ip, char *buf, int n)
{
  int i;

  iunlock(ip);
  acquire(&cons.lock);
  for(i = 0; i < n; i++)
    consputc(buf[i] & 0xff);
  release(&cons.lock);
  ilock(ip);

  return n;
}

void
consoleinit(void)
{
  initlock(&cons.lock, "console");
  initlock(&input.lock, "input");

  devsw[CONSOLE].write = consolewrite;
  devsw[CONSOLE].read = consoleread;
  cons.locking = 1;

  picenable(IRQ_KBD);
  ioapicenable(IRQ_KBD, 0);
}
