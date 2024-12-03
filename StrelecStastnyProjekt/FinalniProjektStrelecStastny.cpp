#include "FinalniProjektStrelecStastny.h"
// CMakeProject16.cpp: Definuje vstupní bod pro aplikaci.
//


#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <chrono>
#include <ctime>

namespace fs = std::filesystem; // nadefinovani fs

// Struktura pro reprezentaci schránky
struct Clipboard {
    std::set<fs::path> files; // unikatnost kazde slozky

    void add(const fs::path& file) {
        files.insert(file);
    } // definice funkce add, zajisteni unikatnosti

    void clear() {
        files.clear();
    }
};          // definice funkce clear

// Struktura pro reprezentaci panelu
struct FilePanel {
    std::string currentPath;
    std::vector<fs::directory_entry> entries;   // definice vectoru entries, directory_entry je typ z knihovny filesystem
    int selectedIndex;
    std::set<fs::path> selectedFiles; // Soubory vybrané pro hromadné operace

    FilePanel(const std::string& path) : currentPath(path), selectedIndex(0) {
        refreshEntries();
    }  // konstruktor, zacina jednotlivy panel, proto selectedindex 0, protoze prvni polozka v seznamu

    void refreshEntries();
    fs::directory_entry selectedEntry();
    void navigateUp();//  klavesa w
        void navigateDown(); // klavesa s
        void enterDirectory(); // klavesa o
        void goBack(); // klavesa p
        void toggleSelection();   // soubor/slozka
    void clearSelection();    // zruseni vyberu
    void createNewFile();     // klavesa n
    void createNewFolder();   // klavesa k
    void deleteSelectedFile(); // klavesa l
    void displayRow(size_t rowIndex, bool isActive, int width) const;
    std::string getLastModifiedTime(const fs::path& path) const;
    std::string getFileSizeOrDir(const fs::directory_entry& entry) const; // definice jednotlivych funkci
};

// implementace FilePanel
void FilePanel::refreshEntries() {
    entries.clear();
    try {
        for (const auto& entry : fs::directory_iterator(currentPath)) {
            entries.push_back(entry);
        }
    }        // try vyzkousi tento kod
    catch (const std::exception& e) {
        std::cerr << "Chyba pri ctení adresare: " << e.what() << "\n";
    }
}      // kdyby nastala chyba

fs::directory_entry FilePanel::selectedEntry() {
    if (entries.empty()) return {};  // overuje zda je seznam entries prazdny
    return entries[selectedIndex];
} // kdyz prazdny vrati promenou selectedIndex    

void FilePanel::navigateUp() {
    if (selectedIndex > 0) {
        --selectedIndex;
    }
} //snizeni promenne selectedindex o jedna, posun dolu

void FilePanel::navigateDown() {
    if (selectedIndex < entries.size() - 1) {
        ++selectedIndex;
    }// zvyseni promenne selectedIndex o jedna, posun nahoru
}

void FilePanel::enterDirectory() {
    if (!entries.empty() && fs::is_directory(selectedEntry())) {
        currentPath = selectedEntry().path().string();
        selectedIndex = 0;
        refreshEntries();
        clearSelection();
    } //vstoupeni do slozky, klavesa o
}

void FilePanel::goBack() {
    if (currentPath != "/") {
        currentPath = fs::path(currentPath).parent_path().string();
        selectedIndex = 0;
        refreshEntries();
        clearSelection();
    } // klavesa p, jit zpatky
}

void FilePanel::toggleSelection() {
    if (!entries.empty()) {
        fs::path filePath = selectedEntry().path();
        if (selectedFiles.count(filePath)) {
            selectedFiles.erase(filePath); // Zrušení výběru
        }
        else {
            selectedFiles.insert(filePath); // Přidání do výběru
        }
    }
}  // vyber jednotlivych souboru pro praci s nimi, klavesa m

void FilePanel::clearSelection() {
    selectedFiles.clear();
} //odstraneni vsech vybranych souboru

void FilePanel::createNewFile() {
    std::cout << "Zadejte nazev noveho souboru: ";
    std::string fileName;
    std::cin >> fileName;
    fs::path filePath = fs::path(currentPath) / fileName;
    // vytvoreni noveho souboru, klavesa n
    try {
        std::ofstream(filePath.string()); // Vytvoří prázdný soubor
        refreshEntries();
        std::cout << "Soubor \"" << fileName << "\" vytvoren.\n";
    }
    catch (const std::exception& e) {
        std::cerr << "Chyba pri vytvareni souboru: " << e.what() << "\n"; // chyba pri vytvareni
    }
}

void FilePanel::createNewFolder() {
    std::cout << "Zadejte nazev nove slozky: ";
    std::string folderName;
    std::cin >> folderName;
    fs::path folderPath = fs::path(currentPath) / folderName;       // vytvareni nove slozky, klavesa k

    try {
        fs::create_directory(folderPath); // Vytvoření složky
        refreshEntries();

    }
    catch (const std::exception& e) {
        std::cerr << "Chyba pri vytvareni slozky: " << e.what() << "\n";
    }
}

void FilePanel::deleteSelectedFile() {
    if (entries.empty()) {
        std::cout << "Zadny soubor k odstraneni.\n";
        return;
    }            // funkce na smazani souboru, klavesa l

    fs::path filePath = selectedEntry().path();
    std::cout << "Opravdu chcete smazat \"" << filePath.filename().string() << "\"? (y/n): ";
    char confirmation;
    std::cin >> confirmation;

    if (confirmation == 'y' || confirmation == 'Y') {        //potvrzeni volby smazani
        try {
            if (fs::is_directory(filePath)) {
                fs::remove_all(filePath); // Smaže složku a její obsah
            }
            else {
                fs::remove(filePath); // Smaže soubor
            }
            refreshEntries(); // Aktualizuje obsah panelu
            std::cout << "Soubor/slozka \"" << filePath.filename().string() << "\" odstranen(a).\n";
        }
        catch (const std::exception& e) {
            std::cerr << "Chyba pri odstranovani: " << e.what() << "\n";
        }
    }
    else {
        std::cout << "Odstraneni zruseno.\n";
    }
}

std::string FilePanel::getLastModifiedTime(const fs::path& path) const {
    try {
        auto ftime = fs::last_write_time(path);
        auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
        std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);
        char buffer[20];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", std::localtime(&cftime));
        return std::string(buffer);
    }
    catch (...) {
        return "N/A";
    }
}        // funkce na ziskavani casu posledni upravy

std::string FilePanel::getFileSizeOrDir(const fs::directory_entry& entry) const {
    if (fs::is_directory(entry)) {
        return "DIR";
    }
    try {
        auto size = fs::file_size(entry);
        return std::to_string(size) + " B";
    }
    catch (...) {
        return "N/A";
    }
} // funkce na ziskavani velikosti souboru ci slozky

void FilePanel::displayRow(size_t rowIndex, bool isActive, int width) const {
    if (rowIndex == 0) {
        std::cout << (isActive ? ">>> " : "    ") << std::setw(width - 4) << std::left << currentPath;
    }
    else if (rowIndex - 1 < entries.size()) {
        const auto& entry = entries[rowIndex - 1];
        std::string name = entry.path().filename().string();
        if (fs::is_directory(entry)) {
            name += "/";
        }
        std::string sizeOrDir = getFileSizeOrDir(entry);
        std::string modifiedTime = getLastModifiedTime(entry.path());

        bool isSelected = selectedFiles.count(entry.path()) > 0;

        std::cout << (rowIndex - 1 == selectedIndex ? " > " : "   ")
            << (isSelected ? "*" : " ") // Označení vybraného souboru
            << std::setw(width - 20) << name
            << std::setw(12) << sizeOrDir
            << modifiedTime;
    }
    else {
        std::cout << std::setw(width) << " ";
    }
}  // struktura panelu

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
    const int panelWidth = 60;  // Nastavuje konstantní šířku pro každý panel
    FilePanel leftPanel("/"); // Levý a pravy panel zobrazující obsah kořenového adresáře ("/")
    FilePanel rightPanel("/");
    Clipboard clipboard; // Schránka pro kopírování souboru a složek
    bool activeLeft = true; // definice proměnné bool pro navazující while

    while (true) { //pokud je proměnná active=true
        clearScreen();

        std::cout << "=<=<=< Dvou-panelovy spravce souboru >=>=>=\n";
        std::cout << "Ovladani pomoci funkcnich klaves: w/s (nahoru/dolu), a/d (prepnuti panelu), m (vybrat vice), "
            "c (kopirovat), v (vlozit), n (novy soubor), k (nova slozka), l (smazat), "
            "o (otevrit), p (zpet), q (konec)\n Pokyny k pouziti: Pred kazdym kopirovanim je nutne nejdrive soubor ci slozku oznacit klavesou m\n";

        size_t maxRows = std::max(leftPanel.entries.size(), rightPanel.entries.size()) + 1; //nastavuje počet řádků potřebný pro zobrazení panelu +1

        for (size_t i = 0; i < maxRows; ++i) {
            leftPanel.displayRow(i, activeLeft, panelWidth);
            std::cout << " | "; // panely jsou odděleny svislou čarou
            rightPanel.displayRow(i, !activeLeft, panelWidth);
            std::cout << "\n";
        } //zobrazení obou panelů na jeden řádek

        FilePanel& activePanel = activeLeft ? leftPanel : rightPanel; // indikator aktivniho panelu

        char ch;
        std::cin >> ch; //zadani funkcnich klaves

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
        case 'm': // Výběr více souborů
            activePanel.toggleSelection();
            break;
        case 'c': // Kopírování
            for (const auto& file : activePanel.selectedFiles) {
                clipboard.add(file);
            }
            std::cout << "Vybrane polozky byly zkopirovany do schranky.\n";
            break;
        case 'v': // Vložení
            for (const auto& file : clipboard.files) {
                fs::path destination = fs::path(activePanel.currentPath) / file.filename();
                try {
                    fs::copy(file, destination, fs::copy_options::recursive);
                }
                catch (const std::exception& e) {
                    std::cerr << "Chyba pri vkladani: " << e.what() << "\n";
                }
            }
            clipboard.clear();
            activePanel.refreshEntries();
            break;
        case 'n': // Nový soubor
            activePanel.createNewFile();
            break;
        case 'k': // Nová složka
            activePanel.createNewFolder();
            break;
        case 'l': // Smazání souboru
            activePanel.deleteSelectedFile();
            break;
        case 'o': // Otevřít složku
            activePanel.enterDirectory();
            break;
        case 'p': // Zpět
            activePanel.goBack();
            break;
        case 'q': // Ukončit program
            return 0;
        default:
            std::cout << "Neplatna volba.\n";
        }
    }
}

 

 
