int main(void) {
    int returnValue = 0;

    for (int i = 0; i < 5; ++i) {
        if (i == 2)
            continue;;
        
        for (int j = 5; j >= 0; --j) {
            ++returnValue;
            break;
        }
    }
}