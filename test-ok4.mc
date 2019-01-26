//OPIS: if iskazi ugnjezdeni, rezultat 3

int f() {
  int a;
  int b;
  int c;

  a = 0;
  b = 3;
  c = 6;

  if(a >= b)
    c = c + 3;  
  else
    if(a > b)
      c = c + 3;  
    else
      c = c - 3; 

  return c;
}

int main() {
  int r;
  r = f(); 
  return r;       
}
