
void init_keymatrix()
{
  /* Source: https://gist.github.com/mitchtech/2865219 */
  int32_t i=0;

  // Reading pins in Input mode, and High state
  for(;i<(sizeof(pins_read)/sizeof(pins_read[0]));++i){
    // Enable internal pull-ups
    pinMode(pins_read[i], INPUT_PULLUP);
  }
  // Writing pins in output mode
  for(i=0;i<(sizeof(pins_write)/sizeof(pins_write[0]));++i){
    pinMode(pins_write[i], OUTPUT);
  }
}

/* Function to scan key matrix and return the keys pressed */
uint8_t scankeys()
{
  /* Source: https://github.com/FriesFlorian/keyawesome */
}

