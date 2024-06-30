#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <limits>
#include <algorithm>

using namespace std;

// Struct untuk representasi user
struct User {
    string username;
    string password;
    string role;
};

// Struct untuk representasi Angkutan Umum (dianggap sebagai Edge)
struct AngkutanUmum {
    string nama;
    string kotaAsal; // Kota asal angkutan umum
    string kotaTujuan; // Kota tujuan angkutan umum
    string jamMulaiOperasional; // Jam mulai operasional
    string jamTutupOperasional; // Jam tutup operasional
    int waktuTempuh; // Waktu tempuh dalam menit
};

// Struct untuk representasi Kota (dianggap sebagai Node)
struct Kota {
    string nama; // Nama kota
    vector<AngkutanUmum> angkutanUmum; // Daftar angkutan umum yang tersedia di kota ini
};

// Struct untuk representasi Graf
struct Graph {
    map<string, vector<pair<string, AngkutanUmum>>> adjList; // key: lokasi, value: pasangan (lokasi tujuan, angkutan umum)
};

// Fungsi untuk menampilkan graf antar kota (adjacency list) yang hanya menampilkan kota terdaftar
void tampilkanGraf(const Graph &graph, const string &namaFileKota) {
    cout << "\nGraf Antar Kota (Adjacency List):\n";
    ifstream fileKota(namaFileKota);
    string kota;
    while (fileKota >> kota) {
        if (graph.adjList.find(kota) != graph.adjList.end()) {
            cout << "Kota " << kota << ": ";
            for (const auto &neighbor : graph.adjList.at(kota)) {
                cout << "(" << neighbor.first << ", " << neighbor.second.nama << ", " << neighbor.second.waktuTempuh << ") ";
            }
            cout << endl;
        }
    }
    fileKota.close();
    cout << endl;
}

// Fungsi untuk mengecek ketersediaan angkutan umum berdasarkan waktu
bool isAngkutanUmumAvailable(const AngkutanUmum &au, const string &waktu) {
    return au.jamMulaiOperasional <= waktu && waktu <= au.jamTutupOperasional;
}

// Fungsi untuk memuat data angkutan umum dari file
void muatDataAngkutanUmum(vector<AngkutanUmum> &angkutanUmum, const string &namaFile) {
    ifstream file(namaFile);
    string line;

    while (getline(file, line)) {
        stringstream ss(line);
        AngkutanUmum angkutan;
        ss >> angkutan.nama >> angkutan.kotaAsal >> angkutan.kotaTujuan >> angkutan.jamMulaiOperasional >> angkutan.jamTutupOperasional >> angkutan.waktuTempuh;
        angkutanUmum.push_back(angkutan);
    }

    file.close();
}

// Fungsi untuk menambahkan angkutan umum ke graf
void tambahAngkutanUmumKeGraf(Graph &graph, const AngkutanUmum &au) {
    // Tambahkan kota tujuan ke graf jika belum ada
    if (graph.adjList.find(au.kotaTujuan) == graph.adjList.end()) {
        graph.adjList[au.kotaTujuan] = {};
    }
    // Tambahkan angkutan umum ke kota asal
    graph.adjList[au.kotaAsal].push_back({au.kotaTujuan, au});
}

// Fungsi untuk mencari rute tercepat berdasarkan angkutan umum yang tersedia dan waktu tempuh tercepat
vector<string> cariRuteTercepat(const Graph &graph, const string &asal, const string &tujuan, const string &waktu) {
    if (graph.adjList.find(asal) == graph.adjList.end() || graph.adjList.find(tujuan) == graph.adjList.end()) {
        return {}; // Kota asal atau tujuan tidak ada di dalam graf
    }

    map<string, int> dist;
    map<string, string> prev;
    set<string> visited;
    priority_queue<pair<int, string>, vector<pair<int, string>>, greater<pair<int, string>>> pq;

    // Inisialisasi jarak semua lokasi dengan nilai tak hingga
    for (const auto &loc : graph.adjList) {
        dist[loc.first] = numeric_limits<int>::max();
    }

    // Jarak dari kota asal ke dirinya sendiri adalah 0
    dist[asal] = 0;
    pq.push({0, asal});

    // Proses algoritma Dijkstra
    while (!pq.empty()) {
        string current = pq.top().second;
        pq.pop();

        // Skip jika kota sudah dikunjungi sebelumnya
        if (visited.find(current) != visited.end()) continue;
        visited.insert(current);

        // Iterasi semua tetangga dari kota saat ini
        for (const auto &neighbor : graph.adjList.at(current)) {
            string next = neighbor.first;
            const AngkutanUmum &au = neighbor.second;

            // Periksa ketersediaan angkutan umum berdasarkan waktu
            if (!isAngkutanUmumAvailable(au, waktu)) continue;

            int weight = au.waktuTempuh;

            // Update jarak jika menemukan jarak yang lebih pendek
            if (dist[current] + weight < dist[next]) {
                dist[next] = dist[current] + weight;
                prev[next] = current;
                pq.push({dist[next], next});
            }
        }
    }

    // Rekonstruksi rute tercepat dari kota asal ke kota tujuan
    vector<string> path;
    for (string at = tujuan; !at.empty(); at = prev[at]) {
        path.push_back(at);
    }
    reverse(path.begin(), path.end());

    if (path.size() == 1 && path[0] == asal) {
        return {}; // Tidak ada rute yang ditemukan
    }

    return path;
}


// Fungsi untuk mencari rute alternatif (rute tercepat kedua)
vector<string> cariRuteAlternatif(const Graph &graph, const string &asal, const string &tujuan, const string &waktu) {
    // Mencari rute tercepat pertama
    vector<string> ruteTercepat = cariRuteTercepat(graph, asal, tujuan, waktu);

    // Menghapus rute tercepat pertama dari graf
    Graph graphWithoutFastestRoute = graph;
    string prevNode = asal;
    for (const auto &node : ruteTercepat) {
        if (node != asal) {
            auto &neighbors = graphWithoutFastestRoute.adjList[prevNode];
            neighbors.erase(remove_if(neighbors.begin(), neighbors.end(), [&](const pair<string, AngkutanUmum> &neighbor) {
                return neighbor.first == node;
            }), neighbors.end());
        }
        prevNode = node;
    }

    // Mencari rute tercepat kedua
    return cariRuteTercepat(graphWithoutFastestRoute, asal, tujuan, waktu);
}

// Fungsi untuk melihat jadwal angkutan umum dengan nomor
void lihatJadwalAngkutanUmum(const vector<AngkutanUmum> &angkutanUmum) {
    if (angkutanUmum.empty()) {
        cout << "Jadwal Angkutan Umum Tidak Ditemukan!\n";
        return;
    }

    cout << "Jadwal Angkutan Umum:\n";
    cout << "---------------------------------------------\n";
    int index = 1;
    for (const auto &au : angkutanUmum) {
        cout << index++ << ". " << au.nama << " dari " << au.kotaAsal << " ke " << au.kotaTujuan
             << "\n --> Jam Operasi: " << au.jamMulaiOperasional << " - " << au.jamTutupOperasional
             << ".\n --> Waktu tempuh: " << au.waktuTempuh << " menit.\n";
        cout << "---------------------------------------------\n";
    }
}

// Fungsi untuk membaca daftar kota dari file dan memasukkannya ke dalam graf
void bacaDaftarKota(Graph &graph, const string &namaFileKota) {
    ifstream file(namaFileKota);
    string namaKota;

    while (getline(file, namaKota)) {
        // Masukkan kota ke dalam graf dengan daftar tetangga kosong
        graph.adjList[namaKota] = vector<pair<string, AngkutanUmum>>();
    }

    file.close();
}

// Fungsi untuk membaca data angkutan umum dari file
vector<AngkutanUmum> bacaDataAngkutanUmum(const string& namaFile) {
    vector<AngkutanUmum> angkutanUmum;
    ifstream file(namaFile);
    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        AngkutanUmum angkutan;
        ss >> angkutan.nama >> angkutan.kotaAsal >> angkutan.kotaTujuan >> angkutan.jamMulaiOperasional >> angkutan.jamTutupOperasional >> angkutan.waktuTempuh;
        angkutanUmum.push_back(angkutan);
    }
    return angkutanUmum;
}

// Fungsi untuk menulis data ke file eksternal
void tulisDataAngkutanUmum(const vector<AngkutanUmum> &data, const string &namaFile) {
    ofstream file(namaFile);
    if (!file.is_open()) {
        cerr << "Gagal membuka file " << namaFile << " untuk penulisan." << endl;
        return;
    }

    for (const auto &au : data) {
        file << au.nama << " " << au.kotaAsal << " " << au.kotaTujuan << " " << au.jamMulaiOperasional << " " << au.jamTutupOperasional << " " << au.waktuTempuh << endl;
    }

    file.close();
}

void tambahKota(Graph &graph, const string &kota, vector<AngkutanUmum> &angkutanUmum, const string &namaFileKota) {

    // Memeriksa apakah kota sudah ada dalam graf
    if (graph.adjList.find(kota) != graph.adjList.end()) {
        cout << "\nKota sudah ada dalam graf!" << endl;
        return;
    }

    // Menambahkan kota baru ke graf
    graph.adjList[kota] = vector<pair<string, AngkutanUmum>>();

    // Membaca file kota dan memperbarui data
    ofstream file(namaFileKota, ios::app);
    if (file.is_open()) {
        file << kota << endl;
        file.close();
        cout << "\nKota " << kota << " berhasil ditambahkan dan disimpan ke file." << endl;
    } else {
        cout << "\nGagal membuka file kota!" << endl;
    }
}



// Fungsi untuk menghapus kota dari daftar
void hapusKota(Graph &graph, vector<AngkutanUmum> &angkutanUmum, const string &namaFileKota, const string &namaFileAngkutanUmum) {
    cout << "Daftar Kota yang Tersedia:" << endl;

    // Menampilkan daftar kota beserta nomornya
    ifstream file(namaFileKota);
    string namaKota;
    int nomor = 1;

    while (getline(file, namaKota)) {
        cout << nomor << ". " << namaKota << endl;
        nomor++;
    }

    file.close();

    // Meminta input nomor kota yang ingin dihapus
    int nomorHapus;
    cout << "0.  Kembali ke menu\n";
    cout << "Masukkan nomor kota yang ingin dihapus: ";
    cin >> nomorHapus;

    if (nomorHapus == 0) {
        return; // Kembali ke menu
    }

    // Menghapus kota dari daftar kota
    ifstream inFile(namaFileKota);
    ofstream outFile(namaFileKota + ".tmp");
    string line;
    int lineCount = 1;

    while (getline(inFile, line)) {
        if (lineCount != nomorHapus) {
            outFile << line << endl;
        }
        lineCount++;
    }

    inFile.close();
    outFile.close();

    // Menghapus file asli dan mengganti namanya dengan file sementara
    remove(namaFileKota.c_str());
    rename((namaFileKota + ".tmp").c_str(), namaFileKota.c_str());

    cout << "Kota berhasil dihapus dari daftar." << endl;
}

// Fungsi untuk menambah jadwal angkutan umum
void tambahJadwalAngkutanUmum(vector<AngkutanUmum> &angkutanUmum, Graph &graph, const string &namaFile, const string &namaFileKota) {
    AngkutanUmum au;
    cout << "Masukkan nama angkutan umum: ";
    cin >> au.nama;
    cout << "Masukkan kota asal: ";
    cin >> au.kotaAsal;
    cout << "Masukkan kota tujuan: ";
    cin >> au.kotaTujuan;
    cout << "Masukkan jam mulai operasional (HH:MM): ";
    cin >> au.jamMulaiOperasional;
    cout << "Masukkan jam tutup operasional (HH:MM): ";
    cin >> au.jamTutupOperasional;
    cout << "Masukkan waktu tempuh (dalam menit): ";
    cin >> au.waktuTempuh;

    // Tambahkan kota asal dan kota tujuan ke dalam file kota terdaftar jika belum terdaftar
    tambahKota(graph, au.kotaAsal, angkutanUmum, namaFileKota);
    tambahKota(graph, au.kotaTujuan, angkutanUmum, namaFileKota);

    tambahAngkutanUmumKeGraf(graph, au);

    angkutanUmum.push_back(au);
    cout << "\nJadwal angkutan umum berhasil ditambahkan.\n";
    cout << endl;

    // Setelah menambah jadwal angkutan umum, tulis data ke file
    tulisDataAngkutanUmum(angkutanUmum, namaFile);
}


// Fungsi untuk menghapus jadwal angkutan umum
void hapusJadwalAngkutanUmum(vector<AngkutanUmum> &angkutanUmum, Graph &graph, const string &namaFile) {
    if (angkutanUmum.empty()) {
        cout << "Tidak ada jadwal angkutan umum yang tersedia untuk dihapus.\n";
        return;
    }

    lihatJadwalAngkutanUmum(angkutanUmum);
    int nomor;
    cout << "Masukkan nomor jadwal angkutan umum yang ingin dihapus: ";
    cin >> nomor;

    if (nomor < 1 || nomor > angkutanUmum.size()) {
        cout << "Nomor tidak valid.\n";
        return;
    }

    // Mendapatkan angkutan umum yang akan dihapus
    AngkutanUmum au = angkutanUmum[nomor - 1];

    // Hapus dari vector angkutan umum
    angkutanUmum.erase(angkutanUmum.begin() + (nomor - 1));

    // Hapus dari graf
    graph.adjList[au.kotaAsal].erase(remove_if(graph.adjList[au.kotaAsal].begin(), graph.adjList[au.kotaAsal].end(),
        [&au](const pair<string, AngkutanUmum> &p) {
            return p.second.nama == au.nama && p.second.kotaAsal == au.kotaAsal && p.second.kotaTujuan == au.kotaTujuan
                   && p.second.jamMulaiOperasional == au.jamMulaiOperasional && p.second.jamTutupOperasional == au.jamTutupOperasional
                   && p.second.waktuTempuh == au.waktuTempuh;
        }), graph.adjList[au.kotaAsal].end());

    cout << "Jadwal angkutan umum berhasil dihapus.\n";

    // Setelah menghapus jadwal angkutan umum, tulis data ke file
    tulisDataAngkutanUmum(angkutanUmum, namaFile);
}

// Fungsi untuk menampilkan rute dan rekomendasi angkutan umum yang tersedia
void tampilkanRuteDanRekomendasi(const vector<string> &rute, const vector<AngkutanUmum> &angkutanUmum, const string &waktu) {
    if (rute.size() < 2) {
        cout << "Tidak ada rute yang tersedia.\n";
        cout << endl;
        return;
    }

    int total_waktu = 0;
    bool adaAngkutan = false; // Flag untuk mengecek ada tidaknya angkutan umum

    for (size_t i = 0; i < rute.size() - 1; ++i) {
        bool found = false;
        for (const auto &au : angkutanUmum) {
            if (au.kotaAsal == rute[i] && au.kotaTujuan == rute[i + 1] && isAngkutanUmumAvailable(au, waktu)) {
                cout << "Gunakan [" << au.nama << "] dari [" << rute[i] << "] ke [" << rute[i + 1] << "] (" << au.jamMulaiOperasional << " - " << au.jamTutupOperasional << ")\n--> waktu tempuh: " << au.waktuTempuh <<" menit \n";
                found = true;
                adaAngkutan = true; // Menandakan ada angkutan umum yang ditemukan
                total_waktu += au.waktuTempuh;
                break;
            }
        }

        if (!found) {
            cout << "Tidak ada angkutan umum tersedia dari " << rute[i] << " ke " << rute[i + 1] << " pada pukul " << waktu << ".\n";
        }
    }

    if (!adaAngkutan) {
        cout << "Tidak ada angkutan umum yang beroperasi pada rute ini.\n";
    } else {
        cout << "Estimasi waktu perjalanan: " << total_waktu << " menit\n";
    }
    
    cout << endl;
}


// Fungsi untuk mencari jadwal angkutan umum
void cariJadwalAngkutanUmum(const Graph &graph) {
    // Mengumpulkan nama angkutan umum unik
    set<string> namaAngkutanSet;
    for (const auto &entry : graph.adjList) {
        for (const auto &pair : entry.second) {
            namaAngkutanSet.insert(pair.second.nama);
        }
    }

    // Menampilkan daftar nama angkutan umum
    cout << "Daftar Angkutan Umum yang Tersedia:" << endl;
    vector<string> namaAngkutanList(namaAngkutanSet.begin(), namaAngkutanSet.end());
    for (size_t i = 0; i < namaAngkutanList.size(); ++i) {
        cout << i + 1 << ". " << namaAngkutanList[i] << endl;
    }

    // Meminta input nomor angkutan umum dari pengguna
    int nomorAngkutan;
    cout << "Masukkan nomor angkutan umum: ";
    cin >> nomorAngkutan;

    if (nomorAngkutan < 1 || nomorAngkutan > namaAngkutanList.size()) {
        cout << "Nomor angkutan umum tidak valid!" << endl;
        return;
    }

    string namaAngkutan = namaAngkutanList[nomorAngkutan - 1];

    // Menampilkan jadwal angkutan umum yang dipilih
    cout << "Jadwal untuk " << namaAngkutan << ":";
    cout << "\n---------------------------------------------\n";
    for (const auto &entry : graph.adjList) {
        for (const auto &pair : entry.second) {
            if (pair.second.nama == namaAngkutan) {
                cout << "Kota Asal: " << pair.second.kotaAsal
                     << "\nKota Tujuan: " << pair.second.kotaTujuan
                     << "\nJam Mulai: " << pair.second.jamMulaiOperasional
                     << "\nJam Tutup: " << pair.second.jamTutupOperasional
                     << "\nWaktu Tempuh: " << pair.second.waktuTempuh << " menit";
                cout << "\n---------------------------------------------\n";
            }
        }
    }
}


// Fungsi untuk menampilkan daftar kota dengan penomoran
void lihatDaftarKota(Graph &graph, const string &namaFileKota) {
    cout << "Daftar Kota:" << endl;
    ifstream file(namaFileKota);
    string namaKota;
    int nomor = 1;

    // Membaca setiap baris dalam file dan menampilkan nama kota dengan penomoran
    while (getline(file, namaKota)) {
        cout << nomor << ". " << namaKota << endl;
        nomor++;
    }

    file.close();
}

// Fungsi untuk mendapatkan nama kota dari nomor urutan
string namaKotaDariNomor(Graph &graph, int nomor, const string &namaFileKota) {
    ifstream file(namaFileKota);
    string namaKota;
    int currentNumber = 1;

    while (getline(file, namaKota)) {
        if (currentNumber == nomor) {
            return namaKota;
        }
        currentNumber++;
    }

    file.close();
    return ""; // Return empty string if the number is not valid
}

// Fungsi untuk menampilkan menu login dan register
void tampilkanMenuLoginRegister() {
    cout << "-----SISTEM REKOMENDASI ANGKUTAN UMUM-----\n";
    cout << "1. Login\n";
    cout << "2. Register\n";
    cout << "0. Keluar\n";
}

// Fungsi untuk menampilkan menu utama
void tampilkanMenuUtamaAdmin() {
    cout << "\n-----SISTEM REKOMENDASI ANGKUTAN UMUM-----\n";
    cout << "1.  Cari rute tercepat dan rekomendasi angkutan umum\n";
    cout << "2.  Lihat jadwal angkutan umum\n";
    cout << "3.  Tambah jadwal angkutan umum\n";
    cout << "4.  Hapus jadwal angkutan umum\n";
    cout << "5.  Tambah kota\n";
    cout << "6.  Hapus kota\n";
    cout << "7.  Tampilkan graf antar kota\n";
    cout << "8.  Cari jadwal angkutan umum berdasarkan nama\n";
    cout << "9.  Lihat daftar kota\n";
    cout << "99. Logout\n";
    cout << "0.  Keluar\n";
    cout << "Pilih menu: ";
}

// Fungsi untuk menampilkan menu utama user
void tampilkanMenuUtamaUser() {
    cout << "\n-----SISTEM REKOMENDASI ANGKUTAN UMUM-----\n";
    cout << "1.  Cari rute tercepat dan rekomendasi angkutan umum\n";
    cout << "2.  Lihat jadwal angkutan umum\n";
    cout << "3.  Cari jadwal angkutan umum berdasarkan nama\n";
    cout << "4.  Tampilkan graf antar kota\n";
    cout << "5.  Lihat daftar kota\n";
    cout << "99. Logout\n";
    cout << "0.  Keluar\n";
    cout << "Pilih menu: ";
}

// Fungsi untuk menambahkan data user baru ke dalam file data_user.txt
void tambahDataUser(const User &user) {
    ofstream file("data_user.txt", ios::app);
    if (!file.is_open()) {
        cerr << "Gagal membuka file data_user.txt untuk penulisan." << endl;
        return;
    }

    file << user.username << " " << user.password << " " << user.role << endl;

    file.close();
}

// Fungsi untuk melakukan login
bool loginUser(const vector<User> &users, string &role) {
    string username, password;
    cout << "Username: ";
    cin >> username;
    cout << "Password: ";
    cin >> password;

    for (const auto &user : users) {
        if (user.username == username && user.password == password) {
            role = user.role;
            cout << "\nBerhasil Login. . . \n\nSelamat Datang!\n\n";
            return true;
        }
    }

    cout << "Username atau password salah." << endl;
    return false;
}

// Fungsi untuk melakukan registrasi
void registerUser(vector<User> &users) {
    string username, password, role;
    cout << "Username: ";
    cin >> username;

    // Periksa apakah username sudah terdaftar
    for (const auto &user : users) {
        if (user.username == username) {
            cout << "Username sudah terdaftar. Silakan pilih username lain." << endl;
            return;
        }
    }

    cout << "Password: ";
    cin >> password;
    cout << "Role (user/admin): ";
    cin >> role;

    User newUser;
    newUser.username = username;
    newUser.password = password;
    newUser.role = role;

    users.push_back(newUser);

    // Tambahkan data user ke file
    tambahDataUser(newUser);

    cout << "Registrasi berhasil." << endl;
}

// Fungsi untuk membaca data user dari file eksternal
vector<User> bacaDataUser() {
    vector<User> users;
    ifstream file("data_user.txt");
    if (!file.is_open()) {
        cerr << "Gagal membuka file data_user.txt." << endl;
        return users;
    }

    User user;
    while (file >> user.username >> user.password >> user.role) {
        users.push_back(user);
    }

    file.close();
    return users;
}

// Fungsi untuk logout
void logout(string &role) {
    // Mengembalikan peran ke string kosong
    role = "";
}

// Main function
int main() {
    vector<User> users = bacaDataUser();
    string role;

    do {
        tampilkanMenuLoginRegister();

        int pilihan;
        cout << "Pilihan: ";
        cin >> pilihan;

        switch (pilihan) {
            case 1: {
                if (users.empty()) {
                    cout << "Tidak ada pengguna terdaftar. Silakan register terlebih dahulu." << endl;
                    break;
                }
                if (loginUser(users, role)) {

                    Graph graph;
                    vector<AngkutanUmum> angkutanUmum;

                    // Baca data dari file eksternal saat memulai program
                    const string namaFile = "data_angkutan_umum.txt";
                    const string namaFileKota = "data_kota_terdaftar.txt";
                    angkutanUmum = bacaDataAngkutanUmum(namaFile);

                    // Masukkan daftar kota ke dalam graf
                    bacaDaftarKota(graph, namaFileKota);

                    // Memasukkan data angkutan umum ke dalam graf
                    for (const auto &au : angkutanUmum) {
                        tambahAngkutanUmumKeGraf(graph, au);
                    }

                    // Masuk ke menu utama
                    if (role == "user") {
                        // Tampilkan menu utama user
                        while (true) {
                            tampilkanMenuUtamaUser();
                            int pilihan;
                            cin >> pilihan;

                            // Handling untuk setiap pilihan menu
                            switch (pilihan) {
                                case 1: {
                                    // Cari rute tercepat dan rekomendasi angkutan umum
                                    cout << "\n-------Mau pergi ke mana?-------\n";
                                    lihatDaftarKota(graph, namaFileKota);

                                    int nomorAsal, nomorTujuan;
                                    string asal, tujuan, waktu;

                                    cout << "Pilih nomor kota asal: ";
                                    cin >> nomorAsal;
                                    asal = namaKotaDariNomor(graph, nomorAsal, namaFileKota);

                                    cout << "Pilih nomor kota tujuan: ";
                                    cin >> nomorTujuan;
                                    tujuan = namaKotaDariNomor(graph, nomorTujuan, namaFileKota);

                                    cout << "Pukul (HH:MM): ";
                                    cin >> waktu;
                                    cout << endl;

                                    vector<string> ruteTercepat = cariRuteTercepat(graph, asal, tujuan, waktu);
                                    if (ruteTercepat.empty()) {
                                        cout << "Tidak ada angkutan umum yang beroperasi atau rute tidak ditemukan!\n";
                                    } else {
                                        vector<string> ruteAlternatif = cariRuteAlternatif(graph, asal, tujuan, waktu);

                                        cout << "Rute tercepat:\n";
                                        tampilkanRuteDanRekomendasi(ruteTercepat, angkutanUmum, waktu);

                                        cout << "\nRute Alternatif:\n";
                                        tampilkanRuteDanRekomendasi(ruteAlternatif, angkutanUmum, waktu);
                                    }
                                    break;
                                }
                                case 2: {
                                    // Lihat jadwal angkutan umum
                                    lihatJadwalAngkutanUmum(angkutanUmum);
                                    break;
                                }
                                case 3: {
                                    // Cari jadwal angkutan umum berdasarkan nama
                                    cariJadwalAngkutanUmum(graph);
                                    break;
                                }
                                case 4: {
                                    // Tampilkan graf antar kota
                                    tampilkanGraf(graph, namaFileKota);
                                    break;
                                }
                                case 5: {
                                    // Lihat daftar kota
                                    lihatDaftarKota(graph, namaFileKota);
                                    break;
                                }
                                case 99: {
                                    // Logout
                                    // Implementasi logout (kembali ke menu sebelumnya)
                                    role = "";
                                    break;
                                }
                                case 0: {
                                    // Keluar
                                    cout << "\nKeluar dari sistem...\nTerima Kasih!\n";
                                    cout << endl;
                                    return 0;
                                }
                                default: {
                                    cout << "\n[!!invalid!!] Menu tidak ada!\n";
                                    cout << endl;
                                }
                            }
                            if (role.empty()) break; // Exit loop if logged out
                        }
                    } else if (role == "admin") {
                        // Tampilkan menu utama admin
                        while (true) {
                            tampilkanMenuUtamaAdmin();
                            int pilihan;
                            cin >> pilihan;

                            // Handling untuk setiap pilihan menu
                            switch (pilihan) {
                                case 1: {
                                    // Cari rute tercepat dan rekomendasi angkutan umum
                                    cout << "\n-------Mau pergi ke mana?-------\n";
                                    lihatDaftarKota(graph, namaFileKota);

                                    int nomorAsal, nomorTujuan;
                                    string asal, tujuan, waktu;

                                    cout << "Pilih nomor kota asal: ";
                                    cin >> nomorAsal;
                                    asal = namaKotaDariNomor(graph, nomorAsal, namaFileKota);

                                    cout << "Pilih nomor kota tujuan: ";
                                    cin >> nomorTujuan;
                                    tujuan = namaKotaDariNomor(graph, nomorTujuan, namaFileKota);

                                    cout << "Pukul (HH:MM): ";
                                    cin >> waktu;
                                    cout << endl;

                                    vector<string> ruteTercepat = cariRuteTercepat(graph, asal, tujuan, waktu);
                                    if (ruteTercepat.empty()) {
                                        cout << "[!INVALID!]: Tidak ada angkutan umum yang beroperasi atau rute tidak ditemukan!\n";
                                    } else {
                                        vector<string> ruteAlternatif = cariRuteAlternatif(graph, asal, tujuan, waktu);

                                        cout << "Rute tercepat:\n";
                                        tampilkanRuteDanRekomendasi(ruteTercepat, angkutanUmum, waktu);

                                        cout << "\nRute Alternatif:\n";
                                        tampilkanRuteDanRekomendasi(ruteAlternatif, angkutanUmum, waktu);
                                    }
                                    break;
                                }

                                case 2: {
                                    // Lihat jadwal angkutan umum
                                    lihatJadwalAngkutanUmum(angkutanUmum);
                                    break;
                                }
                                case 3: {
                                    // Tambah jadwal angkutan umum
                                    tambahJadwalAngkutanUmum(angkutanUmum, graph, namaFile, namaFileKota);
                                    break;
                                }
                                case 4: {
                                    // Hapus jadwal angkutan umum
                                    hapusJadwalAngkutanUmum(angkutanUmum, graph, namaFile);
                                    break;
                                }
                                case 5: {
                                    // Tambah kota
                                    // Menggunakan string untuk menampung input kota
                                    string namaKota;
                                    cout << "Masukkan nama kota yang ingin ditambahkan: ";
                                    cin.ignore(); // Membersihkan newline character dari input sebelumnya
                                    getline(cin, namaKota);// Menerima input kota dengan spasi
                                    tambahKota(graph, namaKota, angkutanUmum, namaFileKota);
                                    break;
                                }
                                case 6: {
                                    // Hapus kota
                                    hapusKota(graph, angkutanUmum, namaFileKota, namaFile);
                                    break;
                                }
                                case 7: {
                                    // Tampilkan graf antar kota
                                    tampilkanGraf(graph, namaFileKota);
                                    break;
                                }
                                case 8: {
                                    // Cari jadwal angkutan umum berdasarkan nama
                                    cariJadwalAngkutanUmum(graph);
                                    break;
                                }
                                case 9: {
                                    // Lihat daftar kota
                                    lihatDaftarKota(graph, namaFileKota);
                                    break;
                                }
                                case 99: {
                                    // Logout
                                    // Implementasi logout (kembali ke menu sebelumnya)
                                    role = "";
                                    break;
                                }
                                case 0: {
                                    // Keluar
                                    cout << "\nKeluar dari sistem...\nTerima Kasih!\n";
                                    cout << endl;
                                    return 0;
                                }
                                default: {
                                    cout << "\n[!!invalid!!] Menu tidak ada!\n";
                                    cout << endl;
                                }
                            }
                            if (role.empty()) break; // Exit loop if logged out
                        }
                    }
                }
                break;
            }
            case 2: {
                // Register user baru
                registerUser(users);
                break;
            }
            case 0: {
                // Keluar dari program
                cout << "\nKeluar dari sistem...\nTerima Kasih!\n";
                cout << endl;
                return 0;
            }
            default: {
                cout << "\n[!!invalid!!] Menu tidak ada!\n";
                cout << endl;
            }
        }
    } while (true);

    return 0;
}