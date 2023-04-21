class Path {
public:
    Path(unsigned char*);
    void getPath(unsigned short, unsigned char*);
    bool isRelative();
    unsigned short begin();
    unsigned short end();
    unsigned short next(unsigned short);
private:
    unsigned char path[32][64];
    unsigned short depth;
    bool relative;
};