unsigned int t1(const unsigned char *message, unsigned int len) {
   int i, j;
   unsigned int byte, crc, mask;
   i = 0;
   crc = 0xFFFFFFFF;
   while (i < len) {
      byte = message[i];            
      crc = crc ^ byte;
      for (j = 7; j >= 0; j--) {   
         mask = -(crc & 1);
         crc = (crc >> 1) ^ (0xEDB88320 & mask);
      }
      i = i + 1;
   }
   return ~crc;
}