# Projekt: redukcja punktów, otoczki wypukłe i fuzja czujników

Projekt zawiera dwa niezależne programy budowane z jednego `CMakeLists.txt`:

| Program (target CMake) | Źródło | Zadanie |
|---|---|---|
| `ex1_deduplication` | `src/main.cpp` | Zad. 1: redukcja punktów (Greedy, Spatial Hash) + otoczka wypukła (Convex Hull, Graham Scan) |
| `zad2_sensors` | `src/main_zad2.cpp` | Zad. 2: fuzja danych z dwóch czujników pozycji |

## Wymagania

- kompilator C++ ze wsparciem **C++20** (GCC 10+, Clang 11+, MSVC 2019+)
- **CMake 3.10** lub nowszy

## Struktura katalogów

```
.
├── CMakeLists.txt
├── data/DaneDoZadan/
│   ├── zad1/example_input.txt
│   └── zad2/sensor1.csv, sensor2.csv
├── include/        # nagłówki (.h)
└── src/            # pliki źródłowe (.cpp)
```

## Budowanie

W katalogu głównym projektu:

```bash
cmake -S . -B build
cmake --build build
```

Zbudowanie tylko jednego programu:

```bash
cmake --build build --target ex1_deduplication
cmake --build build --target zad2_sensors
```

> Uwaga (Windows / Visual Studio): pliki wykonywalne mogą znajdować się w `build/Debug/` lub `build/Release/`. Można też zbudować z flagą `--config Release`.

Alternatywnie projekt można otworzyć w CLion (domyślny katalog budowania to `cmake-build-debug`).

---

## Zadanie 1: `ex1_deduplication`

Program nie przyjmuje argumentów. Dane wejściowe są wczytywane ze stałej ścieżki:

```
data/DaneDoZadan/zad1/example_input.txt
```

Ścieżka jest **względna**, dlatego program trzeba uruchamiać **z katalogu głównego projektu**.

### Uruchomienie

Linux / macOS:

```bash
./build/ex1_deduplication
```

Windows (PowerShell / cmd):

```powershell
.\build\Debug\ex1_deduplication.exe
```

### Co robi program

1. Wczytuje punkty z `example_input.txt`.
2. Redukuje je dwiema metodami (próg `eps = 0.5`): **Greedy** i **Spatial Hash**.
3. Dla każdego wyniku liczy otoczkę wypukłą dwoma algorytmami: **Convex Hull** i **Graham Scan**.
4. Zapisuje 4 pliki wynikowe **w bieżącym katalogu** (tam, skąd uruchomiono program):

| Plik wyjściowy | Redukcja | Otoczka |
|---|---|---|
| `output_greedy_convexhull.txt` | Greedy | Convex Hull |
| `output_spatial_convexhull.txt` | Spatial Hash | Convex Hull |
| `output_greedy_graham.txt` | Greedy | Graham Scan |
| `output_spatial_graham.txt` | Spatial Hash | Graham Scan |

### Format wyniku

```
CONE <id> <x> <y> [<id_oryginalnych_punktów>]
...
ENCLOSURE <obwód_otoczki> [<id_punktów_otoczki>]
```

Po poprawnym wykonaniu program wypisuje komunikat o sukcesie. W razie błędu wczytania pliku zwraca kod `1`.

### Uruchomienie w CLion

Jeśli program nie znajduje pliku wejściowego, ustaw w *Run → Edit Configurations → Working directory* katalog główny projektu (`$ProjectFileDir$`).

---

## Zadanie 2: `zad2_sensors`

Program łączy dane z dwóch czujników: gęstego (z dryfem) i rzadkiego (z pojedynczymi outlierami). Wynikiem jest skorygowana trajektoria.

### Uruchomienie

Składnia:

```
zad2_sensors [sensor1.csv sensor2.csv [prefiks_wyjscia]]
```

**Wariant 1: z jawnymi ścieżkami (zalecany, niezależny od katalogu roboczego):**

```bash
./build/zad2_sensors data/DaneDoZadan/zad2/sensor1.csv data/DaneDoZadan/zad2/sensor2.csv
```

**Wariant 2: z własnym prefiksem plików wyjściowych:**

```bash
./build/zad2_sensors data/DaneDoZadan/zad2/sensor1.csv data/DaneDoZadan/zad2/sensor2.csv wynik
```

**Wariant 3: bez argumentów.** Program szuka plików w kolejności:

1. `../data/DaneDoZadan/zad2/sensor1.csv` i `sensor2.csv` (ścieżka względem podkatalogu, np. `build/` lub `cmake-build-debug/`, więc uruchamiaj z tego katalogu),
2. jeśli nie znajdzie: `sensor1.csv` i `sensor2.csv` w bieżącym katalogu.

```bash
cd build
./zad2_sensors
```

### Format danych wejściowych

Plik CSV z nagłówkiem (pierwsza linia jest pomijana) i wierszami:

```
t,x,y
```

### Pliki wyjściowe

Zapisywane w bieżącym katalogu, z prefiksem (domyślnie `output`):

| Plik | Zawartość |
|---|---|
| `<prefiks>_fast.csv` | znormalizowany czujnik gęsty (`t,x,y`) |
| `<prefiks>_slow.csv` | znormalizowany czujnik rzadki (`t,x,y,accepted`; `accepted=0` oznacza odrzucony outlier) |
| `<prefiks>_fused.csv` | wynik fuzji próbkowany co 0.01 s dla t ∈ [0, 33] |

### Tryb interaktywny

Po wygenerowaniu plików program pyta o czas:

```
<< Podaj T
>> 12.5
<< Pozycja w t=12.5s -> X: 3.142 m, Y: -8.765 m
```

- dozwolony zakres: **0 – 33 s** (przecinek dziesiętny jest akceptowany),
- niepoprawna liczba lub wartość spoza zakresu wypisuje komunikat i pyta ponownie,
- zakończenie: **Ctrl+D** (Linux/macOS) lub **Ctrl+Z, Enter** (Windows), albo zamknięcie terminala.

Przykład z danymi przekazanymi z pliku (bez trybu interaktywnego):

```bash
echo 10.5 | ./build/zad2_sensors data/DaneDoZadan/zad2/sensor1.csv data/DaneDoZadan/zad2/sensor2.csv
```

---

## Najczęstsze problemy

| Problem | Rozwiązanie |
|---|---|
| `Błąd wczytywania pliku lub pusta lista!` (zad. 1) | Uruchom program z katalogu głównego projektu lub ustaw *Working directory* w IDE. |
| `Nie mozna otworzyc pliku: ...` (zad. 2) | Podaj ścieżki do `sensor1.csv` i `sensor2.csv` jako argumenty. |
| Pliki `output_*` pojawiły się w nieoczekiwanym miejscu | Powstają w bieżącym katalogu roboczym procesu. |
