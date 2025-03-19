/*
  
           .                      .
           .                      ;
           :                  - --+- -
           !           .          !
           |        .             .
          _|_         +
        ,  | `.
  --- --+-<#>-+- ---  --  -
        `._|_,'
           T
           |
           !
           :         
           .      
 */
 
void DAC(int Data, int CSpin, boolean Channel){
  Data |=0xf000;// B15(A/B)=1 B, B14(BUF)=1 on, B13(GAn) 1=x1  B12(SHDNn) 1=off
  if(Channel == true) Data &= ~0x8000; // for A-out
  SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
  digitalWrite(CSpin, LOW);
  SPI.transfer((0xff00 & Data)>>8);
  SPI.transfer(0x00ff & Data);
  digitalWrite(CSpin, HIGH);
  SPI.endTransaction();
}
