#ifndef FILEMANAGER_HPP

class FileManager {
    public:
        static string readFileString(string filename) {
            ifstream infile(filename, fstream :: in);
            if(!infile.is_open()) errorlog("error in `readFileString`");
            ostringstream tmp; tmp << infile.rdbuf();
            string res = tmp.str();
            infile.close();
            return res;
        }
        static void writeFileString(string filename, string data) {
            ofstream outfile(filename, fstream :: out);
            if(!outfile.is_open()) errorlog("error in `writeFileString`");
            outfile << data;
            outfile.close();
        }
        static void writeFileBinary(string filename, char *buffer, int size) {
            ofstream outfile(filename, fstream :: out | fstream :: binary);
            if(!outfile.is_open()) errorlog("error in `writeFileBinary`");
            outfile.write(buffer, size);
            outfile.close();
        }
};

#endif
#define FILEMANAGER_HPP
