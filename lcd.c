// cc -o lcd lcd.c -lwiringPi -lm

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#define RPI_I2C_DEV             "/dev/i2c-1"    /* Rev.2 */
#define ACM1602N1_SLAVE         0x50
#define ACM1602N1_ADR_CMD       0x0
#define ACM1602N1_ADR_DAT       0x80

#define ACM1602N1_CMD_CLEAR_DISP                0x01
#define ACM1602N1_CMD_RETURN_HOME               0x02

#define ACM1602N1_CMD_ENTRY_MODE_ID_INC         0x02
#define ACM1602N1_CMD_ENTRY_MODE_ID_DEC         0x00
#define ACM1602N1_CMD_ENTRY_MODE_S_SHIFT        0x01
#define ACM1602N1_CMD_ENTRY_MODE_S_NOSHIFT      0x00

#define ACM1602N1_CMD_ENTRY_MODE(id, s) \
        (0x04 | \
         ACM1602N1_CMD_ENTRY_MODE_ID_##id | \
         ACM1602N1_CMD_ENTRY_MODE_S_##s)

#define ACM1602N1_CMD_DISP_ON_OFF_DISP_ON       0x04
#define ACM1602N1_CMD_DISP_ON_OFF_DISP_OFF      0x00
#define ACM1602N1_CMD_DISP_ON_OFF_CUR_ON        0x02
#define ACM1602N1_CMD_DISP_ON_OFF_CUR_OFF       0x00
#define ACM1602N1_CMD_DISP_ON_OFF_BLINK_ON      0x01
#define ACM1602N1_CMD_DISP_ON_OFF_BLINK_OFF     0x00

#define ACM1602N1_CMD_DISP_ON_OFF(disp, cursor, blink) \
        (0x08 | \
         ACM1602N1_CMD_DISP_ON_OFF_DISP_##disp | \
         ACM1602N1_CMD_DISP_ON_OFF_CUR_##cursor | \
         ACM1602N1_CMD_DISP_ON_OFF_BLINK_##blink)

#define ACM1602N1_CMD_CUR_DISP_SHIFT_SC_DISP    0x08
#define ACM1602N1_CMD_CUR_DISP_SHIFT_SC_CUR     0x00
#define ACM1602N1_CMD_CUR_DISP_SHIFT_DIR_RIGHT  0x04
#define ACM1602N1_CMD_CUR_DISP_SHIFT_DIR_LEFT   0x00

#define ACM1602N1_CMD_CUR_DISP_SHIFT(sc, dir) \
        (0x10 | \
         ACM1602N1_CMD_CUR_DISP_SHIFT_SC_##sc | \
         ACM1602N1_CMD_CUR_DISP_SHIFT_DIR_##dir)

#define ACM1602N1_CMD_FUNCTION_DATA_LEN_8BIT            0x10
#define ACM1602N1_CMD_FUNCTION_DATA_LEN_4BIT            0x00
#define ACM1602N1_CMD_FUNCTION_LINE_NUMBER_2LINES       0x08
#define ACM1602N1_CMD_FUNCTION_LINE_NUMBER_1LINE        0x00
#define ACM1602N1_CMD_FUNCTION_FONT_5x11                0x04
#define ACM1602N1_CMD_FUNCTION_FONT_5x8                 0x00

#define ACM1602N1_CMD_FUNCTION_SET(data_len, line_number, font) \
        (0x20 | \
         ACM1602N1_CMD_FUNCTION_DATA_LEN_##data_len | \
         ACM1602N1_CMD_FUNCTION_LINE_NUMBER_##line_number | \
         ACM1602N1_CMD_FUNCTION_FONT_##font)

#define ACM1602N1_CMD_SET_CGRAM_ADR(adr)        (0x40 | adr)
#define ACM1602N1_CMD_SET_DDRAM_ADR(adr)        (0x80 | adr)

static void busyloop(unsigned long n)
{
        volatile unsigned long i;       /* volatile not to be optimized */
        for (i = 0; i < n; i++);
}

static int acm_cmd_write(int fd, unsigned char cmd)
{
        const unsigned char buf[2] = {ACM1602N1_ADR_CMD, cmd};

        if (write(fd, buf, 2) != 2) {
                fprintf(stderr, "ACM1602N1 command write error: 0x%02x\n", cmd);
                return -1;
        }

        return 0;
}

static int acm_dat_write(int fd, const unsigned char *dat, int n)
{
        unsigned char buf[n * 2];
        int i;

        for (i = 0; i < n; i++) {
                buf[i * 2] = ACM1602N1_ADR_DAT;
                buf[i * 2 + 1] = dat[i];
        }
        if (write(fd, buf, n * 2) != n * 2) {
                fprintf(stderr,
                        "ACM1602N1 data write error: data=[0x%02x, ...]\n",
                        dat[0]);
                return -1;
        }

        return 0;
}

static int init_acm(int fd)
{
        if (acm_cmd_write(fd, ACM1602N1_CMD_CLEAR_DISP) < 0)
                return -1;
        busyloop(0x10000);      /* clear display takes some millisecs */
        if (acm_cmd_write(fd,
                          ACM1602N1_CMD_FUNCTION_SET(8BIT, 2LINES, 5x8)) < 0)
                return -1;
        if (acm_cmd_write(fd, ACM1602N1_CMD_DISP_ON_OFF(ON, OFF, OFF)) < 0)
                return -1;
        if (acm_cmd_write(fd, ACM1602N1_CMD_ENTRY_MODE(INC, NOSHIFT)) < 0)
                return -1;

        return 0;
}

int main(void)
{
        int fd;
        const unsigned char test_dat1[] = "good morning!";
        const unsigned char test_dat2[] = {
                0xb5, 0xca, 0xd6, 0xb3, 0xba, 0xde, 0xbb, 0xde, 0xb2, 0xcf, 0xbd, 0x00
        };

        if ((fd = open(RPI_I2C_DEV, O_RDWR)) < 0) {
                printf("Faild to open i2c port\n");
                return 1;
        }
        if (ioctl(fd, I2C_SLAVE, ACM1602N1_SLAVE) < 0) {
                printf("Unable to get bus access to talk to slave\n");
                return 1;
        }

        if (init_acm(fd) < 0)
                return 1;
        if (acm_dat_write(fd, test_dat1, sizeof(test_dat1) - 1) < 0)
                return 1;
        if (acm_cmd_write(fd, ACM1602N1_CMD_SET_DDRAM_ADR(0x40)) < 0)
                return 1;
        if (acm_dat_write(fd, test_dat2, sizeof(test_dat2) - 1) < 0)
                return 1;

        return 0;
}
