
void printNumI(int x, int y, int num);
void printNumF (int x, int y, double num, byte dec = 1, int length = 0);
void printAt (int x, int y, char *st, boolean onScreen = true);
void printAt (int x, int y, String st, boolean onScreen = true);

void printAt (int x, int y, char *st, boolean onScreen) {
  display.setCursor(x, y + OFFSET);
  display.print(st);
  if (onScreen) {
    display.display();
  }
}

void printAt (int x, int y, String st, boolean onScreen) {
  display.setCursor(x, y + OFFSET);
  display.print(st);
  if (onScreen) {
    display.display();
  }
}

void printNumI(int x, int y, int num)
{
  char st[27];

  display.setCursor(x, y + OFFSET);
  sprintf(st, "%i", num);
  if (strlen(st) == 1) {
    display.print(" ");
  }
  display.print(st);
}

void printNumF (int x, int y, double num, byte dec, int length)
{
  char st[27];

  display.setCursor(x, y + OFFSET);
  dtostrf(num, length, dec, st );
  int l = strlen(st);
  // fixed output length 4 chars
  if (l == 3) {
    display.print(F("  "));
  }
  if (l == 4) {
    display.print(F(" "));
  }
  display.print(st);
}
