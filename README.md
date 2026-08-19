# Stack Buffer Overflow — Shellcode Injection 

![C++](https://img.shields.io/badge/C%2B%2B-00599C?style=flat-square&logo=cplusplus&logoColor=white)
![Windows](https://img.shields.io/badge/Windows_x86-0078D6?style=flat-square&logo=windows&logoColor=white)
![Immunity Debugger](https://img.shields.io/badge/Immunity_Debugger-4B275F?style=flat-square)
![Assembly](https://img.shields.io/badge/Assembly-6E4C13?style=flat-square)

> A hands-on study of a classic stack-based buffer overflow in a C++ program, exploited to redirect execution into injected shellcode that launches the Windows Calculator (`calc.exe`) as a harmless proof-of-concept.



## 📖 Overview

Buffer overflows are one of the oldest and most instructive vulnerability classes in software security. This project walks through the complete lifecycle of one:

1. **The vulnerability** — a C++ program that reads user input into a fixed-size stack buffer with no bounds checking.
2. **The analysis** — using **Immunity Debugger** to inspect the stack, find the exact offset that overwrites the saved return address, and locate a usable `JMP ESP` instruction.
3. **The exploit** — crafting a payload (padding + return-address overwrite + NOP sled + shellcode) that redirects execution into our own code.
4. **The result** — the vulnerable program launches `calc.exe` instead of returning normally.


---

## 🧩 The Vulnerability

```cpp
char buffer[64];
cout << "Enter your name: ";
gets(buffer);            // <-- no bounds checking
```

The root cause is the use of **`gets()`**, a function so dangerous it was **removed from the C11 standard**. It reads input until a newline with *no way to limit length*, so any input longer than 64 bytes writes past the end of `buffer` and corrupts adjacent stack memory — including the **saved return address** (EIP) that the CPU uses to know where to continue after `main()` finishes.

By controlling that return address, an attacker controls where the program executes next.

See [`docs/VULNERABILITY.md`](docs/VULNERABILITY.md) for the full technical breakdown.

---

## 🛠️ How the Exploit Works

The payload written by [`src/exploit.cpp`](src/exploit.cpp) is laid out like this:

```
[  76 bytes 'A' padding  ][  JMP ESP address  ][  NOP sled  ][  shellcode  ]
         fills buffer +        overwrites the      lands       spawns
         saved EBP             return address      execution   calc.exe
```

| Component | Size | Role |
|-----------|------|------|
| Padding (`AAAA...`) | 76 bytes | Fills the buffer up to the saved return address |
| `JMP ESP` address | 4 bytes | Overwrites EIP; points to an instruction that jumps to the stack |
| NOP sled (`0x90`) | 16 bytes | Gives the CPU a safe "runway" to slide into the shellcode |
| Shellcode | ~118 bytes | Resolves `WinExec` and runs `calc.exe` |

The offset (**76**) and the `JMP ESP` address were both found **manually in Immunity Debugger** — that discovery process is the real learning of the project, and this discovery process was the core learning of the project.

---

## 🧪 Reproducing It (Lab Setup)

> These steps require an **x86 Windows environment you control**, with modern protections disabled. This is intentionally *not* how real software should run — the mitigations exist precisely to stop this attack.

1. **Compile the vulnerable program** with an old/permissive toolchain (32-bit).
2. **Disable protections** for the study (this is why the exploit works at all)
   - **DEP** (Data Execution Prevention)
   - **ASLR** (Address Space Layout Randomization)
   - Stack canaries / `/GS` (compiler stack protection)
3. **Generate the payload:** compile and run [`src/exploit.cpp`](src/exploit.cpp) to produce `payload.bin`.
4. **Attach Immunity Debugger** to the vulnerable process and feed it the payload.
5. Observe execution jump into the shellcode → **Calculator opens**.


---

## 🛡️ Only Works With Protections Off

A key learning outcome: this exploit only succeeds because we **turned the defenses off**. On a normally-configured modern system, each of these would break it:

| Mitigation | What it does | Effect on this exploit |
|------------|--------------|------------------------|
| **DEP / NX** | Marks the stack non-executable | Shellcode on the stack won't run |
| **ASLR** | Randomizes memory addresses | Hardcoded `JMP ESP` address becomes unreliable |
| **Stack Canaries (`/GS`)** | Places a guard value before the return address | Overflow is detected before `main()` returns |
| **Safe functions** | `fgets()` / `gets_s()` enforce a length limit | The overflow never happens in the first place |

**The fix in the source code is one line:**

```cpp
// Vulnerable:
gets(buffer);

// Safe:
fgets(buffer, sizeof(buffer), stdin);
```

---

## 📂 Repository Structure

```
.
├── README.md                  
├── src/
│   ├── vulnerable.cpp         # The program with the buffer overflow
│   └── exploit.cpp            # Builds payload.bin (padding + JMP ESP + NOP + shellcode)
├── docs/
│   ├── VULNERABILITY.md       # Deep-dive: why gets() breaks, stack anatomy
│   └── LEARNINGS.md           # What we learned 
└── LICENSE
```

---

## 🎓 What We Learned

It connected C++ language basics to real security consequences. Highlights:

- How the **call stack** actually works — buffers, saved EBP, and the return address.
- Why **memory-unsafe functions** like `gets()` are dangerous, and what safe alternatives exist.
- **Little-endian** byte ordering (why the `JMP ESP` address is written backwards).
- Reading **assembly and CPU registers** (ESP, EIP) in a live debugger.
- The purpose of real-world **exploit mitigations** — and a new appreciation for *why* they exist.

Full reflection in [`docs/LEARNINGS.md`](docs/LEARNINGS.md).



## 📄 License

Released under the [MIT License](LICENSE) for educational use.
