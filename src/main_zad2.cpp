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

// Sample
struct Sample {
    double t;
    double x;
    double y;
};

// Stałe
constexpr double kTMin = 0.0;
constexpr double kTMax = 33.0;
constexpr double kTurnWindow = 10.0;
constexpr int    kHampelHalf = 2;
constexpr double kHampelK = 5.0;
constexpr double kMinThreshold = 1.0;
constexpr double kExportStep = 0.01;


// Skrót
using Track = std::vector<Sample>;

// Wczytanie Danych
std::vector<Sample> loadCsv(const std::string& path) {
    std::ifstream file(path);
    if (!file) throw std::runtime_error("Nie mozna otworzyc pliku: " + path);

    std::vector<Sample> data;
    std::string line;
    std::getline(file, line);

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        Sample s;
        char c1, c2;

        if (ss >> s.t >> c1 >> s.x >> c2 >> s.y && c1 == ',' && c2 == ',') {
            data.push_back(s);
        }
    }

    // Sortuje po czasie i usuwa duplikaty
    std::stable_sort(data.begin(), data.end(),
                     [](const Sample& l, const Sample& r) { return l.t < r.t; });
    data.erase(std::unique(data.begin(), data.end(),
                           [](const Sample& l, const Sample& r) { return l.t == r.t; }),
               data.end());

    return data;
}

// Interpolacja liniowa, zeby móc estymwoać punkty w dowolnej chwili wyścigu
Sample interpolate(const Track& d, double T) {
    if (d.empty()) throw std::logic_error("interpolate: pusty tor");
    if (T <= d.front().t) return {T, d.front().x, d.front().y};
    if (T >= d.back().t) return {T, d.back().x, d.back().y};

    auto it = std::upper_bound(d.begin(), d.end(), T,
                               [](double val, const Sample& s) { return val < s.t; });

    const Sample& b = *it;
    const Sample& a = *std::prev(it);

    const double dt = b.t - a.t;
    const double k = (dt > 0.0) ? (T - a.t) / dt : 0.0;

    // Interpolacja liniowa położenia X oraz Y
    return {
        T,
        a.x + k * (b.x - a.x),
        a.y + k * (b.y - a.y)
    };
}

// Znalezienie, czy y>0 czy <0
int firstTurnSign(const std::vector<Sample>& d) {
    const double t0 = d.front().t;
    double best = 0.0;
    for (const Sample& s : d) {
        if (s.t - t0 > kTurnWindow) break;
        if (std::abs(s.y) > std::abs(best)) best = s.y;
    }
    return best < 0.0 ? -1 : 1;
}

// Y -> -Y
void normalizeFrame(Track& d) {
    if (firstTurnSign(d) > 0) {
        for (Sample& s : d) {
            s.y = -s.y;
        }
    }
}

// Szybkie znajdywanie mediany O(n)
double median(std::vector<double> v) {
    if (v.empty()) return 0.0;

    const size_t n = v.size();
    auto mid = v.begin() + n / 2;
    std::nth_element(v.begin(), mid, v.end());

    if (n % 2 != 0) return *mid;

    auto max_low = std::max_element(v.begin(), mid);
    return 0.5 * (*mid + *max_low);
}

Track buildCorrection(const Track& fast, const Track& slow, std::vector<char>& accepted) {
    const size_t n = slow.size();

    // Wyznaczenie błędu
    Track res(n);
    for (size_t i = 0; i < n; ++i) {
        const Sample f = interpolate(fast, slow[i].t);
        res[i] = {slow[i].t, slow[i].x - f.x, slow[i].y - f.y};
    }

    // Filtr Hampela
    std::vector<double> dev(n);
    for (size_t i = 0; i < n; ++i) {
        const size_t lo = (i >= static_cast<size_t>(kHampelHalf)) ? i - kHampelHalf : 0;
        const size_t hi = std::min(n - 1, i + kHampelHalf);

        std::vector<double> wx, wy;
        wx.reserve(hi - lo + 1);
        wy.reserve(hi - lo + 1);
        for (size_t j = lo; j <= hi; ++j) {
            wx.push_back(res[j].x);
            wy.push_back(res[j].y);
        }
        dev[i] = std::hypot(res[i].x - median(wx), res[i].y - median(wy));
    }

    const double sigma = 1.4826 * median(dev);
    const double threshold = std::max(kMinThreshold, kHampelK * sigma);

    // Przefiltrowanie próbek oraz utworzenei wektora poprawek
    accepted.assign(n, 0);
    Track corr;
    corr.reserve(n);
    for (size_t i = 0; i < n; ++i) {
        if (dev[i] <= threshold) {
            accepted[i] = 1;
            corr.push_back(res[i]);
        }
    }

    if (corr.empty()) throw std::runtime_error("Wszystkie pomiary rzadkiego sensora odrzucone");
    return corr;
}

// Tworzy fuzję ścieżek
Sample fusedAt(const Track& fast, const Track& corr, double T) {
    const Sample f = interpolate(fast, T);
    const Sample e = interpolate(corr, T);
    return {T, f.x + e.x, f.y + e.y};
}

void writeTrack(const std::string& path, const Track& tr, const std::vector<char>* accepted = nullptr) {
    std::ofstream out(path);
    if (!out) return;

    out << "t,x,y" << (accepted ? ",accepted" : "") << "\n" << std::fixed << std::setprecision(4);
    for (size_t i = 0; i < tr.size(); ++i) {
        out << tr[i].t << "," << tr[i].x << "," << tr[i].y;
        if (accepted) out << "," << static_cast<int>((*accepted)[i]);
        out << "\n";
    }
}

int main(int argc, char** argv) {
    std::string path1 = "../data/DaneDoZadan/zad2/sensor1.csv";
    std::string path2 = "../data/DaneDoZadan/zad2/sensor2.csv";
    std::string exportPrefix = "output";

    if (argc >= 3) {
        path1 = argv[1];
        path2 = argv[2];
        if (argc >= 4) exportPrefix = argv[3];
    } else {
        std::ifstream testFile(path1);
        if (!testFile.good()) {
            path1 = "sensor1.csv";
            path2 = "sensor2.csv";
        }
    }

    try {
        Track a = loadCsv(path1);
        Track b = loadCsv(path2);
        if (a.size() < 2 || b.size() < 2) throw std::runtime_error("Za malo danych w plikach");

        normalizeFrame(a);
        normalizeFrame(b);

        const bool aIsFast = a.size() >= b.size();
        const Track& fast = aIsFast ? a : b;
        const Track& slow = aIsFast ? b : a;

        std::vector<char> accepted;
        const Track corr = buildCorrection(fast, slow, accepted);

        Track fused;
        fused.reserve(static_cast<size_t>(kTMax / kExportStep) + 1);
        for (int k = 0; k * kExportStep <= kTMax + 1e-9; ++k) {
            fused.push_back(fusedAt(fast, corr, k * kExportStep));
        }
        writeTrack(exportPrefix + "_fast.csv", fast);
        writeTrack(exportPrefix + "_slow.csv", slow, &accepted);
        writeTrack(exportPrefix + "_fused.csv", fused);

        std::string line;
        while (true) {
            std::cout << "<< Podaj T\n>> " << std::flush;
            if (!std::getline(std::cin, line)) break;
            std::replace(line.begin(), line.end(), ',', '.');

            double T = 0.0;
            try {
                T = std::stod(line);
            } catch (const std::exception&) {
                std::cout << "<< Niepoprawna liczba!\n";
                continue;
            }
            if (!(T >= kTMin && T <= kTMax)) {
                std::cout << "<< Wartosc spoza zakresu (0 - 33 s)!\n";
                continue;
            }
            const Sample s = fusedAt(fast, corr, T);
            std::cout << "<< Pozycja w t=" << T << "s -> X: "
                      << std::fixed << std::setprecision(3) << s.x
                      << " m, Y: " << s.y << " m\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Blad: " << e.what() << "\n";
        return 1;
    }
    return 0;
}