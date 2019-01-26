//OPIS: Konstante, rezultat -562

int f(int x) {
  int z;
  return x + z;
} 

int y() {
  return 2147483647; //max int
}

int main() {
  int a;
  unsigned b;
  a = 0;

  if (a < -10)
    a = f(a + 3);
  else
    a = y();
  a = -556;

  return a - 6;   //-562
}

unsigned z() {
  unsigned b;
  b = 16u;
  if (b < 10u)
    b = 3u;
  return b + 4u;
}
