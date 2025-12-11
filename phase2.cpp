#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <ctime>
#include <vector>
#include <cctype>
#include <set>
using namespace std;

char M[300][4], IR[4], R[4];
bool C;
int PTR, IC, VA, RA;
int SI, PI, TI, EM;
int TTC, LLC, TTL, TLL;
int used[30];
int jobCount = 0;
vector<string> dataLines;
int dataPtr = 0;

struct PCB {
    string jobid;
    int TTL, TLL;
} pcb;

fstream fin, fout;
string line;

void INIT();
void LOAD();
void PAGETABLE();
void ALLOCATE(vector<string> &program);
void STARTEXECUTION();
void EXECUTE();
void ADDRESSMAP();
void MOS();
void READ();
void WRITE();
void TERMINATE(int code);
int allocateFrame();


void INIT() {
    SI = PI = TI = EM = TTC = LLC = IC = VA = RA = 0;
    C = false;
    dataPtr = 0;
    dataLines.clear();
    for (int i = 0; i < 30; i++) used[i] = 0;
    for (int i = 0; i < 300; i++)
        for (int j = 0; j < 4; j++)
            M[i][j] = '_';
    for (int i = 0; i < 4; i++) {
        IR[i] = '_';
        R[i] = '_';
    }
}

int allocateFrame() {
    for (int i = 0; i < 30; i++) {
        if (!used[i]) {
            used[i] = 1;
            return i;
        }
    }
    return -1;
}

void PAGETABLE() {
    PTR = allocateFrame() * 10;
    if (PTR == -10) {
        cout << "Memory Full \n";
        fout << "Memory Full \n";
        return;
    }
    for (int i = PTR; i < PTR + 10; i++)
        for (int j = 0; j < 4; j++)
            M[i][j] = '*';
}

void ALLOCATE(vector<string> &program) {
    string stream;
    for (auto &line : program) {
        for (char ch : line) {
            if (ch != '\r' && ch != '\n')
                stream.push_back(ch);
        }
    }

    int pageTablePtr = PTR;
    int frame = -1;
    int wordInFrame = 10;
    size_t k = 0;

    while (k < stream.size()) {
        if (wordInFrame >= 10) {
            if (pageTablePtr >= PTR + 10) {
                cout << "Page Table limit reached\n";
                fout << "Page Table limit reached\n";
                return;
            }
            frame = allocateFrame();
            if (frame == -1) {
                cout << "No free frame available\n";
                fout << "No free frame available\n";
                return;
            }

            string fs = (frame < 10 ? "0" + to_string(frame) : to_string(frame));
            M[pageTablePtr][2] = fs[0];
            M[pageTablePtr][3] = fs[1];

            pageTablePtr++;
            wordInFrame = 0;
        }

        for (int j = 0; j < 4; j++) {
            M[frame * 10 + wordInFrame][j] =
                (k < stream.size()) ? stream[k++] : ' ';
        }
        wordInFrame++;
    }
}


void LOAD() {
    while (getline(fin, line)) {
        if (line.empty()) continue;

        if (line.substr(0, 4) == "$AMJ") {
            INIT();
            pcb.jobid = line.substr(4, 4);
            pcb.TTL = stoi(line.substr(8, 4));
            pcb.TLL = stoi(line.substr(12, 4));
            TTL = pcb.TTL;
            TLL = pcb.TLL;

            PAGETABLE();

            vector<string> program;
            while (getline(fin, line)) {
                if (line.substr(0, 4) == "$DTA") break;
                program.push_back(line);
            }

            ALLOCATE(program);

            dataLines.clear();
            while (getline(fin, line)) {
                if (line.substr(0, 4) == "$END") break;
                dataLines.push_back(line);
            }

            STARTEXECUTION();
        }
    }
}

void STARTEXECUTION() {
    IC = 0;
    try {
        EXECUTE();
    } catch (runtime_error&) {
        // continue to next job
    }
}

void ADDRESSMAP() {
    PI = 0;

    int page = VA / 10;
    int offset = VA % 10;
    int pte = PTR + page;

    if (pte < PTR || pte >= PTR + 10) {
        PI = 2;
        return;
    }

    if (M[pte][2] == '*' && M[pte][3] == '*') {
        PI = 3;
        return;
    }

    if (!isdigit(M[pte][2]) || !isdigit(M[pte][3])) {
        PI = 2;
        return;
    }

    int frame = (M[pte][2] - '0') * 10 + (M[pte][3] - '0');
    RA = frame * 10 + offset;
}

void EXECUTE() {
    bool halt = false;

    while (!halt) {
        if (TTC >= TTL) {
            TI = 2;
        }

        VA = IC;
        ADDRESSMAP();
        if (PI) {
            if (TTC >= TTL) TI = 2;
            MOS();
            if (EM) break;
            continue;
        }

        for (int i = 0; i < 4; i++)
            IR[i] = M[RA][i];

        string opcode = string(1, IR[0]);
        string op2 = string(1, IR[1]);
        string opPair = opcode + op2;

        TTC++;

        if (TTC >= TTL) {
            TI = 2;
        }

        set<string> validOps = {"GD", "PD", "LR", "SR", "CR", "BT"};
        if (!(validOps.count(opPair) || opcode == "H")) {
            PI = 1;
            if (TTC >= TTL) TI = 2;
            MOS();
            if (EM) return;
            break;
        }

        if (opcode == "H") {
            SI = 3;
            MOS();
            halt = true;
            return;
        }

        if (!isdigit(IR[2]) || !isdigit(IR[3])) {
            PI = 2;
            if (TTC >= TTL) TI = 2;
            MOS();
            if (EM) return;
            break;
        }

        VA = (IR[2] - '0') * 10 + (IR[3] - '0');
        ADDRESSMAP();
        if (PI) {
            if (TTC >= TTL) TI = 2;
            MOS();
            if (EM) break;
            continue;
        }

        if (opPair == "LR") {
            for (int i = 0; i < 4; i++) R[i] = M[RA][i];
        } else if (opPair == "SR") {
            for (int i = 0; i < 4; i++) M[RA][i] = R[i];
        } else if (opPair == "CR") {
            C = true;
            for (int i = 0; i < 4; i++)
                if (R[i] != M[RA][i]) { C = false; break; }
        } else if (opPair == "BT") {
            if (C) IC = VA - 1;
        } else if (opPair == "GD") {
            SI = 1;
            MOS();
        } else if (opPair == "PD") {
            SI = 2;
            MOS();
        }

        IC++;

        if (TTC >= TTL && TI != 2) {
            TI = 2;
        }
    }
}

void MOS() {
    string op = string(1, IR[0]) + string(1, IR[1]);

    if (TI == 0) {
        if (PI == 1) { TERMINATE(4); return; }
        else if (PI == 2) { TERMINATE(5); return; }
        else if (PI == 3) {
            if (op == "GD" || op == "SR") {
                int newFrame = allocateFrame();
                if (newFrame == -1) { TERMINATE(6); return; }

                string fs = (newFrame < 10 ? "0" + to_string(newFrame) : to_string(newFrame));
                int page = VA / 10;
                M[PTR + page][2] = fs[0];
                M[PTR + page][3] = fs[1];

                PI = 0;
                ADDRESSMAP();
                return;
            } else {
                TERMINATE(6);
                return;
            }
        }

        if (SI == 1) READ();
        else if (SI == 2) WRITE();
        else if (SI == 3) TERMINATE(0);

        SI = 0;
        return;
    }

    else if (TI == 2) {
        if (PI == 1) { TERMINATE(7); return; }
        else if (PI == 2) { TERMINATE(8); return; }
        else if (PI == 3) { TERMINATE(3); return; }

        if (SI == 1) { TERMINATE(3); return; }
        else if (SI == 2) { WRITE(); TERMINATE(3); return; }
        else if (SI == 3) { TERMINATE(0); return; }

        TERMINATE(3);
        return;
    }
}

void READ() {
    if (dataPtr >= dataLines.size()) { TERMINATE(1); return; }
    string data = dataLines[dataPtr++];
    int k = 0;
    for (int i = 0; k < data.size() && i < 10; i++)
        for (int j = 0; j < 4 && k < data.size(); j++)
            M[RA + i][j] = data[k++];
    TTC++;
}

void WRITE() {
    LLC++;
    if (LLC > TLL) { TI = 2; TERMINATE(2); return; }
    string out = "";
    for (int i = 0; i < 10; i++)
        for (int j = 0; j < 4; j++)
            if (M[RA + i][j] != '_') out += M[RA + i][j];
    fout << out << endl;
    cout << out << endl;
    TTC++;
}

void TERMINATE(int code) {
    EM = code;
    fout << "\nJob ID: " << pcb.jobid
         << " TTL=" << TTL << " TLL=" << TLL
         << " TTC=" << TTC << " LLC=" << LLC << endl;
    fout << "Termination Code: " << EM << " - ";

    cout << "\nJob ID: " << pcb.jobid
         << " TTL=" << TTL << " TLL=" << TLL
         << " TTC=" << TTC << " LLC=" << LLC << endl;
    cout << "Termination Code: " << EM << " - ";

    switch (EM) {
        case 0: fout << "Normal Termination"; cout << "Normal Termination"; break;
        case 1: fout << "Out of Data"; cout << "Out of Data"; break;
        case 2: fout << "Line Limit Exceeded"; cout << "Line Limit Exceeded"; break;
        case 3: fout << "Time Limit Exceeded"; cout << "Time Limit Exceeded"; break;
        case 4: fout << "Operation Code Error"; cout << "Operation Code Error"; break;
        case 5: fout << "Operand Error"; cout << "Operand Error"; break;
        case 6: fout << "Invalid Page Fault"; cout << "Invalid Page Fault"; break;
        case 7: fout << "Time Limit Exceeded + Operation Code Error"; cout << "Time Limit Exceeded + Operation Code Error"; break;
        case 8: fout << "Time Limit Exceeded + Operand Error"; cout << "Time Limit Exceeded + Operand Error"; break;
        default: fout << "Unknown Error"; cout << "Unknown Error"; break;
    }

    fout << "\n\n";
    cout << "\n\n";
    throw runtime_error("Job Terminated");
}

int main() {
    srand(time(0));
    fin.open("input2.txt", ios::in);
    fout.open("output2.txt", ios::out);

    if (!fin) {
        cout << "Error: input2.txt not found\n";
        return 1;
    }

    fout << "Simulation Output \n";
    cout << "Simulation Output \n";

    LOAD();

    fout << "Execution complete.\n";
    cout << "Execution complete.\nCheck output2.txt\n";

    fin.close();
    fout.close();
    return 0;
}
