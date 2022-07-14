// GAMELAN MIDI CONTROLLER
// 6 Pads menggunakan tangga nada pelok (nada dasar = C)
// Tangga nada dan nada dasar dapat diubah-ubah menyesuakian kondisi
// Tangga nada laras pelog = C-D-D#-F-G-A-Bb

#define channel_midi 1         // channel koneksi midi [1-16]
#define state_velocity 1       // 1 untuk aktif; 0 untuk tidak aktif
#define velocity_mapping 1     // 1 untuk Logaritmic mapping; 0 untuk Linear mapping
#define jumlah_gamelan 14      // Jumlah Gamelan yang akan digunakan dan dapat diganti-ganti
#define maximum_velocity 127   // set untuk maksimum velocity (Pukulan terkencang gamelan)
#define midi_on 0b1001         // MS nibble for note on status message
#define midi_off 0b1000        // MS nibble for note off status message

uint8_t padNote[jumlah_gamelan] = {48, 50, 51, 53, 55, 57, 58, 60, 62, 63, 65, 67, 69, 70};             // MIDI notes menggunakan tangga nada Pelog (Gamelan). 
uint16_t padThreshold[jumlah_gamelan] = {80, 80, 80, 80, 80, 80, 80 ,80, 80, 80, 80, 80, 80, 80}; // Batasan treshold untuk membedakan informasi impulse yang berasal dari user dan gangguan
uint16_t padCycles[jumlah_gamelan] = {60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60};          // Waktu minimal untuk trigger selanjutnya (Jika terlalu cepat, tidak akan dijalankan)
uint8_t activePad;                           // State dari Pad
uint16_t padCurrentCycles[jumlah_gamelan];   // Siklus untuk looping pembacaan sensor Piezo

void setup() {
  Serial.begin(115200);                                                    // inisiasi
}

// Fungsi Pembacaan Sensor Analog
void loop() {
    
  for (uint8_t pin = 0; pin <jumlah_gamelan; pin++) {                      // Looping untuk tiap pin AnalogInput
    uint16_t val = analogRead(pin);                                        // Pembacaan Sensor
    if ((val > padThreshold[pin]) && (!padActive(pin))) {                  // Fungsi untuk membaca logika sensor jika melebihi Treshold value (dapat disetting menyesuaikan bentuk mekanis gamelan

      //Kondisi Value akan bergantung pada set awal MIDI controller yang bergantung pada velocity atau tidak
      val = state_velocity ? velocityAlgorithm(val,velocity_mapping) : maximum_velocity; // penyimpanan variabel informasi MIDI yang bergantung pada pad mana yang dipukul
                                                                                         // Velocity akan di set maksimum jika Midi Controller di-set tidak bergantung velocity (state_velocity=0)
                                                                                         
      midi_tx_note_on(padNote[pin], val);                                  // Mengirimkan bit informasi bahwa MIDI telah ditekan
      padCurrentCycles[pin] = 0;                                           // reset the current pad cycle counter
      activePad |= 1 << pin;                                               // set corresponding bit (active flag)
    }

    if (padActive(pin)) {                                                  // enter if pad is active
      padCurrentCycles[pin] += 1;                                          // increment the cycle counter by 1

      if (padCurrentCycles[pin] >= padCycles[pin]) {                       // enter if cycle counter reached the desired number of cycles
        midi_tx_note_off(padNote[pin]);                                    // send a note off MIDI message
        activePad &= ~(1 << pin);                                          // clear corresponding bit (active flag)
      }}}}

// Value yang telah dibaca sensor akan di mapping kan kembali ke MIDI message [0-127]
// Fungsi mapping bergantung pada variabel logswitch. fungsi mapping dapat menggunakan linear approach velocity dan logaritmic approach velocity
// Lebih cocok jika menggunakan logaritmic approach dikarenakan pukulan gamelan yang sifatnya tidak linear (logswitch=1)

uint8_t velocityAlgorithm(uint16_t val, uint8_t logswitch) {
  if (logswitch) {
     return log(val + 1)/ log(1024) * 127;         //jika menggunakan fungsi Mapping Logaritmic
  }
    return (val - 0) * (127 - 0) / (1023 - 0) + 0; //jika menggunakan fungsi mapping Linear                    
}

uint8_t padActive(uint8_t currentPin) {                                    // check if current pad active
  return (activePad >> currentPin) & 1;
}

void midi_tx_note_on(uint8_t pitch, uint8_t velocity) {                    // Mengirimkan bit representasi "note on" dalam format MIDI
  Serial.write((midi_on<< 4) | (channel_midi - 1));
  Serial.write(pitch);
  Serial.write(velocity);
}

void midi_tx_note_off(uint8_t pitch) {                                     // Mengirimkan bit representasi "note off" dalam format MIDI
  Serial.write((midi_off << 4) | (channel_midi - 1));
  Serial.write(pitch);
  Serial.write(0);
}
