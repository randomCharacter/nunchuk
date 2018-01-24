# Nunuchuk driver
Za uvezivanje driver-a potrebno je pokrenuti sledeće komande
- `make` (kako bi se kernel modul napravio)
- `sudo insmod nunchuk.ko` (kako bi se kernel modul ubacio)
- `sudo mknod /dev/nunchuk c X 0`, ***X** - major number kernel modula* (ova komanda se ispisuje prilikom uvezivanja modula i može se dobiti pokretanjem komande `dmesg`)
- `sudo chmod 666 /dev/nunchuk` (kako bi druge aplikacije dobile dozvolu da čitaju i pišu)
