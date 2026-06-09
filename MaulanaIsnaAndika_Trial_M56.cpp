#include <iostream>
#include <string>
#include <ctime>

#define COLOR_RESET   "\033[0m"
#define COLOR_GREEN   "\033[1;32m"
#define COLOR_YELLOW  "\033[1;33m"
#define COLOR_RED     "\033[1;31m"
#define COLOR_CYAN    "\033[1;36m"
#define COLOR_GRAY    "\033[0;90m"
#define COLOR_WHITE   "\033[1;37m"
#define COLOR_MAGENTA "\033[1;35m"
#define clear "\033[2J\033[H"

using namespace std;

struct commit{
    string id;
    string author;
    string pesan;
    string tanggal;
    commit* next;
};

struct branch{
    string nama;
    commit* headCommit;
    int commitCount;
    branch* next;
};

struct repository{
    string nama;
    branch* branchList;
    branch* branchAktif;
    repository* next;
};

string generateShortHash(int commitId) {
    const char hexChars[] = "0123456789abcdef";
    string hash = "";
    unsigned int value = commitId * 2654435761u;
    for (int i = 0; i < 7; i++) {
        hash = hexChars[value & 0xF] + hash;
        value >>= 4;
    }
    return hash;
}

string getCurrentTimestamp() {
    time_t now = time(nullptr);
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", localtime(&now));
    return string(buffer);
}

int main(int argc, char* argv[]){

    if (argc < 2) {
        cout << COLOR_RED << "[ERROR] Jalankan program dengan menyertakan nama Author!" << COLOR_RESET << endl;
        cout << "Contoh: .\\gitsim nama_kamu" << endl;
        return 1;
    }

    string namaAuthor = argv[1];
    repository* repoHead = nullptr;
    repository* repoAktif = nullptr;
    int totalRepo = 0;
    int commitIdCounter = 1;

    cout << COLOR_CYAN << "GITSIM" << COLOR_RESET << endl; 
    cout << "Lightweight Git Simulator" << endl; 
    cout << "Author: " << COLOR_GREEN << namaAuthor << COLOR_RESET << endl << endl; 
    cout << "git init" << endl; 
    cout << "Repository name: ";

    string repoInput;
    getline(cin, repoInput);

    if (repoInput.empty()){
        repoInput = "my-repo";
    }

    repository* repoBaru = new repository;
    repoBaru->nama = repoInput;
    repoBaru->next = nullptr;

    repoHead = repoBaru;
    repoAktif = repoBaru;
    totalRepo++;

    branch* branchAwal = new branch;
    branchAwal->nama = "main";
    branchAwal->headCommit = nullptr;
    branchAwal->commitCount = 0;
    branchAwal->next = nullptr;

    repoAktif->branchList = branchAwal;
    repoAktif->branchAktif = branchAwal;

    cout << COLOR_GREEN << "[OK] Initialized empty repository: " << repoAktif->nama << COLOR_RESET << endl; 
    cout << "On branch: " << COLOR_GREEN << repoAktif->branchAktif->nama << COLOR_RESET << endl; 
    cout << "Press Enter...";
    cin.get();

    int pilihan = -1;
    int indeksRepo = 1;

    while (pilihan != 0){
        cout << clear;

        cout << COLOR_CYAN << "GITSIM Git Simulator" << COLOR_RESET << endl;
        cout << "Author: " << COLOR_GREEN << namaAuthor << COLOR_RESET 
             << "  Repo: " << COLOR_GREEN << repoAktif->nama << COLOR_RESET 
             << "  HEAD: " << COLOR_GREEN << repoAktif->branchAktif->nama << COLOR_RESET 
             << " | [" << indeksRepo << "/" << totalRepo << "]" << endl << endl;

        cout << "[1] git commit" << endl;
        cout << "[2] git log" << endl;
        cout << "[3] git branch" << endl;
        cout << "[4] git checkout" << endl;
        cout << "[5] new repository" << endl;
        cout << "[6] switch repository" << endl;
        cout << "[0] exit" << endl << endl;

        cout << "Select option: ";
        cin >> pilihan;
        cin.ignore();

        if (pilihan == 1){
            cout << "\n--- git commit [" << COLOR_GREEN << repoAktif->branchAktif->nama << COLOR_RESET << "] ---" << endl;

            string commitMassage;
            cout << "Message: ";
            getline(cin, commitMassage);

            if(commitMassage.empty()){
                cout << COLOR_RED << "[ERROR] Message cannot be empty!" << COLOR_RESET << endl;
                cout << "\nPress Enter...";
                cin.get();
                continue;
            }

            char konfirmasi;
            cout << "Push commit? (y/n): ";
            cin >> konfirmasi;
            cin.ignore();

            if (konfirmasi == 'y' || konfirmasi == 'Y'){
                commit* commitBaru = new commit;
                commitBaru->id = generateShortHash(commitIdCounter);
                commitBaru->pesan = commitMassage;
                commitBaru->author = namaAuthor;
                commitBaru->tanggal = getCurrentTimestamp();
                commitBaru->next = repoAktif->branchAktif->headCommit;
                repoAktif->branchAktif->headCommit = commitBaru;
                repoAktif->branchAktif->commitCount++;
                commitIdCounter++;

                cout << COLOR_GREEN << "[" << repoAktif->branchAktif->nama << " " << commitBaru->id << "] " << commitBaru->pesan << COLOR_RESET << endl;
                cout << COLOR_GREEN << repoAktif->branchAktif->nama << " -> origin/" << repoAktif->branchAktif->nama << COLOR_RESET << endl;
                cout << " $ git push origin " << repoAktif->branchAktif->nama << endl;
            }
            else{
                cout << COLOR_YELLOW << "[INFO] Commit dibatalkan." << COLOR_RESET << endl;
            }
            cout << "\nPress Enter...";
            cin.get();
        }
        else if (pilihan == 2){
            cout << "\n--- git log [" << COLOR_GREEN << repoAktif->branchAktif->nama << COLOR_RESET << "] ---" << endl;

            commit* current = repoAktif->branchAktif->headCommit;

            if (current == nullptr){
                cout << COLOR_YELLOW << "(No commits on this branch)" << COLOR_RESET << endl;
            }
            else{
                while (current != nullptr){
                    cout << COLOR_YELLOW << "commit  : " << current->id << COLOR_RESET << endl;
                    cout << "Author  : " << current->author << endl;
                    cout << "Date    : " << current->tanggal << endl << endl;
                    cout << "    " << COLOR_WHITE << current->pesan << COLOR_RESET << endl;
                    cout << COLOR_GRAY << "---------------------------------------" << COLOR_RESET << endl;
                    current = current->next;
                }
            }
            cout << "\nPress Enter...";
            cin.get();
        }
        else if (pilihan == 3){
            cout << "\n--- git branch ---" << endl;

            branch* currentBranch = repoAktif->branchList;
            while(currentBranch != nullptr){
                if(currentBranch == repoAktif->branchAktif){
                    cout << COLOR_GREEN << "* " << currentBranch->nama << " (" << currentBranch->commitCount << " commits)" << COLOR_RESET << endl;
                }
                else{
                    cout << "  " << currentBranch->nama << " (" << currentBranch->commitCount << " commits)" << endl;
                }
                currentBranch = currentBranch->next;
            }
            string newBranch;
            cout << "\nNew branch name: ";
            getline(cin, newBranch);

            if(newBranch.empty()){
                cout << COLOR_RED << "[ERROR] Branch name cannot be empty!" << COLOR_RESET << endl;
                cout << "\nPress Enter...";
                cin.get();
                continue;
            }

            bool duplicate = false;
            branch* checkBranch = repoAktif->branchList;

            while(checkBranch != nullptr){
                if(checkBranch->nama == newBranch){
                    duplicate = true;
                    break;
                }
                checkBranch = checkBranch->next;
            }

            if(duplicate){
                cout << COLOR_RED << "[ERROR] Branch '" << newBranch << "' already exists!" << COLOR_RESET << endl;
                cout << "\nPress Enter...";
                cin.get();
                continue;
            }
            
            branch* newBranch2 = new branch;
            newBranch2->nama = newBranch;
            newBranch2->headCommit = repoAktif->branchAktif->headCommit;
            newBranch2->commitCount = repoAktif->branchAktif->commitCount;
            newBranch2->next = nullptr;
            
            branch* branchTerakhir = repoAktif->branchList;
            while(branchTerakhir->next != nullptr){
                branchTerakhir = branchTerakhir->next;
            }
            branchTerakhir->next = newBranch2;

            cout << COLOR_GREEN << "[OK] Branch '" << newBranch << "' created from '" << repoAktif->branchAktif->nama << "'" << COLOR_RESET << endl;
            cout << "     " << newBranch2->commitCount << " commit(s) inherited" << endl;

            cout << "\nPress Enter...";
            cin.get();
        }
        else if (pilihan == 4){
            cout << "\n--- git checkout ---" << endl;

            branch* branchSekarang = repoAktif->branchList;
            while (branchSekarang != nullptr){
                if(branchSekarang == repoAktif->branchAktif){
                    cout << COLOR_GREEN << "* " << branchSekarang->nama << COLOR_RESET << endl;
                }
                else{
                    cout << "  " << branchSekarang->nama << endl;
                }
                branchSekarang = branchSekarang->next;
            }

            string namaBranchTujuan;
            cout << "Switch to branch: ";
            getline(cin, namaBranchTujuan);

            branch* branchKetemu = nullptr;
            branch* cariPtr = repoAktif->branchList;

            while(cariPtr != nullptr){
                if(cariPtr->nama == namaBranchTujuan){
                    branchKetemu = cariPtr;
                    break;
                }
                cariPtr = cariPtr->next;
            }
            if (branchKetemu == nullptr || branchKetemu == repoAktif->branchAktif){
                cout << COLOR_RED << "[ERROR] Invalid branch!" << COLOR_RESET << endl;
            }
            else{
                repoAktif->branchAktif = branchKetemu;
                cout << COLOR_GREEN << "[OK] Switched to branch '" << repoAktif->branchAktif->nama << "'" << COLOR_RESET << endl;
            }

            cout << "\nPress Enter...";
            cin.get();
        }
        else if (pilihan == 5){
            cout << "\n--- git init (new repository) ---" << endl;

            string namaRepositoryBaru;
            cout << "New repository name: ";
            getline(cin, namaRepositoryBaru);

            if(namaRepositoryBaru.empty()){
                namaRepositoryBaru = "repo-" + to_string(totalRepo + 1);
            }

            bool repoDuplicated = false;
            repository* checkRepository = repoHead;

            while(checkRepository != nullptr){
                if(checkRepository->nama == namaRepositoryBaru){
                    repoDuplicated = true;
                    break;
                }
                checkRepository = checkRepository->next;
            }

            if(repoDuplicated){
                cout << COLOR_RED << "[ERROR] Repository '" << namaRepositoryBaru << "' already exists!" << COLOR_RESET << endl;
                cout << "\nPress Enter...";
                cin.get();
                continue;
            }

            repository* brandRepoBaru = new repository;
            brandRepoBaru->nama = namaRepositoryBaru;
            brandRepoBaru->next = nullptr;

            branch* repositoryBranchUtama = new branch;
            repositoryBranchUtama->nama = "main";
            repositoryBranchUtama->headCommit= nullptr;
            repositoryBranchUtama->commitCount = 0;
            repositoryBranchUtama->next = nullptr;
            brandRepoBaru->branchList = repositoryBranchUtama;
            brandRepoBaru->branchAktif = repositoryBranchUtama;

            repository* repositoryTerakhir = repoHead;
            while (repositoryTerakhir->next != nullptr) {
                repositoryTerakhir = repositoryTerakhir->next;
            }
            repositoryTerakhir->next = brandRepoBaru;
            repoAktif = brandRepoBaru;
            totalRepo++;
            indeksRepo = totalRepo;

            cout << COLOR_GREEN << "[OK] Repository '" << namaRepositoryBaru << "' created and set as active." << COLOR_RESET << endl;
            cout << "On branch: main" << endl;

            cout << "\nPress Enter...";
            cin.get();
        }
        else if (pilihan == 6){
            cout << "\n--- switch repository ---" << endl;

            repository* repoSekarang = repoHead;
            int nomorUrut = 1;

            while (repoSekarang != nullptr) {
                int hitungBranch = 0;
                branch* bPtr = repoSekarang->branchList;
                while (bPtr != nullptr) {
                    hitungBranch++;
                    bPtr = bPtr->next;
                }

                int commitHead = repoSekarang->branchAktif->commitCount;

                if (repoSekarang == repoAktif) {
                    cout << COLOR_GREEN << "* [" << nomorUrut << "] " << repoSekarang->nama << " (" << hitungBranch << " branch, " << commitHead << " commits at HEAD)" << COLOR_RESET << endl;
                } else {
                    cout << "  [" << nomorUrut << "] " << repoSekarang->nama << " (" << hitungBranch << " branch, " << commitHead << " commits at HEAD)" << endl;
                }
                repoSekarang = repoSekarang->next;
                nomorUrut++;
            }

            int pilihanNomor;
            cout << "\nSelect repository number: ";
            cin >> pilihanNomor;
            cin.ignore();

            if (pilihanNomor < 1 || pilihanNomor > totalRepo) {
                cout << COLOR_RED << "[ERROR] Nomor repository tidak valid!" << COLOR_RESET << endl;
            } 
            else {
                repository* targetRepo = repoHead;
                for (int i = 1; i < pilihanNomor; i++) {
                    targetRepo = targetRepo->next;
                }

                repoAktif = targetRepo;
                indeksRepo = pilihanNomor;

                cout << COLOR_GREEN << "[OK] Switched to repository '" << repoAktif->nama << "'" << COLOR_RESET << endl;
                cout << "HEAD: " << repoAktif->branchAktif->nama << endl;
            }

            cout << "\nPress Enter...";
            cin.get();
        }
        else if (pilihan == 0){
            cout << COLOR_MAGENTA << "\nSession Ended" << COLOR_RESET << endl;
            cout << "Author: " << namaAuthor << endl;
            cout << COLOR_CYAN << "Goodbye!" << COLOR_RESET << endl;

            repository* currRepo = repoHead;
            while (currRepo != nullptr) {
                branch* currBranch = currRepo->branchList;
                while (currBranch != nullptr) {
                    commit* currCommit = currBranch->headCommit;
                    while (currCommit != nullptr) {
                        commit* tempCommit = currCommit;
                        currCommit = currCommit->next;
                        delete tempCommit;
                    }
                    branch* tempBranch = currBranch;
                    currBranch = currBranch->next;
                    delete tempBranch;
                }
                repository* tempRepo = currRepo;
                currRepo = currRepo->next;
                delete tempRepo;
            }
            break;
        }
        else{
            cout << COLOR_RED << "\n[ERROR] Pilihan menu tidak valid!" << COLOR_RESET << endl;
            cout << "Press Enter...";
            cin.get();
        }
    }

    return 0;
}