//OPIS: Nedostaje num_exp kod return

int abs(int i) {
  int res;
  if(i < 0)
    res = 0 - i;
  else 
    res = i; 
  return ;
}

int main() {
  return abs(-5);
}
