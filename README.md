# Počítačová grafika 2

## Funkce
| Požadavek | Splněno |
| :--- | :---: |
| 3D GL Core profile + shadery verze 4.6, GL debug, JSON config | ✅ |
| Vysoký výkon => alespoň 60 FPS (zobrazení FPS) | ✅ (1000+ FPS) |
| Ovládání VSync, antialiasing, přepínání fullscreen/okno | ✅ |
| Zpracování událostí: myš (obě osy, kolečko), klávesnice | ✅ |
| Více různých nezávisle se pohybujících 3D modelů (min. 2 ze souboru) | ✅ (slunce, průhledný box, zajíc) |
| Alespoň tři různé textury | ✅ (heightmap) |
| Model osvětlení (ambient, directional, min. 2x point, min. 1x reflector) | ✅ (slunce, čelovka, létající světla) |
| Korektní plná alfa průhlednost (min. 2 objekty) | ✅ |
| Správné kolize | ✅ |
|  |
| **EXTRAS** | |
| Výšková mapa texturovaná podle výšky | ✅ |
| Zvuk (lepší než jen na pozadí) | ✅ (zvuk při skákání) |

---

## 🎯 Ovládání

### 🕹️ Pohyb postavy
| Klávesa | Akce |
|:---:|---|
| **W** | Pohyb dopředu |
| **S** | Pohyb dozadu |
| **A** | Pohyb doleva |
| **D** | Pohyb doprava |
| **Mezerník** | Skok |

### 👁️ Ovládání kamery
| Ovládací prvek | Akce |
|:---:|---|
| **Pohyb myší** | Otáčení kamery (FPS pohled) |
| **Kolečko myši** | Přiblížení/oddálení (změna FOV) |

### ⚙️ Nastavení systému
| Klávesa | Funkce |
|:---:|---|
| **F10** | Přepínání režimu celá obrazovka/okno |
| **F11** | Přepnutí antialiasingu (potřeba restart aplikace) |
| **F12** | Přepnutí VSync |
| **Esc** | Ukončení aplikace |

---
### 📦 Požadované knihovny
- **GLFW** - Správa oken a vstupů
- **GLEW** - Rozšíření OpenGL
- **GLM** - Matematická knihovna pro grafiku
- **OpenCV** - Počítačové vidění (nainstalováno na `C:`)
- **nlohmann/json** - Práce s JSON daty
- **miniaudio** - Zvukový engine (součást projektu)
---
## 🚀 Instalace a spuštění

### Prerekvizity
- Visual Studio 2022
- Správce balíčků VCPKG pro závislosti
- OpenCV nainstalované na výchozí cestě `C:`

### Kroky pro spuštění
1. Naklonujte/otevřete projekt ve Visual Studio 2022
2. Stáhněte závislosti přes VCPKG
3. Sestavte řešení
4. Spusťte aplikaci
