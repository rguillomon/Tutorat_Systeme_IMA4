READ ME

Tutorat système

Rémy Guillomon // Guillaume Cremaschi

Objectif : Créer une manette avec une carte Arduino relier en connexion USB avec un PC.

Pour compiler, lancer et ultiliser les programmes :

Étape 1: Vérifier que la carte ne soit pas flashée.
>lsusb
On doit voir Arduino apparaitre.

Si flashée : enregistrer le binaire du lien : https://raw.githubusercontent.com/arduino/ArduinoCore-avr/master/firmwares/atmegaxxu2/arduino-usbserial/Arduino-usbserial-uno.hex 

Puis écrire :

>dfu-programmer atmega16u2 erase
>dfu-programmer atmega16u2 flash Arduino-usbserial-uno.hex
>dfu-programmer atmega16u2 reset


Étape 2 :
Compiler et Upload le programme main.c sur l'arduino.

>gcc -o main main.c -Wall  (pas sur que ce soit nécessaire)
>make all
>make upload

(Remarque : On peut tester ce progrmme directement avec :
>minicom -8 -o -b 9600 -D /dev/ttyACM0                   )

Le programme est maintenant sur l'Arduino.

Étape 3:
Flasher la carte

>cd lufa-master/
>cd PolytechLille PAD/
>make all
>dfu-programmer atmega16u2 erase
>dfu-programmer atmega16u2 flash PAD.hex
>dfu-programmer atmega16u2 reset

Étape 4:
Compiler la partie 1 et l'exécuter.
>gcc -o exe init_USB.c -lusb-1.0 -Wall -Wextra
>./exe



Ce qui marche :
_ La Partie 2, fonctionne correctement et compile sans warnings.


Ce qui ne marche pas :     (La partie 3 marche-t-elle ?)