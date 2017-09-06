/* DS1307 library for pic16f886
 * Author: dinky winky
 */

void ds_set(unsigned char *data/*y, unsigned char m, unsigned char d, unsigned char h, unsigned char min*/);
unsigned char ds_get(unsigned char data);