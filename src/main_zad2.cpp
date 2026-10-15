/**
 * Zadanie 2.2 – Skidpad: pozycja bolidu w dokładnym momencie T.
 *
 * Wnioski z analizy danych (zad. 2.1), na których opiera się algorytm:
 *
 *  1. Sensor GĘSTY (~45 Hz) jest gładki, ale DRYFUJE – jego błąd to wolnozmienny
 *     wektor (typowo całkowanie żyroskopu/IMU: mały błąd kąta kursu rośnie
 *     w czasie i "obraca" całą trajektorię).
 *  2. Sensor RZADKI (~2 Hz) nie dryfuje, ale ma pojedyncze OUTLIERY (skoki typu
 *     GPS/multipath) oraz jest odbity lustrzanie w osi Y.
 *  3. Regulamin Skidpad: najpierw okrążenia po PRAWEJ stronie, potem po lewej.
 *     Dodatnie y = lewo, więc poprawny sensor na pierwszym kole ma y < 0.
 *     Sensor, który skręca najpierw w lewo, ma odwróconą oś Y -> odwracamy znak y
 *     (decyzja z danych + regulaminu, a nie z numeru pliku).
 *
 * Algorytm (filtr komplementarny w wersji "error-state"):
 *     wynik(T) = gęsty(T) + e(T),   e(t) = rzadki(t) - gęsty(t)
 *  - gęsty sensor daje wysokie częstotliwości (kształt toru, dynamika),
 *  - rzadki sensor daje niskie częstotliwości (brak dryfu),
 *  - e(t) to estymata błędu gęstego sensora; outliery rzadkiego sensora są
 *    usuwane filtrem Hampela PRZED zbudowaniem e(t), więc nie trafiają do wyniku.
 *  Nie ma przełączania między sensorami, więc nie ma skoków w wyniku.
 *
 * Budowanie:  g++ -std=c++17 -O2 -Wall -Wextra skidpad_fusion.cpp -o skidpad
 * Uruchomienie: ./skidpad sensor1.csv sensor2.csv [prefiks_eksportu]
 *   (opcjonalny prefiks zapisuje pliki CSV do wykresów: *_fast.csv, *_slow.csv, *_fused.csv)
 */

#include <algorithm>
#include <cmath>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

/// Jeden pomiar położenia środka ciężkości.
struct Sample {
    double t;  ///< znacznik czasu [s]
    double x;  ///< położenie w przód [m]
    double y;  ///< położenie w lewo [m]
};
using Track = std::vector<Sample>;

// ---------------------------------------------------------------- parametry
constexpr double kTMin = 0.0;           ///< dolna granica zakresu T (z treści zadania)
constexpr double kTMax = 33.0;          ///< górna granica zakresu T (z treści zadania)
constexpr double kTurnWindow = 10.0;    ///< [s] okno, w którym szukamy pierwszego zakrętu
constexpr int    kHampelHalf = 2;       ///< połowa okna Hampela (okno = 5 próbek rzadkiego sensora)
constexpr double kHampelK = 5.0;        ///< próg = kHampelK * sigma (sigma z MAD)
constexpr double kMinThreshold = 1.0;   ///< [m] dolne ograniczenie progu (gdy szum jest bardzo mały)
constexpr double kExportStep = 0.01;    ///< [s] krok próbkowania eksportu wyniku

// ------------------------------------------------------------ wczytywanie CSV
/**
 * Wczytuje plik "timestamp,x,y". Pomija nagłówek, puste i uszkodzone wiersze
 * (z ostrzeżeniem), usuwa '\r' z plików windowsowych, sortuje po czasie
 * i usuwa duplikaty znaczników czasu (dzielenie przez zero w interpolacji).
 */
Track loadCsv(const std::string& path) {
    std::ifstream file(path);
    if (!file) throw std::runtime_error("Nie mozna otworzyc pliku: " + path);

    Track data;
    std::string line;
    std::getline(file, line);  // nagłówek
    size_t lineNo = 1;
    while (std::getline(file, line)) {
        ++lineNo;
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string a, b, c;
        if (!std::getline(ss, a, ',') || !std::getline(ss, b, ',') || !std::getline(ss, c, ',')) {
            std::cerr << "[ostrzezenie] " << path << ":" << lineNo << " - zla liczba kolumn, pomijam\n";
            continue;
        }
        try {
            data.push_back({std::stod(a), std::stod(b), std::stod(c)});
        } catch (const std::exception&) {
            std::cerr << "[ostrzezenie] " << path << ":" << lineNo << " - nie liczba, pomijam\n";
        }
    }

    std::stable_sort(data.begin(), data.end(),
                     [](const Sample& l, const Sample& r) { return l.t < r.t; });
    data.erase(std::unique(data.begin(), data.end(),
                           [](const Sample& l, const Sample& r) { return l.t == r.t; }),
               data.end());
    return data;
}

// -------------------------------------------------------------- interpolacja
/**
 * Interpolacja liniowa położenia w dowolnym momencie T.
 * Poza zakresem danych zwraca wartość skrajną (clamp), więc T tuż przy
 * końcach zakresu nie powoduje wyjścia poza tablicę.
 * Szukanie binarne (upper_bound) -> O(log n) na zapytanie.
 */
Sample interpolate(const Track& d, double T) {
    if (d.empty()) throw std::logic_error("interpolate: pusty tor");
    if (T <= d.front().t) return {T, d.front().x, d.front().y};
    if (T >= d.back().t) return {T, d.back().x, d.back().y};

    // pierwszy element ze znacznikiem > T; dzięki clampowi wyżej nie jest to begin() ani end()
    auto it = std::upper_bound(d.begin(), d.end(), T,
                               [](double v, const Sample& s) { return v < s.t; });
    const Sample& b = *it;
    const Sample& a = *(it - 1);
    const double dt = b.t - a.t;
    const double k = dt > 0.0 ? (T - a.t) / dt : 0.0;
    return {T, a.x + k * (b.x - a.x), a.y + k * (b.y - a.y)};
}

// ----------------------------------------------------- korekta układu odniesienia
/**
 * Zwraca znak y w punkcie największego |y| w pierwszych kTurnWindow sekundach
 * (to szczyt pierwszego koła). -1 = skręt w prawo, +1 = skręt w lewo.
 * Outlier rzędu 2-3 m nie wygra z y ~ 17 m na szczycie koła.
 */
int firstTurnSign(const Track& d) {
    const double t0 = d.front().t;
    double best = 0.0;
    for (const Sample& s : d) {
        if (s.t - t0 > kTurnWindow) break;
        if (std::abs(s.y) > std::abs(best)) best = s.y;
    }
    return best < 0.0 ? -1 : 1;
}

/**
 * Regulamin: najpierw koła prawe (y < 0). Sensor jadący najpierw w lewo
 * ma odwróconą oś Y, więc odwracamy mu znak y. Poprawny sensor zostaje bez zmian.
 */
void normalizeFrame(Track& d, const std::string& name) {
    if (firstTurnSign(d) > 0) {
        for (Sample& s : d) s.y = -s.y;
        std::cerr << "[info] " << name << ": pierwszy zakret w lewo -> odwracam znak y\n";
    } else {
        std::cerr << "[info] " << name << ": pierwszy zakret w prawo -> zgodny z regulaminem\n";
    }
}

// ------------------------------------------------------------------- statystyka
/// Mediana (kopia wektora, bo nth_element zmienia kolejność). Pusty wektor -> 0.
double median(std::vector<double> v) {
    if (v.empty()) return 0.0;
    const size_t n = v.size();
    auto mid = v.begin() + n / 2;
    std::nth_element(v.begin(), mid, v.end());
    const double hi = *mid;
    if (n % 2 == 1) return hi;
    return 0.5 * (hi + *std::max_element(v.begin(), mid));
}

// ------------------------------------------------- estymacja błędu gęstego sensora
/**
 * Buduje poprawkę e(t) = rzadki(t) - gęsty(t) tylko z przyjętych próbek.
 *
 * 1. Residuum e_i liczymy w chwilach pomiarów rzadkiego sensora (tam rzadki
 *    jest "prawdziwym" pomiarem, a gęsty interpolujemy).
 * 2. Filtr Hampela: odchylenie e_i od mediany sąsiednich residuów (okno 5).
 *    Mediana ignoruje pojedyncze skoki i nie zniekształca wolnego dryfu
 *    (dla sekwencji monotonicznej mediana to element środkowy).
 *    Próg = max(kMinThreshold, kHampelK * 1.4826 * mediana(odchyleń)).
 * 3. Wynikiem jest tor poprawek {t, ex, ey} (pole x,y przechowuje ex,ey).
 *
 * @param accepted [out] flaga 1/0 dla każdej próbki rzadkiego sensora (do wykresów)
 */
Track buildCorrection(const Track& fast, const Track& slow, std::vector<char>& accepted) {
    const size_t n = slow.size();

    Track res(n);
    for (size_t i = 0; i < n; ++i) {
        const Sample f = interpolate(fast, slow[i].t);
        res[i] = {slow[i].t, slow[i].x - f.x, slow[i].y - f.y};
    }

    std::vector<double> dev(n);
    for (size_t i = 0; i < n; ++i) {
        const size_t lo = i >= static_cast<size_t>(kHampelHalf) ? i - kHampelHalf : 0;
        const size_t hi = std::min(n - 1, i + kHampelHalf);
        std::vector<double> wx, wy;
        for (size_t j = lo; j <= hi; ++j) {
            wx.push_back(res[j].x);
            wy.push_back(res[j].y);
        }
        dev[i] = std::hypot(res[i].x - median(wx), res[i].y - median(wy));
    }

    const double sigma = 1.4826 * median(dev);
    const double threshold = std::max(kMinThreshold, kHampelK * sigma);

    accepted.assign(n, 0);
    Track corr;
    for (size_t i = 0; i < n; ++i) {
        if (dev[i] <= threshold) {
            accepted[i] = 1;
            corr.push_back(res[i]);
        }
    }
    if (corr.empty()) throw std::runtime_error("Wszystkie pomiary rzadkiego sensora odrzucone");

    std::cerr << "[info] outliery: odrzucono " << (n - corr.size()) << " z " << n
              << " probek rzadkiego sensora (prog " << std::fixed << std::setprecision(2)
              << threshold << " m)\n";
    return corr;
}

/// Wynik fuzji w chwili T: gęsty(T) + poprawka(T) (poprawka interpolowana liniowo).
Sample fusedAt(const Track& fast, const Track& corr, double T) {
    const Sample f = interpolate(fast, T);
    const Sample e = interpolate(corr, T);
    return {T, f.x + e.x, f.y + e.y};
}

// --------------------------------------------------------------------- eksport
void writeTrack(const std::string& path, const Track& tr, const std::vector<char>* accepted = nullptr) {
    std::ofstream out(path);
    if (!out) {
        std::cerr << "[ostrzezenie] nie moge zapisac: " << path << " (czy katalog istnieje?)\n";
        return;
    }
    out << "t,x,y" << (accepted ? ",accepted" : "") << "\n" << std::fixed << std::setprecision(4);
    for (size_t i = 0; i < tr.size(); ++i) {
        out << tr[i].t << "," << tr[i].x << "," << tr[i].y;
        if (accepted) out << "," << static_cast<int>((*accepted)[i]);
        out << "\n";
    }
}

// ------------------------------------------------------------------------ main
int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Uzycie: " << argv[0] << " sensor1.csv sensor2.csv [prefiks_eksportu]\n";
        return 1;
    }

    try {
        Track a = loadCsv(argv[1]);
        Track b = loadCsv(argv[2]);
        if (a.size() < 2 || b.size() < 2) throw std::runtime_error("Za malo danych w plikach");

        normalizeFrame(a, argv[1]);
        normalizeFrame(b, argv[2]);

        // Rola sensora wynika z częstotliwości próbkowania (więcej próbek = gęstszy).
        const bool aIsFast = a.size() >= b.size();
        const Track& fast = aIsFast ? a : b;
        const Track& slow = aIsFast ? b : a;
        std::cerr << "[info] sensor gesty: " << (aIsFast ? argv[1] : argv[2])
                  << ", rzadki: " << (aIsFast ? argv[2] : argv[1]) << "\n";

        std::vector<char> accepted;
        const Track corr = buildCorrection(fast, slow, accepted);

        if (argc >= 4) {
            const std::string prefix = argv[3];
            Track fused;
            for (int k = 0; k * kExportStep <= kTMax + 1e-9; ++k)
                fused.push_back(fusedAt(fast, corr, k * kExportStep));
            writeTrack(prefix + "_fast.csv", fast);
            writeTrack(prefix + "_slow.csv", slow, &accepted);
            writeTrack(prefix + "_fused.csv", fused);
        }

        // Pętla interaktywna: czytamy linię, żeby błędne dane nie zabijały programu.
        std::string line;
        while (true) {
            std::cout << "<< Podaj T\n>> " << std::flush;
            if (!std::getline(std::cin, line)) break;  // EOF
            std::replace(line.begin(), line.end(), ',', '.');  // akceptuj przecinek dziesiętny

            double T = 0.0;
            try {
                T = std::stod(line);
            } catch (const std::exception&) {
                std::cout << "<< Niepoprawna liczba!\n";
                continue;
            }
            if (!(T >= kTMin && T <= kTMax)) {  // negacja łapie też NaN
                std::cout << "<< Wartosc spoza zakresu!\n";
                continue;
            }
            const Sample s = fusedAt(fast, corr, T);
            std::cout << "<< " << std::fixed << std::setprecision(3) << s.x << " " << s.y << "\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Blad: " << e.what() << "\n";
        return 1;
    }
    return 0;
}