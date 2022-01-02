#ifndef TABLE_HPP

class Table {   // table purify
    private:
        string title;                 // title
        vector<string> head;          // column
        vector<vector<string> > body; // row
        vector<int> lenCol;           // column width
        int szCol;                    // max output column width 
        int maxCol;                   // max count of column
        int quad;                     // placeholder length of between columns
        stringstream ss;
    public:
        Table() {}
        Table(string title, vector<string> &head, vector<vector<string> > &body, int szCol = 64, int quad = 3) {
            this -> title = title;
            this -> head = head;
            this -> body = body;
            this -> szCol = szCol;
            this -> quad = quad;
        }
        void echoPlaceholder(char c, int times, bool nextLine = false) {
            while(times --) ss.put(c);
            if(nextLine) ss << endl;
        }
        void echoCenter(string str, bool nextLine = false, int mxl = -1) {
            if(mxl == -1) mxl = this -> maxCol;
            echoPlaceholder(' ', (mxl - str.length()) / 2);
            ss << str;
            if(nextLine) ss << endl;
        }
        void echoRight(string str, bool nextLine = false, int mxl = -1) {
            if(mxl == -1) mxl = this -> maxCol;
            echoPlaceholder(' ', mxl - str.length());
            ss << str;
            if(nextLine) ss << endl;
        }
        string getshowstr(bool showCnt = true) {
            ss.str("");

            // align data
            for(int i = 0 ; i < (int) body.size() ; ++ i) {
                lenCol.resize(head.size());
            }
            lenCol.resize(head.size());
            // init column width
            for(int i = 0 ; i < (int) head.size() ; ++ i) {
                lenCol[i] = head[i].length();
            } 
            for(int i = 0 ; i < (int) body.size() ; ++ i) {
                for(int j = 0 ; j < (int) body[i].size() ; ++ j) {
                    lenCol[j] = max(lenCol[j], (int) body[i][j].length());
                }
            }
            int totLen = 0;
            for(int i = 0 ; i < (int) lenCol.size() ; ++ i) {
                totLen += lenCol[i];
            }
            
            // empty table isn't allowed
            assert(lenCol.size());
            if(lenCol.size() > 1) {
                quad = max(quad, (int) (szCol - totLen) / ((int) lenCol.size() - 1));
            }
            
            maxCol = 0;
            for(int i = 0 ; i < (int) lenCol.size() ; ++ i) {
                if(i > 0) {
                    maxCol += quad;
                }
                maxCol += lenCol[i];
            }
            
            
            echoPlaceholder('-', maxCol, true);
            echoCenter(title, true);
            echoPlaceholder('-', maxCol, true);
            
            for(int i = 0 ; i < (int) head.size() ; ++ i) {
                if(i > 0) {
                    // echo quad
                    echoPlaceholder(' ', quad);
                }
                ss << head[i]; echoPlaceholder(' ', lenCol[i] - head[i].length());
            } ss << endl;
            echoPlaceholder('-', maxCol, true);
            for(int i = 0 ; i < (int) body.size() ; ++ i) {
                for(int j = 0 ; j < (int) body[i].size() ; ++ j) {
                    if(j > 0) {
                        echoPlaceholder(' ', quad);
                    }
                    ss << body[i][j]; echoPlaceholder(' ', lenCol[j] - body[i][j].length());
                } ss << endl;
            }
            
            echoPlaceholder('-', maxCol, true);
            if(showCnt) {
                echoRight("Total " + to_string(body.size()) + " lines.", true);
                echoPlaceholder('-', maxCol, true);
            }
            ss << endl;
            return ss.str();
        }
        void show(bool showCnt = true) {
            printf("%s", getshowstr(showCnt).c_str());
        }
};

#endif
#define TABLE_HPP
