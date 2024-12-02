#include "CMakeProject11.h"
using namespace std;
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <chrono>

namespace fs = std::filesystem;

// Struktura pro reprezentaci panelu
struct FilePanel {
    std::string currentPath;
    std::vector<fs::directory_entry> entries;
    int selectedIndex;

    FilePanel(const std::string& path) : currentPath(path), selectedIndex(0) {
        refreshEntries();
    }

    void refreshEntries() {
        entries.clear();
        for (const auto& entry : fs::directory_iterator(currentPath)) {
            entries.push_back(entry);
        }
    }

    fs::directory_entry selectedEntry() {
        if (entries.empty()) return {};
        return entries[selectedIndex];
    }

    void navigateUp() {
        if (selectedIndex > 0) --selectedIndex;
    }

    void navigateDown() {
        if (selectedIndex < entries.size() - 1) ++selectedIndex;
    }

    void enterDirectory() {
        if (entries.empty()) return;
        if (fs::is_directory(selectedEntry())) {
            currentPath = selectedEntry().path().string();
            selectedIndex = 0;
            refreshEntries();
        }
    }

    void goBack() {
        if (currentPath != "/") {
            currentPath = fs::path(currentPath).parent_path().string();
            selectedIndex = 0;
            refreshEntries();
        }
    }

    void display(bool isActive, int width) const {
        std::cout << (isActive ? ">>> " : "    ") << std::setw(width - 4) << std::left << currentPath << "\n";
        for (size_t i = 0; i < entries.size(); ++i) {
            if (i == selectedIndex) {
                std::cout << (isActive ? " > " : "   ");
            }
            else {
                std::cout << "   ";
            }

            const auto& entry = entries[i];
            std::string name = entry.path().filename().string();
            if (fs::is_directory(entry)) {
                name += "/";
            }

            // Získání velikosti a data poslední úpravy
            std::string size = "DIR";
            if (fs::is_regular_file(entry)) {
                size = std::to_string(fs::file_size(entry)) + " bit";
            }

            std::string lastWriteTime = "N/A";
            try {
                auto ftime = fs::last_write_time(entry);
                auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::time_point::duration>(ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
                std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);
                lastWriteTime = std::asctime(std::localtime(&cftime));
                lastWriteTime.pop_back(); // Odstranění koncového nového řádku
            }
            catch (...) {
                lastWriteTime = "Error";
            }

            std::cout << std::setw(25) << std::left << name
                << std::setw(12) << std::right << size
                << "  " << lastWriteTime << "\n";
        }
        std::cout << "\n";
    }
};

// Funkce pro vyčištění konzole
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// Hlavní funkce
int main() {
    const int panelWidth = 60;  // Šířka jednoho panelu

    FilePanel leftPanel("/");
    FilePanel rightPanel("/");
    bool activeLeft = true;

    while (true) {
        clearScreen();

        std::cout << "vita vas dvoupanelovy spravce souboru uzivatelu TomasDekom42 a VladaBallester69\n";
        std::cout << "Ovladani: w/s (nahoru/dolu), a/d (prepniti panelu), "
            "O (jako Ota) (otevrit), p (zpet), c (kopirovat), l (smazat), n (novy soubor), q (konec)\n\n";

        // Zobrazení levého panelu
        std::cout << "Levy panel\n";
        std::cout << std::string(panelWidth, '-') << "\n";
        leftPanel.display(activeLeft, panelWidth);
        std::cout << std::string(panelWidth, '-') << "\n";

        // Zobrazení pravého panelu
        std::cout << "Pravy panel\n";
        std::cout << std::string(panelWidth, '-') << "\n";
        rightPanel.display(!activeLeft, panelWidth);
        std::cout << std::string(panelWidth, '-') << "\n";

        FilePanel& activePanel = activeLeft ? leftPanel : rightPanel;

        char ch;
        std::cin >> ch;

        switch (ch) {
        case 'w': // Nahoru
            activePanel.navigateUp();
            break;
        case 's': // Dolů
            activePanel.navigateDown();
            break;
        case 'a': // Přepnout na levý panel
            activeLeft = true;
            break;
        case 'd': // Přepnout na pravý panel
            activeLeft = false;
            break;
        case 'o': // o - otevřít složku
            activePanel.enterDirectory();
            break;
        case 'p': // Backspace - zpět
            activePanel.goBack();
            break;
        case 'c': { // Kopírování souboru
            FilePanel& targetPanel = activeLeft ? rightPanel : leftPanel;
            if (!activePanel.entries.empty()) {
                fs::copy(activePanel.selectedEntry().path(),
                    targetPanel.currentPath + "/" +
                    activePanel.selectedEntry().path().filename().string());
                targetPanel.refreshEntries();
            }
            break;
        }
        case 'l': { // Smazání souboru
            if (!activePanel.entries.empty()) {
                fs::remove(activePanel.selectedEntry().path());
                activePanel.refreshEntries();
            }
            break;
        }
        case 'n': { // Vytvoření nového souboru
            std::cout << "Zadejte nazev nového souboru: ";
            std::string newFileName;
            std::cin >> newFileName;
            std::ofstream outfile(activePanel.currentPath + "/" + newFileName);
            outfile.close();
            activePanel.refreshEntries();
            break;
        }
        case 'q': // Konec programu
            return 0;
        default:
            std::cout << "Neplatna volba.\n";
        }
    }

    return 0;
}