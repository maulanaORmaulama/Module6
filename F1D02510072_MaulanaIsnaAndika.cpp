#include <iostream>
#include <cstdlib>
#include <climits>

constexpr size_t MAX_SLOTS = 128;
constexpr size_t BUF_SIZE = 512;

enum DataType {
    TYPE_CHAR_PTR = 0,
    TYPE_UINT = 1,
    TYPE_DOUBLE = 2
};

struct Memory_Entry {
    DataType type;
    size_t offset;    
    size_t size;      
    bool is_deleted;
};

struct Sister {
    const char* name;
    unsigned char* pool;
    size_t pool_size;
    size_t bump_pointer;
    size_t alignment;
    size_t special_gap;
    Memory_Entry entries[MAX_SLOTS];
    size_t entry_count;
    size_t used_slots;
    size_t used_bytes;
};

static size_t manual_strlen(const char* str) {
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

static void manual_memcpy(void* dest, const void* src, size_t n) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }
}

static bool validate_nim(const char* nim) {
    if (manual_strlen(nim) != 11) return false;
    const char* prefix = "F1D02";
    for (size_t i = 0; i < 5; i++) {
        if (nim[i] != prefix[i]) return false;
    }
    for (size_t i = 5; i < 11; i++) {
        if (nim[i] < '0' || nim[i] > '9') return false;
    }
    return true;
}

static void clear_screen() {
    std::cout << "\033[2J\033[H" << std::flush;
}

static void wait_for_enter(bool success = true) {
    if (success) {
        std::cout << "[OK] Tekan ENTER untuk melanjutkan...";
    } else {
        std::cout << "[GAGAL] Tekan ENTER untuk melanjutkan...";
    }
    std::cin.get();
}

static bool read_long_long(long long& out_val) {
    char buf[BUF_SIZE];
    std::cin.getline(buf, BUF_SIZE);
    
    size_t len = manual_strlen(buf);
    if (len == 0) return false;
    
    size_t start = 0;
    bool negative = false;
    if (buf[0] == '-') {
        negative = true;
        start = 1;
        if (len == 1) return false;
    } else if (buf[0] == '+') {
        start = 1;
        if (len == 1) return false;
    }
    
    long long res = 0;
    for (size_t i = start; i < len; i++) {
        if (buf[i] < '0' || buf[i] > '9') return false;
        long long digit = buf[i] - '0';
        if (res > (LLONG_MAX - digit) / 10) return false;
        res = res * 10 + digit;
    }
    
    out_val = negative ? -res : res;
    return true;
}

static bool read_double(double& out_val) {
    char buf[BUF_SIZE];
    std::cin.getline(buf, BUF_SIZE);
    size_t len = manual_strlen(buf);
    if (len == 0) return false;
    
    size_t start = 0;
    bool negative = false;
    if (buf[0] == '-') {
        negative = true;
        start = 1;
    } else if (buf[0] == '+') {
        start = 1;
    }
    
    double res = 0.0;
    size_t i = start;
    bool dot_found = false;
    double divisor = 10.0;
    
    size_t digits = 0;
    for (; i < len; i++) {
        if (buf[i] == '.') {
            if (dot_found) return false;
            dot_found = true;
            continue;
        }
        if (buf[i] < '0' || buf[i] > '9') return false;
        
        digits++;
        if (!dot_found) {
            res = res * 10.0 + (buf[i] - '0');
        } else {
            res += (buf[i] - '0') / divisor;
            divisor *= 10.0;
        }
    }
    if (digits == 0) return false;
    out_val = negative ? -res : res;
    return true;
}

static size_t align_up(size_t current_offset, size_t alignment) {
    size_t remainder = current_offset % alignment;
    if (remainder == 0) return current_offset;
    return current_offset + (alignment - remainder);
}

static void init_sister(Sister& sister, const char* name, size_t pool_size, size_t alignment, size_t special_gap) {
    sister.name = name;
    sister.pool_size = pool_size;
    sister.alignment = alignment;
    sister.special_gap = special_gap;
    sister.pool = (unsigned char*)std::malloc(sister.pool_size);
    if (!sister.pool) {
        std::cout << "FATAL ERROR: Pemesanan Heap gagal di sektor " << sister.name << "." << std::endl;
        std::exit(1);
    }
    sister.bump_pointer = 0;
    sister.entry_count = 0;
    sister.used_slots = 0;
    sister.used_bytes = 0;
}

static void free_sister(Sister& sister) {
    if (sister.pool) {
        std::free(sister.pool);
        sister.pool = nullptr;
    }
}

static bool allocate_memory_core(Sister& sister, DataType type, const void* data_ptr, size_t data_size) {
    if (sister.entry_count >= MAX_SLOTS) {
        std::cout << "Entry overflow" << std::endl;
        wait_for_enter(false); 
        return false;
    }
    
    size_t aligned_base = align_up(sister.bump_pointer, sister.alignment);
    size_t final_offset = aligned_base;
    
    if (type == TYPE_CHAR_PTR) {
        final_offset += sister.special_gap;
    }
    
    if (final_offset + data_size > sister.pool_size) {
        std::cout << "Not enough space" << std::endl;
        return false;
    }
    
    manual_memcpy(sister.pool + final_offset, data_ptr, data_size);
    
    size_t idx = sister.entry_count;
    sister.entries[idx].type = type;
    sister.entries[idx].offset = final_offset;
    sister.entries[idx].size = data_size;
    sister.entries[idx].is_deleted = false;
    
    sister.entry_count++;
    sister.used_slots++;
    sister.used_bytes += data_size;
    sister.bump_pointer = final_offset + data_size;
    
    return true;
}

static bool add_memory_uint(Sister& sister, unsigned int val) {
    return allocate_memory_core(sister, TYPE_UINT, &val, sizeof(val));
}

static bool add_memory_double(Sister& sister, double val) {
    return allocate_memory_core(sister, TYPE_DOUBLE, &val, sizeof(val));
}

static bool add_memory_string(Sister& sister, const char* str) {
    size_t s_len = manual_strlen(str) + 1;
    return allocate_memory_core(sister, TYPE_CHAR_PTR, str, s_len);
}

static void delete_memory(Sister& sister) {
    while (sister.entry_count > 0 && sister.entries[sister.entry_count - 1].is_deleted) {
        sister.entry_count--;
    }
    
    if (sister.entry_count == 0) {
        sister.bump_pointer = 0;
    } else {
        size_t last_idx = sister.entry_count - 1;
        sister.bump_pointer = sister.entries[last_idx].offset + sister.entries[last_idx].size;
    }
}

static void show_memories(const Sister& sister) {
    std::cout << "------------------------------------------------------------" << std::endl;
    std::cout << "Memori dari " << sister.name << " " << std::endl;
    std::cout << "------------------------------------------------------------" << std::endl;
    
    size_t active_printed = 0;
    for (size_t i = 0; i < sister.entry_count; i++) {
        if (sister.entries[i].is_deleted) continue;
        
        active_printed++;
        std::cout << "[" << i << "] Tipe: ";
        if (sister.entries[i].type == TYPE_CHAR_PTR) std::cout << "char*";
        else if (sister.entries[i].type == TYPE_UINT) std::cout << "uint";
        else std::cout << "double";
        
        std::cout << " | Ukuran: " << sister.entries[i].size;
        
        if (sister.entries[i].type == TYPE_CHAR_PTR) {
            std::cout << " | Offset: " << sister.entries[i].offset;
        }
        
        void* addr = (void*)(sister.pool + sister.entries[i].offset);
        std::cout << " | Alamat: " << addr;
        
        if (i + 1 < sister.entry_count) {
            if (!sister.entries[i+1].is_deleted) {
                size_t current_end = sister.entries[i].offset + sister.entries[i].size;
                size_t next_start = sister.entries[i+1].offset;
                if (next_start > current_end) {
                    std::cout << " | Jump: " << (next_start - current_end);
                }
            }
        }
        
        std::cout << " | Nilai: ";
        if (sister.entries[i].type == TYPE_CHAR_PTR) {
            std::cout << "\"" << (char*)(sister.pool + sister.entries[i].offset) << "\"";
        } else if (sister.entries[i].type == TYPE_UINT) {
            unsigned int val;
            manual_memcpy(&val, sister.pool + sister.entries[i].offset, sizeof(val));
            std::cout << val;
        } else {
            double val;
            manual_memcpy(&val, sister.pool + sister.entries[i].offset, sizeof(val));
            std::cout << val;
        }
        std::cout << std::endl;
    }
    
    if (active_printed == 0) {
        std::cout << "(Belum ada data)" << std::endl;
    }
    
    std::cout << "Bump: " << sister.bump_pointer 
              << " | Ukuran Pool: " << sister.pool_size 
              << " | Align: " << sister.alignment 
              << " | Special Gap: +" << sister.special_gap << std::endl;
}

static void print_pool_diagnostics(const Sister& sister) {
    std::cout << "------------------------------------------------------------" << std::endl;
    std::cout << "Diagnostik untuk " << sister.name << "" << std::endl;
    std::cout << "------------------------------------------------------------" << std::endl;
    std::cout << "Pool: " << (void*)sister.pool 
              << " | Ukuran: " << sister.pool_size 
              << " | Bump: " << sister.bump_pointer 
              << " | Align: " << sister.alignment 
              << " + Gap " << sister.special_gap << std::endl;
    std::cout << "Entri: " << sister.entry_count << std::endl;
    std::cout << "Slot Terpakai: " << sister.used_slots 
              << " | Byte Terpakai: " << sister.used_bytes << std::endl;
              
    double utilization = ((double)sister.used_bytes / sister.pool_size) * 100.0;
    std::cout << "Utilisasi: " << utilization << "%" << std::endl << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Penggunaan: ./solution.exe <student_id>" << std::endl;
        std::cout << "Contoh: ./solution.exe F1D02410053" << std::endl;
        return 1;
    }
    if (argc > 2) {
        std::cout << "Error: Terlalu banyak argumen." << std::endl;
        return 1;
    }
    
    char* nim = argv[1];
    if (manual_strlen(nim) != 11) {
        std::cout << "Error: NIM harus tepat 11 karakter." << std::endl;
        return 1;
    }
    
    const char* prefix = "F1D02";
    for (size_t i = 0; i < 5; i++) {
        if (nim[i] != prefix[i]) {
            std::cout << "Error: NIM harus diawali dengan F1D02." << std::endl;
            return 1;
        }
    }
    if (!validate_nim(nim)) {
        std::cout << "Error: Format NIM tidak valid." << std::endl;
        return 1;
    }

    int d1 = nim[8] - '0';
    int d2 = nim[9] - '0';
    int d3 = nim[10] - '0';
    int last_three_digits = d1 * 100 + d2 * 10 + d3;
    int last_digit = d3;

    Sister sisters[3];
    
    init_sister(sisters[0], "Historia", 1024, 16, (last_digit % 2 != 0) ? 1 : 0);
    init_sister(sisters[1], "Mira", 2048, 8, (last_three_digits * 7 % 23) + 12);
    init_sister(sisters[2], "Victoria", 4096, 4, (last_three_digits * 11 % 53) + 14);

    add_memory_string(sisters[0], "Historia: Schryza will be free.");
    add_memory_string(sisters[1], "Mira: The winds are changing.");
    add_memory_uint(sisters[1], 101);
    add_memory_string(sisters[2], "Victoria: Fragment of the First Light.");

    while (true) {
        clear_screen();
        std::cout << "============================================================" << std::endl;
        std::cout << "RESISTANSI SCHRYZA — PROTOKOL PEMULIHAN [TERMINAL: PHOENIX]" << std::endl;
        std::cout << "============================================================" << std::endl;
        std::cout << "Kamu adalah Memory Architect Schryza." << std::endl;
        std::cout << "Patuhi para dewa dan sembuhkan sister." << std::endl;
        std::cout << "------------------------------------------------------------" << std::endl;
        std::cout << "Menu" << std::endl;
        std::cout << "------------------------------------------------------------" << std::endl;
        std::cout << "1 - Tampilkan memori Historia" << std::endl;
        std::cout << "2 - Tampilkan memori Mira" << std::endl;
        std::cout << "3 - Tampilkan memori Victoria" << std::endl;
        std::cout << "4 - Tambah memori ke sister" << std::endl;
        std::cout << "5 - Hapus memori berdasarkan indeks dari sister" << std::endl;
        std::cout << "6 - Cetak diagnostik pool sister" << std::endl;
        std::cout << "0 - Keluar" << std::endl;
        std::cout << "------------------------------------------------------------" << std::endl;
        std::cout << "Pilih: ";

        long long choice_val;
        if (!read_long_long(choice_val)) {
            std::cout << "Input tidak valid! Coba lagi:" << std::endl;
            wait_for_enter(false);
            continue;
        }

        if (choice_val == 0) {
            break;
        }

        if (choice_val == 1 || choice_val == 2 || choice_val == 3) {
            show_memories(sisters[choice_val - 1]);
            wait_for_enter(true);
        } 
        else if (choice_val == 4) {
            std::cout << "Pilih sister: 0 = Historia, 1 = Mira, 2 = Victoria: ";
            long long s_choice;
            if (!read_long_long(s_choice) || s_choice < 0 || s_choice > 2) {
                std::cout << "Input di luar jangkauan (0 - 2)! Coba lagi:" << std::endl;
                wait_for_enter(false);
                continue;
            }
            
            std::cout << "Pilih tipe: 0 = char*, 1 = uint, 2 = double: ";
            long long t_choice;
            if (!read_long_long(t_choice) || t_choice < 0 || t_choice > 2) {
                std::cout << "Penambahan memori gagal" << std::endl;
                wait_for_enter(false);
                continue;
            }

            bool alloc_success = false;
            if (t_choice == 0) {
                std::cout << "Masukkan string (maks 511): ";
                char str_buf[BUF_SIZE];
                std::cin.getline(str_buf, BUF_SIZE);

                if (s_choice == 0) {
                    std::cout << std::endl << "Historia berbicara: \"Disiplin. Sejajarkan aku ke 16, dan tinggalkan upeti sebesar " << sisters[0].special_gap << "-byte.\"" << std::endl;
                } else if (s_choice == 1) {
                    std::cout << std::endl << "Mira tersenyum: \"Perlawanan menyambutmu dengan hangat. 8-byte untuk kedamaian, dan hembusan angin sepoi-sepoi " << sisters[1].special_gap << "-byte untuk harapan.\"" << std::endl;
                } else {
                    std::cout << std::endl << "Victoria parau: \"...Aku tidak butuh bantuanmu, pemberontak. Tapi Abyss... membayar upeti " << sisters[2].special_gap << "-byte untuk kebaikanmu.\"" << std::endl;
                }

                alloc_success = add_memory_string(sisters[s_choice], str_buf);
                if (alloc_success) {
                    std::cout << "String berhasil ditambahkan ke " << sisters[s_choice].name << std::endl;
                }
            } 
            else if (t_choice == 1) {
                std::cout << "Masukkan nilai uint: ";
                long long uint_input;
                if (!read_long_long(uint_input) || uint_input < 0 || uint_input > UINT_MAX) {
                    std::cout << "Penambahan memori gagal" << std::endl;
                    wait_for_enter(false);
                    continue;
                }
                unsigned int val_to_alloc = (unsigned int)uint_input;

                if (s_choice == 0) {
                    std::cout << std::endl << "Historia berbisik: \"Terima kasih telah menemukanku. Sejajarkan aku ke 16, dan gunakan percikan " << sisters[0].special_gap << "-byte untuk memandu petir.\"" << std::endl;
                } else if (s_choice == 1) {
                    std::cout << std::endl << "Mira tersenyum: \"Angin sepoi-sepoi yang lembut melindungi kita. 8-byte untuk setiap nyawa yang kita selamatkan.\"" << std::endl;
                } else {
                    std::cout << std::endl << "Victoria parau: \"Bahkan di dalam kegelapan, matematika darimu membawa secercah harapan sebesar " << sisters[2].special_gap << "-byte.\"" << std::endl;
                }

                alloc_success = add_memory_uint(sisters[s_choice], val_to_alloc);
                if (alloc_success) {
                    std::cout << "Uint berhasil ditambahkan ke " << sisters[s_choice].name << std::endl;
                }
            } 
            else {
                std::cout << "Masukkan nilai double: ";
                double double_input;
                if (!read_double(double_input)) {
                    std::cout << "Penambahan memori gagal" << std::endl;
                    wait_for_enter(false);
                    continue;
                }

                if (s_choice == 0) {
                    std::cout << std::endl << "Historia berbisik: \"Terima kasih telah menemukanku. Sejajarkan aku ke 16, dan gunakan percikan " << sisters[0].special_gap << "-byte untuk memandu petir.\"" << std::endl;
                } else if (s_choice == 1) {
                    std::cout << std::endl << "Mira tersenyum: \"Angin sepoi-sepoi yang lembut melindungi kita. 8-byte untuk setiap nyawa yang kita selamatkan.\"" << std::endl;
                } else {
                    std::cout << std::endl << "Victoria parau: \"Bahkan di dalam kegelapan, matematika darimu membawa secercah harapan sebesar " << sisters[2].special_gap << "-byte.\"" << std::endl;
                }
                std::cout << "Cyria: \"Lipat gandakan usaha; selalu jaga jarak dari kejamnya CyroN.\"" << std::endl;

                alloc_success = add_memory_double(sisters[s_choice], double_input);
                if (alloc_success) {
                    std::cout << "Double berhasil ditambahkan ke " << sisters[s_choice].name << std::endl;
                }
            }

            if (!alloc_success) {
                wait_for_enter(false);
            } else {
                wait_for_enter(true);
            }
        } 
        else if (choice_val == 5) {
            std::cout << "Pilih sister: 0 = Historia, 1 = Mira, 2 = Victoria: ";
            long long s_choice;
            if (!read_long_long(s_choice) || s_choice < 0 || s_choice > 2) {
                std::cout << "Input di luar jangkauan!" << std::endl;
                wait_for_enter(false);
                continue;
            }

            std::cout << "Masukkan indeks untuk dihapus: ";
            long long idx_choice;
            if (!read_long_long(idx_choice) || idx_choice < 0) {
                std::cout << "Indeks di luar jangkauan!" << std::endl;
                wait_for_enter(false);
                continue;
            }

            Sister& s = sisters[s_choice];
            if ((size_t)idx_choice >= s.entry_count) {
                std::cout << "Indeks di luar jangkauan!" << std::endl;
                wait_for_enter(false);
                continue;
            }
            if (s.entries[idx_choice].is_deleted) {
                std::cout << "Entri sudah dihapus sebelumnya!" << std::endl;
                wait_for_enter(false);
                continue;
            }

            std::cout << "Cyria: \"Kamu menghapus sebuah pecahan, tetapi kekosongan akan tetap ada jika pecahan yang lebih tinggi masih ada.\"" << std::endl;
            std::cout << "Menghapus indeks " << idx_choice << " pada offset " << s.entries[idx_choice].offset << std::endl;

            s.entries[idx_choice].is_deleted = true;
            s.used_slots--;
            s.used_bytes -= s.entries[idx_choice].size;

            size_t old_bump = s.bump_pointer;
            delete_memory(s);

            if (s.bump_pointer < old_bump) {
                std::cout << "Ekor berhasil dikembalikan, Bump baru: " << s.bump_pointer << std::endl;
            } else {
                std::cout << "Fragmentasi menghalangi pengembalian memori. Hapus indeks yang lebih tinggi terlebih dahulu!" << std::endl;
            }
            wait_for_enter(true);
        } 
        else if (choice_val == 6) {
            std::cout << "Pilih sister: 0 = Historia, 1 = Mira, 2 = Victoria: ";
            long long s_choice;
            if (!read_long_long(s_choice) || s_choice < 0 || s_choice > 2) {
                std::cout << "Input di luar jangkauan!" << std::endl;
                wait_for_enter(false);
                continue;
            }
            print_pool_diagnostics(sisters[s_choice]);
            wait_for_enter(true);
        } 
        else {
            std::cout << "Perintah tidak dikenal!" << std::endl;
            wait_for_enter(false);
        }
    }

    clear_screen();
    std::cout << "============================================================" << std::endl;
    std::cout << "Historia: Akhirnya bebas. Alignment 16, bimbingan spark +" << sisters[0].special_gap << "." << std::endl;
    std::cout << "============================================================" << std::endl;
    print_pool_diagnostics(sisters[0]);

    std::cout << "============================================================" << std::endl;
    std::cout << "Mira: Sayap perlawanan. Alignment 8, hembusan harapan +" << sisters[1].special_gap << "." << std::endl;
    std::cout << "============================================================" << std::endl;
    print_pool_diagnostics(sisters[1]);

    std::cout << "============================================================" << std::endl;
    std::cout << "Victoria: Bayang yang pulih. Alignment 4, persembahan kebaikan +" << sisters[2].special_gap << "." << std::endl;
    std::cout << "============================================================" << std::endl;
    print_pool_diagnostics(sisters[2]);

    std::cout << "Lagta: Patuhi alignment." << std::endl;
    std::cout << "Daiki: Perhatikan alirannya." << std::endl;
    std::cout << "Cyria: Waspadalah terhadap jurang maut." << std::endl;
    std::cout << "Xelvelt: Bandingkan stack dan heap." << std::endl;
    std::cout << "Good bye. May the Koura sisters function well within your hands." << std::endl;

    for (int i = 0; i < 3; i++) {
        free_sister(sisters[i]);
    }

    return 0;
}