void framebf_init();
void drawPixelARGB32(int x, int y, unsigned int attr);
void drawRectARGB32(int x1, int y1, int x2, int y2, unsigned int attr, int fill);
void drawPixel(int x, int y, unsigned char attr);
void drawChar(unsigned char ch, int x, int y, unsigned char attr);
void drawString(int x, int y, char *s, unsigned char attr);
// void drawRect(int x1, int y1, int x2, int y2, unsigned char attr, int fill);
// void drawCircle(int x0, int y0, int radius, unsigned char attr, int fill);
// void drawLine(int x1, int y1, int x2, int y2, unsigned char attr);