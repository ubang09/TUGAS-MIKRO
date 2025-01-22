void merchant(){
  //pembatalan
  char keyf = keypad.getKey();
  Serial.println(keyf);
  if (keyf=='#'){
      batal="ya";
      return;
  }
  //end pembatalan
  
  lcd.setCursor(0,2);
  lcd.print("Tempelkan Kartu");   
  if ( ! mfrc522.PICC_IsNewCardPresent()){
      return;
  }

  if ( ! mfrc522.PICC_ReadCardSerial()){
      return;
  }

  saldo = nilaiBaru;
  saldo =saldo/1000;    
  digit = saldo;
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

  //Serial.println("Current data in sector:");
  mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
  Serial.println();

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
  
  Serial.print("Saldo Kartu Sebelumnya : ");
  Serial.println(OLDsaldo);
  Serial.println();
  
  lcd.setCursor(0,2);
  lcd.print("Saldo Kartu");
  lcd.setCursor(0,3);
  lcd.print(OLDsaldo);
  delay(2000);
  lcd.setCursor(0,2);
  lcd.print("                   ");
  lcd.setCursor(0,3);
  lcd.print("                   ");

  // Kurangi Saldo sebesar tagihan merchant
  if (OLDdigit < digit){
    lcd.setCursor(0,2);
    lcd.print("GAGAL Bayar");
    lcd.setCursor(0,3);
    lcd.print("Saldo Kurang");
    delay(2000);
    lcd.setCursor(0,2);
    lcd.print("                   ");
    lcd.setCursor(0,3);
    lcd.print("                   ");
  
    resetReader();
    return;
  }

  OLDdigit -= digit;
  
  byte dataBlock[]    = {
      //0,      1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15
      OLDdigit, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
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

  Serial.println("Mengurangi Saldo...");
  if (buffer[0] == dataBlock[0]){
    saldo = buffer[0];
    saldo *= 1000;
    
    lcd.setCursor(0,2);
    lcd.print("BERHASIL Transfer");
    lcd.setCursor(0,3);
    lcd.print("Sisa Saldo ");
    lcd.print(saldo);
    delay(2000);
    lcd.setCursor(0,2);
    lcd.print("                   ");
    lcd.setCursor(0,3);
    lcd.print("                   ");
  }else{
    lcd.setCursor(0,2);
    lcd.print("GAGAL Transfer");
  }

  bayarmerchant=true;
  resetReader();
}
