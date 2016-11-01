/*
 * PCD8544 - Interface with Philips PCD8544 (or compatible) LCDs.
 *
 * Copyright (c) 2010 Carlos Rodrigues <cefrodrigues@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * 
 * 21.11.2013 by oryshed <oryshed@gmail.com>
 * - contrast control
 *
 * 20.12.2012 by oryshed <oryshed@gmail.com>
 * Modifications to support the LCD4884 shield:
 * - changed the default pin assignment to reflect the pin usage of
 *   the LCD4884 shield.
 * - changed the display initialization sequence as the original sequence
 *   was not working with the LCD4884
 * - added setBacklight() method
 * - setting the display to inverse mode doesn't seem to have any effect, so inverse mode
 *   is re-implemented by inverting the characters before sending them to display
 */


#include "PCD8544.h"

#if ARDUINO < 100
#include <WProgram.h>
#else
#include <Arduino.h>
#endif

#include <avr/pgmspace.h>


#define PCD8544_CMD  LOW
#define PCD8544_DATA HIGH


/*
 * If this was a ".h", it would get added to sketches when using
 * the "Sketch -> Import Library..." menu on the Arduino IDE...
 */
#include "charset.cpp"


PCD8544::PCD8544(unsigned char sclk, unsigned char sdin,
                 unsigned char dc, unsigned char reset,
                 unsigned char sce, unsigned char bcklt):
    pin_sclk(sclk),
    pin_sdin(sdin),
    pin_dc(dc),
    pin_reset(reset),
    pin_sce(sce),
	pin_bcklt(bcklt)
{}


void PCD8544::begin(unsigned char width, unsigned char height, unsigned char model)
{
    this->width = width;
    this->height = height;

    this->column = 0;
    this->line = 0;
	this->flags = 0;

    // Sanitize the custom glyphs...
    memset(this->custom, 0, sizeof(this->custom));

    // All pins are outputs (these displays cannot be read)...
    pinMode(this->pin_sclk, OUTPUT);
    pinMode(this->pin_sdin, OUTPUT);
    pinMode(this->pin_dc, OUTPUT);
    pinMode(this->pin_reset, OUTPUT);
    pinMode(this->pin_sce, OUTPUT);
    pinMode(this->pin_bcklt, OUTPUT);
	setBacklight(false);

    // Reset the controller state...
    digitalWrite(this->pin_reset, HIGH);
    digitalWrite(this->pin_sce, HIGH);
    digitalWrite(this->pin_reset, LOW);
    delay(100);  
    digitalWrite(this->pin_reset, HIGH);

	// original library display init 
    // Set the LCD parameters...
    // this->send(PCD8544_CMD, 0x21);  // extended instruction set control (H=1)
    // this->send(PCD8544_CMD, 0x13);  // bias system (1:48)

    // if (model == CHIP_ST7576) {
        // this->send(PCD8544_CMD, 0xe0);  // higher Vop, too faint at default
        // this->send(PCD8544_CMD, 0x05);  // partial display mode
    // } else {
        // this->send(PCD8544_CMD, 0xc2);  // default Vop (3.06 + 66 * 0.06 = 7V)
    // }

    // this->send(PCD8544_CMD, 0x20);  // extended instruction set control (H=0)
    // this->send(PCD8544_CMD, 0x09);  // all display segments on

    // // Clear RAM contents...
    // this->clear();

    // // Activate LCD...
    // this->send(PCD8544_CMD, 0x08);  // display blank
    // this->send(PCD8544_CMD, 0x0c);  // normal mode (0x0d = inverse mode)
    // delay(100);

    // initialization sequence from Sparkfun site
    //this->send(PCD8544_CMD, 0x21);   //switch to extended commands  
    //this->send(PCD8544_CMD, 0xE0);   //set value of Vop (controls contrast) = 0x80 | 0x60 (arbitrary): B0-BF 
    //this->send(PCD8544_CMD, 0x04);   //set temperature coefficient  
    //this->send(PCD8544_CMD, 0x14);   //set bias mode to 1:48.  
    //this->send(PCD8544_CMD, 0x20);   //switch back to regular commands  
    //this->send(PCD8544_CMD, 0x0C);   //enable normal display (dark on light), horizontal addressing	
    
    // // shield init from ebay library
    // this->send(PCD8544_CMD, 0x21);
	// this->send(PCD8544_CMD, 0xc0);
	// this->send(PCD8544_CMD, 0x06);
    // this->send(PCD8544_CMD, 0x13);
    // this->send(PCD8544_CMD, 0x20);
	// // LCD_clear();
	// this->send(PCD8544_CMD, 0x0c);
	// // end shield init
	
    // modified init for the LCD4884 shield
    this->send(PCD8544_CMD, 0x21);
	this->send(PCD8544_CMD, 0xb9);  contrast = 0xb9;
	this->send(PCD8544_CMD, 0x06);
    this->send(PCD8544_CMD, 0x13);
    this->send(PCD8544_CMD, 0x20);
	this->send(PCD8544_CMD, 0x0c);
	clear();
	// end shield init	

    // Place the cursor at the origin...
    this->send(PCD8544_CMD, 0x80);
    this->send(PCD8544_CMD, 0x40);
}


void PCD8544::stop()
{
    this->clear();
    this->setPower(false);
}


void PCD8544::clear()
{
    this->setCursor(0, 0);

    for (unsigned short i = 0; i < this->width * (this->height/8U); i++) {
        this->send(PCD8544_DATA, (flags & FLAG_INVERSE) ? 0xff : 0x00);
    }

    this->setCursor(0, 0);
}


void PCD8544::clearLine()
{
    this->setCursor(0, this->line);

    for (unsigned char i = 0; i < this->width; i++) {
        this->send(PCD8544_DATA, (flags & FLAG_INVERSE) ? 0xff : 0x00);
    }

    this->setCursor(0, this->line);
}


void PCD8544::setPower(bool on)
{
    this->send(PCD8544_CMD, on ? 0x20 : 0x24);
}

void PCD8544::setBacklight(bool on)
{
	if (on)	digitalWrite(pin_bcklt, HIGH);
	else	digitalWrite(pin_bcklt, LOW);
}

inline void PCD8544::display()
{
    this->setPower(true);
}


inline void PCD8544::noDisplay()
{
    this->setPower(false);
}


void PCD8544::setInverse(bool inverse)
{
    // commented out as the display doesn't seem to understand this command
    //this->send(PCD8544_CMD, inverse ? 0x0d : 0x0c);
	if (inverse) this->flags |= FLAG_INVERSE;
	else this->flags &= ~FLAG_INVERSE;
}


void PCD8544::home()
{
    this->setCursor(0, this->line);
}


void PCD8544::setCursor(unsigned char column, unsigned char line)
{
    this->column = (column % this->width);
    this->line = (line % (this->height/9 + 1));

    this->send(PCD8544_CMD, 0x80 | this->column);
    this->send(PCD8544_CMD, 0x40 | this->line); 
}


void PCD8544::createChar(unsigned char chr, const unsigned char *glyph)
{
    // ASCII 0-31 only...
    if (chr >= ' ') {
        return;
    }
    
    this->custom[chr] = glyph;
}

void PCD8544::setStatus(const char *status)
{
    this->setInverse(true);
    this->setCursor(0, STATUS_LINE);
    this->clearLine();
    this->print(status);
    this->setInverse(false);
}

void PCD8544::setStatus(const __FlashStringHelper* status)
{	this->setInverse(true);
    this->setCursor(0,STATUS_LINE);
    this->clearLine();
    this->print(status);
    this->setInverse(false);
}

void PCD8544::clearStatus()
{
    this->setCursor(0, STATUS_LINE);
    this->clearLine();
}


#if ARDUINO < 100
void PCD8544::write(uint8_t chr)
#else
size_t PCD8544::write(uint8_t chr)
#endif
{
    // ASCII 7-bit only...
    if (chr >= 0x80) {
#if ARDUINO < 100
        return;
#else
        return 0;
#endif
    }

    const unsigned char *glyph;
    unsigned char pgm_buffer[5];

    if (chr >= ' ') {
        // Regular ASCII characters are kept in flash to save RAM...
        memcpy_P(pgm_buffer, &charset[chr - ' '], sizeof(pgm_buffer));
        glyph = pgm_buffer;
    } else {
        // Custom glyphs, on the other hand, are stored in RAM...
        if (this->custom[chr]) {
            glyph = this->custom[chr];
        } else {
            // Default to a space character if unset...
            memcpy_P(pgm_buffer, &charset[0], sizeof(pgm_buffer));
            glyph = pgm_buffer;
        }
    }

    // Output one column at a time...
    for (unsigned char i = 0; i < 5; i++) {
        this->send(PCD8544_DATA, (flags & FLAG_INVERSE) ? (glyph[i] ^ 0xff) : glyph[i]);
    }

    // One column between characters...
    this->send(PCD8544_DATA, (flags & FLAG_INVERSE) ? 0xff : 0x00);

    // Update the cursor position...
    this->column = (this->column + 6) % this->width;

    if (this->column == 0) {
        this->line = (this->line + 1) % (this->height/9 + 1);
    }

#if ARDUINO >= 100
    return 1;
#endif
}


void PCD8544::drawBitmap(const unsigned char *data, unsigned char columns, unsigned char lines)
{
    unsigned char scolumn = this->column;
    unsigned char sline = this->line;

    // The bitmap will be clipped at the right/bottom edge of the display...
    unsigned char mx = (scolumn + columns > this->width) ? (this->width - scolumn) : columns;
    unsigned char my = (sline + lines > this->height/8) ? (this->height/8 - sline) : lines;

    for (unsigned char y = 0; y < my; y++) {
        this->setCursor(scolumn, sline + y);

        for (unsigned char x = 0; x < mx; x++) {
            this->send(PCD8544_DATA, data[y * columns + x]);
        }
    }

    // Leave the cursor in a consistent position...
    this->setCursor(scolumn + columns, sline);
}


void PCD8544::drawColumn(unsigned char lines, unsigned char value)
{
    unsigned char scolumn = this->column;
    unsigned char sline = this->line;

    // Keep "value" within range...
    if (value > lines*8) {
        value = lines*8;
    }

    // Find the line where "value" resides...
    unsigned char mark = (lines*8 - 1 - value)/8;
    
    // Clear the lines above the mark...
    for (unsigned char line = 0; line < mark; line++) {
        this->setCursor(scolumn, sline + line);
        this->send(PCD8544_DATA, 0x00);
    }

    // Compute the byte to draw at the "mark" line...
    unsigned char b = 0xff;
    for (unsigned char i = 0; i < lines*8 - mark*8 - value; i++) {
        b <<= 1;
    }

    this->setCursor(scolumn, sline + mark);
    this->send(PCD8544_DATA, b);

    // Fill the lines below the mark...
    for (unsigned char line = mark + 1; line < lines; line++) {
        this->setCursor(scolumn, sline + line);
        this->send(PCD8544_DATA, 0xff);
    }
  
    // Leave the cursor in a consistent position...
    this->setCursor(scolumn + 1, sline); 
}

void PCD8544::setContrast(unsigned char newContrast)
{
    this->send(PCD8544_CMD, 0x21);
    this->send(PCD8544_CMD, newContrast);
    this->send(PCD8544_CMD, 0x20);
    contrast = newContrast;
}

unsigned char PCD8544::getContrast()
{
    return contrast;
}



void PCD8544::send(unsigned char type, unsigned char data)
{
    digitalWrite(this->pin_dc, type);
  
    digitalWrite(this->pin_sce, LOW);
    shiftOut(this->pin_sdin, this->pin_sclk, MSBFIRST, data);
    digitalWrite(this->pin_sce, HIGH);
}


/* vim: set expandtab ts=4 sw=4: */
