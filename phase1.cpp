#include <iostream>
#include <fstream>
#include <string>
using namespace std;

class OS {
private:
    char M[100][4];
    char IR[4]; 
    char R[4]; 
    int IC; 
    int SI; 
    bool C; 
    char buffer[40];    

public:
    void init();
    void LOAD();
    void Execute();
    void MOS();

    fstream infile;
    fstream outfile;
};

void OS::init() {
    for (int i = 0; i < 100; i++)
        for (int j = 0; j < 4; j++)
            M[i][j] = ' ';

    for (int i = 0; i < 4; i++) {
        IR[i] = ' ';
        R[i] = ' ';
    }

    C = false;
    IC = 0;
    SI = 0;
}

//master mode
void OS::MOS() {
    if (SI == 1) {  //read data
        for (int i = 0; i < 40; i++) buffer[i] = '\0';
        infile.getline(buffer, 40);

        int k = 0;
        int addr = (IR[2] - '0') * 10 + (IR[3] - '0');

        for (int l = 0; l < 10; l++) {
            for (int j = 0; j < 4; j++) {
                if (buffer[k] != '\0')
                    M[addr][j] = buffer[k++];
                else
                    M[addr][j] = ' ';
            }
            addr++;
        }
    }
    else if (SI == 2) { //write data
        string line = "";
        int addr = (IR[2] - '0') * 10 + (IR[3] - '0');

        for (int l = 0; l < 10; l++) {
            for (int j = 0; j < 4; j++) {
                char ch = M[addr][j];
                if (ch != ' ') line.push_back(ch);
            }
            addr++;
        }

        outfile << line << "\n";
    }
    else if (SI == 3) { //halt
        outfile << "\n\n";
    }
}

void OS::Execute() {
    while (true) {
        for (int i = 0; i < 4; i++)
            IR[i] = M[IC][i];
        IC++;

        if (IR[0] == 'G' && IR[1] == 'D') {
            SI = 1; MOS();
        }
        else if (IR[0] == 'P' && IR[1] == 'D') {
            SI = 2; MOS();
        }
        else if (IR[0] == 'H') {
            SI = 3; MOS(); break;
        }
        else if (IR[0] == 'L' && IR[1] == 'R') { // LR
            int addr = (IR[2] - '0') * 10 + (IR[3] - '0');
            for (int j = 0; j < 4; j++)
                R[j] = M[addr][j];
        }
        else if (IR[0] == 'S' && IR[1] == 'R') { // SR
            int addr = (IR[2] - '0') * 10 + (IR[3] - '0');
            for (int j = 0; j < 4; j++)
                M[addr][j] = R[j];
        }
        else if (IR[0] == 'C' && IR[1] == 'R') { // CR
            int addr = (IR[2] - '0') * 10 + (IR[3] - '0');
            C = true;
            for (int j = 0; j < 4; j++) {
                if (M[addr][j] != R[j]) {
                    C = false; break;
                }
            }
        }
        else if (IR[0] == 'B' && IR[1] == 'T') { // BT
            if (C) {
                int addr = (IR[2] - '0') * 10 + (IR[3] - '0');
                IC = addr;
            }
        }
    }
}

void OS::LOAD() {
    int x = 0;
    while (!infile.eof()) {
        for (int i = 0; i < 40; i++) buffer[i] = '\0';
        infile.getline(buffer, 40);

        if (buffer[0] == '$' && buffer[1] == 'A' && buffer[2] == 'M' && buffer[3] == 'J') {
            init();
            x = 0;
        }
        else if (buffer[0] == '$' && buffer[1] == 'D' && buffer[2] == 'T' && buffer[3] == 'A') {
            IC = 0;
            Execute();
        }
        else if (buffer[0] == '$' && buffer[1] == 'E' && buffer[2] == 'N' && buffer[3] == 'D') {
            continue; 
        }
        else {
            int k = 0;
            while (buffer[k] != '\0' && x < 100) {
                for (int j = 0; j < 4 && buffer[k] != '\0'; j++) {
                    M[x][j] = buffer[k++];
                }
                x++;
            }
        }
    }
}

int main() {
    OS os;

    os.infile.open("input.txt", ios::in);
    os.outfile.open("output.txt", ios::out);

    if (!os.infile) {
        cout << "Error: input.txt not found\n";
        return 1;
    }

    os.LOAD();
    cout << "Execution complete. Check output.txt\n";

    os.infile.close();
    os.outfile.close();
    return 0;
}
