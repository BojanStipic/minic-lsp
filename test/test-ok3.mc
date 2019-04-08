//OPIS: Numerički izraz, rezultat -22

int fja() {
  int z;
  int a;
  int b;
  int c;
  int d;
  z = 2;
  a = 4;
  b = 6;
  c = 8;
  d = 10;
  z = a+( (b-(c+d)) - ((b+c)-((d+ 1)+ ((a+ 1)- ((c+a)-(a-c))))) );
  return z;
}

int main() {
  int r;
  r = fja();
  return r;        
}
