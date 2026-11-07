/* DevPulse OS V-1 - a tiny 32-bit operating system that runs in 128 MB of RAM. */

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

#define OS_NAME    "DevPulse OS"
#define OS_VERSION "V-1"
#define RAM_TARGET_KB (128u * 1024u)

extern char __bss_start[], __bss_end[], __kernel_end[];

/* ---------- tiny C library (the compiler needs these) ---------- */
void *memset(void *d, int v, u32 n){ u8 *p = d; while (n--) *p++ = (u8)v; return d; }
void *memcpy(void *d, const void *s, u32 n){ u8 *a = d; const u8 *b = s; while (n--) *a++ = *b++; return d; }
static u32 slen(const char *s){ u32 n = 0; while (s[n]) n++; return n; }
static int seq(const char *a, const char *b){ while (*a && *a == *b){ a++; b++; } return *a == *b; }

/* ---------- ports ---------- */
static inline void outb(u16 p, u8 v){ __asm__ volatile("outb %0,%1" :: "a"(v), "Nd"(p)); }
static inline u8   inb(u16 p){ u8 v; __asm__ volatile("inb %1,%0" : "=a"(v) : "Nd"(p)); return v; }

/* ---------- serial port (COM1): lets you use the OS from a terminal too ---------- */
static void serial_init(void){
    outb(0x3F9, 0); outb(0x3FB, 0x80); outb(0x3F8, 3); outb(0x3F9, 0);
    outb(0x3FB, 3); outb(0x3FA, 0xC7); outb(0x3FC, 0x0B);
}
static void serial_put(char c){
    int t = 100000;
    while (!(inb(0x3FD) & 0x20) && --t) ;
    outb(0x3F8, (u8)c);
}

/* ---------- screen (VGA text mode 80x25) ---------- */
#define VGA ((volatile u16 *)0xB8000)
static int cx, cy;
static u8 attr = 0x1F;           /* white on blue */
#define ACCENT 0x1E              /* yellow on blue (our "orange") */

static void cursor(void){
    u16 pos = (u16)(cy * 80 + cx);
    outb(0x3D4, 14); outb(0x3D5, (u8)(pos >> 8));
    outb(0x3D4, 15); outb(0x3D5, (u8)(pos & 255));
}
static void scroll(void){
    for (int i = 0; i < 80 * 24; i++) VGA[i] = VGA[i + 80];
    for (int i = 80 * 24; i < 80 * 25; i++) VGA[i] = (u16)((attr << 8) | ' ');
    cy = 24;
}
static void putc(char c){
    if (c == '\n'){ serial_put('\r'); serial_put('\n'); cx = 0; cy++; }
    else if (c == '\b'){
        serial_put('\b'); serial_put(' '); serial_put('\b');
        if (cx > 0){ cx--; VGA[cy * 80 + cx] = (u16)((attr << 8) | ' '); }
    } else {
        serial_put(c);
        VGA[cy * 80 + cx] = (u16)((attr << 8) | (u8)c);
        if (++cx >= 80){ cx = 0; cy++; }
    }
    if (cy >= 25) scroll();
    cursor();
}
static void puts(const char *s){ while (*s) putc(*s++); }
static void putsa(const char *s, u8 a){ u8 o = attr; attr = a; puts(s); attr = o; }
static void clear(void){
    for (int i = 0; i < 80 * 25; i++) VGA[i] = (u16)((attr << 8) | ' ');
    cx = cy = 0; cursor();
}
static void pdec(u32 v){
    char b[12]; int i = 0;
    if (!v){ putc('0'); return; }
    while (v){ b[i++] = (char)('0' + v % 10); v /= 10; }
    while (i) putc(b[--i]);
}
static void pdec_s(int v){ if (v < 0){ putc('-'); pdec((u32)(-v)); } else pdec((u32)v); }
static void p2(u32 v){ putc((char)('0' + v / 10)); putc((char)('0' + v % 10)); }

/* ---------- keyboard (PS/2) + serial input ---------- */
static const char km[] = "\0\033" "1234567890-=" "\b\t" "qwertyuiop[]" "\n\0" "asdfghjkl;'`" "\0\\" "zxcvbnm,./" "\0*\0 ";
static const char ks[] = "\0\033" "!@#$%^&*()_+" "\b\t" "QWERTYUIOP{}" "\n\0" "ASDFGHJKL:\"~" "\0|" "ZXCVBNM<>?" "\0*\0 ";
static int shift;

static char getkey(void){
    for (;;){
        if (inb(0x64) & 1){
            u8 sc = inb(0x60);
            if (sc == 0x2A || sc == 0x36){ shift = 1; continue; }
            if (sc == 0xAA || sc == 0xB6){ shift = 0; continue; }
            if (sc & 0x80) continue;
            if (sc < sizeof(km) - 1){
                char c = shift ? ks[sc] : km[sc];
                if (c) return c;
            }
        }
        if (inb(0x3FD) & 1){
            char c = (char)inb(0x3F8);
            if (c == '\r') c = '\n';
            if (c == 127) c = '\b';
            return c;
        }
        __asm__ volatile("pause");
    }
}
static void readline(char *buf, int max){
    int n = 0;
    for (;;){
        char c = getkey();
        if (c == '\n'){ putc('\n'); buf[n] = 0; return; }
        if (c == '\b'){ if (n > 0){ n--; putc('\b'); } continue; }
        if (c >= 32 && c < 127 && n < max - 1){ buf[n++] = c; putc(c); }
    }
}

/* ---------- clock (CMOS real-time clock) ---------- */
static u8 cmos(u8 r){
    for (;;){ outb(0x70, 0x0A); if (!(inb(0x71) & 0x80)) break; }
    outb(0x70, r); return inb(0x71);
}
static u8 bcd(u8 v){
    outb(0x70, 0x0B);
    if (inb(0x71) & 4) return v;
    return (u8)((v & 15) + (v >> 4) * 10);
}
static u32 day_secs(void){
    return bcd(cmos(4)) * 3600u + bcd(cmos(2)) * 60u + bcd(cmos(0));
}
static u32 boot_secs;

/* ---------- in-memory files ---------- */
#define NFILES 16
struct file { char name[16]; char data[240]; u8 used; };
static struct file files[NFILES];
static struct file *find(const char *n){
    for (int i = 0; i < NFILES; i++) if (files[i].used && seq(files[i].name, n)) return &files[i];
    return 0;
}

/* ---------- commands ---------- */
static u32 total_kb(void){ return *(volatile u32 *)0x500; }

static void banner(void){
    putsa("  ____             ____        _          ___  ____\n", ACCENT);
    putsa(" |  _ \\  _____   _|  _ \\ _   _| |___  ___/ _ \\/ ___|\n", ACCENT);
    putsa(" | | | |/ _ \\ \\ / / |_) | | | | / __|/ _ \\ | | \\___ \\\n", ACCENT);
    putsa(" | |_| |  __/\\ V /|  __/| |_| | \\__ \\  __/ |_| |___) |\n", ACCENT);
    putsa(" |____/ \\___| \\_/ |_|    \\__,_|_|___/\\___|\\___/|____/\n", ACCENT);
    puts("\n  "); puts(OS_NAME); puts(" "); puts(OS_VERSION); puts("   Build AI systems. Ship faster.\n");
}
static void cmd_mem(void){
    u32 kb = total_kb();
    puts("Total RAM    : "); pdec((kb + 512) / 1024); puts(" MB ("); pdec(kb); puts(" KB)\n");
    puts("Kernel image : "); pdec(((u32)__kernel_end - 0x10000u + 1023) / 1024); puts(" KB at 0x10000\n");
    puts("128 MB target: "); puts(kb + 2048 >= RAM_TARGET_KB ? "OK\n" : "NOT MET (give the VM 128 MB or more)\n");
}
static void cmd_cpu(void){
    char v[13]; u32 b, c, d;
    __asm__ volatile("cpuid" : "=b"(b), "=c"(c), "=d"(d) : "a"(0));
    *(u32 *)v = b; *(u32 *)(v + 4) = d; *(u32 *)(v + 8) = c; v[12] = 0;
    puts("CPU vendor   : "); puts(v); puts("\n");
}
static void cmd_time(void){
    u32 s = day_secs();
    p2(s / 3600); putc(':'); p2((s / 60) % 60); putc(':'); p2(s % 60); putc('\n');
}
static void cmd_uptime(void){
    u32 u = (day_secs() + 86400u - boot_secs) % 86400u;
    pdec(u / 3600); puts("h "); pdec((u / 60) % 60); puts("m "); pdec(u % 60); puts("s\n");
}
static int atoi_(const char *s){
    int neg = 0, v = 0;
    if (*s == '-'){ neg = 1; s++; }
    while (*s >= '0' && *s <= '9') v = v * 10 + (*s++ - '0');
    return neg ? -v : v;
}
static char *skip(char *s){ while (*s == ' ') s++; return s; }
static char *word(char *s){            /* ends the first word, returns the rest */
    while (*s && *s != ' ') s++;
    if (*s){ *s++ = 0; s = skip(s); }
    return s;
}
static void cmd_calc(char *a){
    char *op = word(a), *b = word(op); word(b);
    if (!*a || !*op || !*b){ puts("Usage: calc 12 * 3   (+ - * / %)\n"); return; }
    int x = atoi_(a), y = atoi_(b), r = 0;
    switch (op[0]){
        case '+': r = x + y; break;
        case '-': r = x - y; break;
        case '*': r = x * y; break;
        case '/': if (!y){ puts("Cannot divide by zero\n"); return; } r = x / y; break;
        case '%': if (!y){ puts("Cannot divide by zero\n"); return; } r = x % y; break;
        default: puts("Unknown operator. Use + - * / %\n"); return;
    }
    pdec_s(r); putc('\n');
}
static void cmd_color(char *a){
    int n = atoi_(a);
    if (!*a || n < 0 || n > 15){ puts("Usage: color 0-15  (e.g. color 14 for yellow)\n"); return; }
    attr = (u8)((attr & 0xF0) | n);
    for (int i = 0; i < 80 * 25; i++) VGA[i] = (u16)((VGA[i] & 0x00FF) | (attr << 8));
}
static void cmd_ls(void){
    int n = 0;
    for (int i = 0; i < NFILES; i++) if (files[i].used){
        puts("  "); puts(files[i].name); puts("  ("); pdec(slen(files[i].data)); puts(" bytes)\n"); n++;
    }
    if (!n) puts("No files yet. Try: write notes hello world\n");
}
static void cmd_write(char *a){
    char *text = word(a);
    if (!*a){ puts("Usage: write <name> <text>\n"); return; }
    if (slen(a) > 15){ puts("Name is too long (max 15 characters)\n"); return; }
    struct file *f = find(a);
    if (!f){
        for (int i = 0; i < NFILES; i++) if (!files[i].used){ f = &files[i]; break; }
        if (!f){ puts("Disk full (16 files max). Use rm to free space.\n"); return; }
        memset(f, 0, sizeof(*f)); f->used = 1;
        memcpy(f->name, a, slen(a) + 1);
    }
    u32 n = slen(text); if (n > 239) n = 239;
    memcpy(f->data, text, n); f->data[n] = 0;
    puts("Saved "); puts(f->name); puts("\n");
}
static void cmd_cat(char *a){
    word(a);
    struct file *f = find(a);
    if (!*a){ puts("Usage: cat <name>\n"); return; }
    if (!f){ puts("No such file\n"); return; }
    puts(f->data); putc('\n');
}
static void cmd_rm(char *a){
    word(a);
    struct file *f = find(a);
    if (!*a){ puts("Usage: rm <name>\n"); return; }
    if (!f){ puts("No such file\n"); return; }
    f->used = 0; puts("Removed\n");
}
static void cmd_help(void){
    puts("Commands:\n");
    puts("  help            show this list\n");
    puts("  about           about this OS\n");
    puts("  mem             RAM and kernel size\n");
    puts("  cpu             CPU vendor\n");
    puts("  time, uptime    clock and time since boot\n");
    puts("  clear           clear the screen\n");
    puts("  echo <text>     print text\n");
    puts("  calc 12 * 3     calculator\n");
    puts("  color 0-15      text color\n");
    puts("  ls              list files (stored in RAM)\n");
    puts("  write <n> <t>   save text to file n\n");
    puts("  cat <n>         show file n\n");
    puts("  rm <n>          delete file n\n");
    puts("  reboot, halt    restart or stop\n");
}
static void cmd_about(void){
    puts(OS_NAME " " OS_VERSION "\n");
    puts("A small 32-bit OS written from scratch in C and assembly.\n");
    puts("Runs on x86 PCs (BIOS boot) with 128 MB of RAM.\n");
    puts("Not Windows- or Linux-compatible: it runs only its own commands.\n");
    puts("By Abhimanu Kumar - github.com/AbhimanuKD\n");
}
static void reboot(void){
    int t = 100000; while ((inb(0x64) & 2) && --t) ;
    outb(0x64, 0xFE);
    for (;;) __asm__ volatile("hlt");
}

void kmain(void){
    memset(__bss_start, 0, (u32)(__bss_end - __bss_start));
    serial_init();
    clear();
    banner();
    putc('\n');
    puts("  RAM: "); pdec((total_kb() + 512) / 1024); puts(" MB   128 MB target: ");
    puts(total_kb() + 2048 >= RAM_TARGET_KB ? "OK" : "NOT MET");
    puts("\n  Type 'help' to see commands.\n\n");
    boot_secs = day_secs();

    char line[80];
    for (;;){
        putsa("devpulse", ACCENT); putsa("> ", ACCENT);
        readline(line, sizeof(line));
        char *cmd = skip(line);
        if (!*cmd) continue;
        char *arg = word(cmd);
        if      (seq(cmd, "help"))   cmd_help();
        else if (seq(cmd, "about") || seq(cmd, "ver")) cmd_about();
        else if (seq(cmd, "mem"))    cmd_mem();
        else if (seq(cmd, "cpu"))    cmd_cpu();
        else if (seq(cmd, "time"))   cmd_time();
        else if (seq(cmd, "uptime")) cmd_uptime();
        else if (seq(cmd, "clear"))  clear();
        else if (seq(cmd, "echo"))   { puts(arg); putc('\n'); }
        else if (seq(cmd, "calc"))   cmd_calc(arg);
        else if (seq(cmd, "color"))  cmd_color(arg);
        else if (seq(cmd, "ls"))     cmd_ls();
        else if (seq(cmd, "write"))  cmd_write(arg);
        else if (seq(cmd, "cat"))    cmd_cat(arg);
        else if (seq(cmd, "rm"))     cmd_rm(arg);
        else if (seq(cmd, "reboot")) reboot();
        else if (seq(cmd, "halt"))   { puts("System halted. You can power off.\n"); for (;;) __asm__ volatile("cli; hlt"); }
        else { puts("Unknown command: "); puts(cmd); puts("  (type 'help')\n"); }
    }
}
