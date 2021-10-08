#ifndef KONFIGURACJA_H
#define KONFIGURACJA_H
#include "Arduino.h"
#include "LITTLEFS.h"

class Konfiguracja
{
    private:
    
    String p_plik;
    public:

    Konfiguracja(String plik); 
    void zapisz(int l, String t);
    String czytaj(int l);
 
};
 
#endif