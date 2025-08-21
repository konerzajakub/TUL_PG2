# PG2

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


## 🕹️ Ovládání

| Klávesa / Myš     | Akce                                      |
|-------------------|--------------------------------------------|
| `W A S D`         | Pohyb dopředu, dozadu, do stran            |
| Myš               | Otáčení kamery (FPS look)                  |
| `SPACE`           | Skok                 |
| `F10`              | Přepínání fullscreen/window                |
| `F11`               | Přepnutí antialiasing |
| `F12`              | Přepínání VSync                            |
| `Esc`             | Zavřít aplikaci                            |
| Kolečko myši      | Zoom (změna FOV)     |

## 🔧 Závislosti

- `GLFW`
- `GLEW`
- `GLM`
- `OpenCV` (linknuté na C:)
- `nlohmann/json`
- `miniaudio` (src/miniaudio/miniaudio.h) - stažený z internetu

## Spuštění
- Vytvářel jsem to ve Visual Studio 2022, mělo by to jít spustit v projektu
- Pozor aby byly stažené závislosti přes VCPKG a OpenCV na C: (v path)
