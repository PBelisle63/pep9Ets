;File: fig0514b.pep
;Computer Systems, Fifth edition
;Figure 5.14(b)
;
         LDBA    0x0013,d    
         STBA    0xFC08,d    
         LDBA    0x0014,d    
         STBA    0xFC08,d    
         LDBA    0x0006,d    
         STBA    0xFC08,d    
         STOP                
         ADDSP   0x756E,i    
         .END                  
