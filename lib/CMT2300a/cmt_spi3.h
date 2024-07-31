#ifndef __CMT_SPI3_H
#define __CMT_SPI3_H

#include <stdint.h>

<<<<<<< HEAD
void cmt_spi3_init(int8_t pin_sdio, int8_t pin_clk, int8_t pin_cs, int8_t pin_fcs, uint32_t spi_speed);

void cmt_spi3_write(uint8_t addr, uint8_t dat);
uint8_t cmt_spi3_read(uint8_t addr);

void cmt_spi3_write_fifo(const uint8_t* p_buf, uint16_t len);
void cmt_spi3_read_fifo(uint8_t* p_buf, uint16_t len);
=======
void cmt_spi3_init(const int8_t pin_sdio, const int8_t pin_clk, const int8_t pin_cs, const int8_t pin_fcs, const uint32_t spi_speed);

void cmt_spi3_write(const uint8_t addr, const uint8_t dat);
uint8_t cmt_spi3_read(const uint8_t addr);

void cmt_spi3_write_fifo(const uint8_t* p_buf, const uint16_t len);
void cmt_spi3_read_fifo(uint8_t* p_buf, const uint16_t len);
>>>>>>> e541a885f51dcd3b88195ff4fa01a0f413889807

#endif
