#include <iostream>
#include <fstream>

#define SZ 20

int main() {
   std::ofstream of;

   // open file in binary mode
   of.open("test.bin", std::ios::binary | std::ios::out);
   if (of.fail()) {
      std::cout << "Cannot create file" << std::endl;
      return 1;
   }

   // write 20 bytes in it
   char buf[SZ];
   int j = 0;
   for (int i = 0; i < 512; i++) {
      buf[j] = i;

      j++;
      if (j == 20) {
	 j = 0;
	 of.write(buf, SZ);
      }
   }

   of.write(buf, j);

   if (of.fail()) {
      std::cout << "Write failed" << std::endl;
      of.close();
      return 2;
   }

   of.close();
   return 0;
}
