void tambah_saldo() {
  
    //pembatalan
    char keyf = keypad.getKey();
    Serial.println(keyf);
    if (keyf=='#'){
        batal="ya";
        return;
    }
    //end pembatalan
  
    input = "";
    isiSaldo = true;
   
    saldo = nilaiBaru;
    saldo =saldo/1000;
    if (saldo > 255){
      saldo = 0;
      isiSldo="gagal";
      Serial.println("Saldo tidak boleh lebih dari 255000");
      lcd.setCursor(0,3);
      lcd.print("GAGAL Saldo > 255000");
      delay(1000);
      tapisisaldo=true; 
      return;
    }
    if (saldo <= 0){
      saldo = 0;
      lcd.setCursor(0,3);
      lcd.print("Isi min Rp. 1000");
      delay(1000);
      tapisisaldo=true; 
      return;
    }
    
    digit = saldo;
    saldo *= 1000;
    lcd.setCursor(0,2);
    lcd.print("Tempelkan Kartu");              
    
  if ( ! mfrc522.PICC_IsNewCardPresent()){
      return;
  }

  if ( ! mfrc522.PICC_ReadCardSerial()){
      return;
  }
  
  
  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  Serial.println(mfrc522.PICC_GetTypeName(piccType));
 
  // Cek kesesuaian kartu
  if (    piccType != MFRC522::PICC_TYPE_MIFARE_MINI
      &&  piccType != MFRC522::PICC_TYPE_MIFARE_1K
      &&  piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
      Serial.println(F("Kode ini hanya dapat digunakan pada MIFARE Classic cards 1KB - 13.56MHz."));
      notif = true;
      delay(2000);
      resetReader();
      return;
  }

  // that is: sector #1, covering block #4 up to and including block #7
  byte sector         = 1;
  byte blockAddr      = 4;
  
  MFRC522::StatusCode status;
  byte buffer[18];
  byte size = sizeof(buffer);

 
  mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
  Serial.println();

  if (isiSaldo){
    // Baca Saldo yang ada dari Kartu
    status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
    if (status != MFRC522::STATUS_OK) {
        Serial.println("Gagal Baca Kartu");
        //Serial.println(mfrc522.GetStatusCodeName(status));
        resetReader();
        return;
    }
    OLDdigit = buffer[0];
    OLDsaldo = OLDdigit;
    OLDsaldo *= 1000;
    
    // Tambah saldo dan Write Saldo pada Kartu
    saldo += OLDsaldo;
    digit += OLDdigit;
    
    if (digit > 255){
      saldo = 0;
      digit = 0;      
      lcd.setCursor(0,3);
      lcd.print("GAGAL Saldo > 255000");
      delay(1000);
      resetReader();
      tapisisaldo=true; 
      return;
    }
    
    byte dataBlock[]    = {
        //0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15
        digit, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(blockAddr, dataBlock, 16);
    if (status != MFRC522::STATUS_OK) {
        Serial.println("GAGAL Write Saldo pada Kartu");
        //Serial.println(mfrc522.GetStatusCodeName(status));
    }
    Serial.println();
  
    status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
    if (status != MFRC522::STATUS_OK) {
        Serial.println("Gagal Baca Kartu");
        //Serial.println(mfrc522.GetStatusCodeName(status));
    }

    Serial.println();
  
    Serial.println("Menambahkan Saldo...");
    if (buffer[0] == dataBlock[0]){
      //Serial.print("data digit ke 0 : ");
      //Serial.println(buffer[0]);
      Serial.print("Saldo kartu sekarang : ");
      Serial.println(saldo);
      Serial.println("_________ Berhasil isi saldo pada kartu ___________");
    }else{
      Serial.println("------------ GAGAL ISI SALDO --------------");
    }
  }
  isiSldo="sukses";
  nilaiBaru=saldo;
  saldo = 0;
  digit = 0;
  tapisisaldo=true; 
  notif = true;
  isiSaldo = false;
  resetReader();
  
}
void dump_byte_array(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
}

void resetReader(){
  // Halt PICC
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();  
}
