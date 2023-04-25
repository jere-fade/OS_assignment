class Path {
public:
    Path(char*);
    void getPath(unsigned short, char*);
    bool isRelative();
    bool isRoot();
    unsigned short begin();
    unsigned short end();
    unsigned short next(unsigned short);
    void separate(char* path, char* name);
private:
    char path[32][64];
    unsigned short depth;
    bool relative;
};