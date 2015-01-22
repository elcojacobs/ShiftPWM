/******************************************************************************
 * Tis function converts HSV values to RGB values, scaled from 0 to maxBrightness
 * 
 * The ranges for the input variables are:
 * hue: 0-360
 * sat: 0-255
 * lig: 0-255
 * 
 * The ranges for the output variables are:
 * r: 0-maxBrightness
 * g: 0-maxBrightness
 * b: 0-maxBrightness
 * 
 * r,g, and b are passed as pointers, because a function cannot have 3 return variables
 * Use it like this:
 * int hue, sat, val; 
 * unsigned char red, green, blue;
 * // set hue, sat and val
 * hsv2rgb(hue, sat, val, &red, &green, &blue, maxBrightness); //pass r, g, and b as the location where the result should be stored
 * // use r, b and g.
 * 
 * (c) Elco Jacobs, E-atelier Industrial Design TU/e, July 2011.
 * 
 *****************************************************************************/


void hsv2rgb(unsigned int hue, unsigned int sat, unsigned int val, \
              unsigned char * r, unsigned char * g, unsigned char * b, unsigned char maxBrightness ) { 
  unsigned int H_accent = hue/60;
  unsigned int bottom = ((255 - sat) * val)>>8;
  unsigned int top = val;
  unsigned char rising  = ((top-bottom)  *(hue%60   )  )  /  60  +  bottom;
  unsigned char falling = ((top-bottom)  *(60-hue%60)  )  /  60  +  bottom;

  switch(H_accent) {
  case 0:
    *r = top;
    *g = rising;
    *b = bottom;
    break;

  case 1:
    *r = falling;
    *g = top;
    *b = bottom;
    break;

  case 2:
    *r = bottom;
    *g = top;
    *b = rising;
    break;

  case 3:
    *r = bottom;
    *g = falling;
    *b = top;
    break;

  case 4:
    *r = rising;
    *g = bottom;
    *b = top;
    break;

  case 5:
    *r = top;
    *g = bottom;
    *b = falling;
    break;
  }
  // Scale values to maxBrightness
  *r = *r * maxBrightness/255;
  *g = *g * maxBrightness/255;
  *b = *b * maxBrightness/255;
}



