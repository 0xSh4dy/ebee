char* t31() {
    char xm[10] = {66, 121, 37, 117, 32, 84, 32, 34, 34, 38};
    static char val[10]; 
    for (int i = 0; i < 10; i++) {
        val[i] = xm[i] ^ 17;
    }
    return val;
}

char *t32(){
    char xm[10] = {109, 37, 37, 98, 39, 113, 34, 35, 104, 113};
    static char val[10];
    for(int i=0;i<10;i++){
        val[i] = (xm[i]+10)^28;
    }
    return val;
}