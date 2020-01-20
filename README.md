READ ME

Tutorat système

Rémy Guillomon // Guillaume Cremaschi

Objectif : Créer une manette avec une carte Arduino relier en connexion USB avec un PC.

Pour compiler, lancer et ultiliser les programmes :

Étape 1: Vérifier que la carte ne soit pas flashée.

\>lsusb

On doit voir une ligne "Arduino SA Uno (CDC ACM)" apparaître.

Si elle est déjà flashée, il faut enregistrer le binaire du lien : https://raw.githubusercontent.com/arduino/ArduinoCore-avr/master/firmwares/atmegaxxu2/arduino-usbserial/Arduino-usbserial-uno.hex 

Court-circuiter les broches GND et RESET de l'ATMega16U2 sur l'ICSP de ce micro-contrôleur

Puis écrire :

\>dfu-programmer atmega16u2 erase

\>dfu-programmer atmega16u2 flash Arduino-usbserial-uno.hex

\>dfu-programmer atmega16u2 reset

Débrancher et rebrancher le port USB de la carte.

Étape 2 :
Compiler et charger le programme main.c sur l'arduino.

\>make all

\>make upload

Passer en root pour l'upload si nécessaire.

Remarque : Nous pouvons tester ce progrmme directement avec :

\>minicom -8 -o -b 9600 -D /dev/ttyACM0


Le programme est maintenant sur l'Arduino.

Étape 3:
Flasher la carte

\>cd lufa-master/PolytechLille/PAD/

\>make all

Court-circuiter les broches GND et RESET de l'ATMega16U2 sur l'ICSP de ce micro-contrôleur

\>dfu-programmer atmega16u2 erase

\>dfu-programmer atmega16u2 flash PAD.hex

\>dfu-programmer atmega16u2 reset

Débrancher et rebrancher le port USB de la carte.

Étape 4:
Compiler la partie 1 et l'exécuter.

\>cd ../../../Pgm\ PC/
\>gcc -o exe init_USB.c -lusb-1.0 -Wall -Wextra

\>./exe



Ce qui marche :
_ La Partie 2, fonctionne correctement et compile sans warnings.


Ce qui ne marche pas :     (La partie 3 marche-t-elle ?)
