# Testne aplikacije
- Date su 2 testne aplikacije, status i 2048
    - Status ispisuje trenutno stanje nunchuka (džojstika, akcelerometra i tastera)
    - 2048 koja predstavlja port igre [2048](https://github.com/mevdschee/2048.c), sa promenjenim komandama, tako da se može igrati sa priloženim drajverom
- Moguće je kreirati ih obe sa komandom `make all`, ali i svaku posebno sa komandama `make status` i `make 2048`, redom.  
- Pre pokretanja aplikacija potrebno je registrovati chardev `/dev/nunchuk` sa komandom koja se ispise u kernel log-u prilikom uvezivanja modula (`sudo mknod /dev/nunchuk X 0`, gde je `X` major number dobijen pri uvezivanju modula) i dati mu potrebne privilegije (`sudo chmod 666 /dev/nunchuk`)
