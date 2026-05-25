jmp_t(_start);

int x;
int y;
int two;

int _preload() {
    x = 0;
    y = 21;
    two = 2;
    return;
}

int _start() {
    _preload();
    jmp_t(main);
}

int main() {
    abc();
    return;
}